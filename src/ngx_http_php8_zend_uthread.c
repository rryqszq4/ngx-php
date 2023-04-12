/*
==============================================================================
Copyright (c) 2016-2020, rryqszq4 <rryqszq@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
==============================================================================
*/

#include "ngx_http_php_module.h"
#include "ngx_http_php8_zend_uthread.h"
#include "ngx_http_php_util.h"

#if PHP_MAJOR_VERSION >= 8

#include <zend_closures.h>
#include <zend_dtrace.h>
#include <zend_execute.h>

#if HAVE_DTRACE
#define php_exception__thrown_semaphore 0
#endif /* HAVE_DTRACE */

static int ngx_http_php_zend_call_function(zend_fcall_info *fci, zend_fcall_info_cache *fci_cache);
static void ngx_http_php_zend_throw_exception_internal(zend_object *exception);

static zend_always_inline uint32_t ngx_http_php_zend_get_arg_offset_by_name(
        zend_function *fbc, zend_string *arg_name, void **cache_slot);

static void ZEND_FASTCALL ngx_http_php_zend_param_must_be_ref(const zend_function *func, uint32_t arg_num);

static zval * ZEND_FASTCALL ngx_http_php_zend_handle_named_arg(
        zend_execute_data **call_ptr, zend_string *arg_name,
        uint32_t *arg_num_ptr, void **cache_slot);

int ngx_http_php_zend_eval_stringl(char *str, size_t str_len, zval *retval_ptr, char *string_name) /* {{{ */
{
    zend_op_array *new_op_array;
    uint32_t original_compiler_options;
    int retval;
    zend_string *code_str;

    if (retval_ptr) {
        code_str = zend_string_concat3(
            "return ", sizeof("return ")-1, str, str_len, ";", sizeof(";")-1);
    } else {
        code_str = zend_string_init(str, str_len, 0);
    }

    /*printf("Evaluating '%s'\n", pv.value.str.val);*/

    original_compiler_options = CG(compiler_options);
    CG(compiler_options) = ZEND_COMPILE_DEFAULT_FOR_EVAL;
#if (PHP_MAJOR_VERSION >= 8 && PHP_MINOR_VERSION > 1)
    new_op_array = zend_compile_string(code_str, string_name, ZEND_COMPILE_POSITION_AFTER_OPEN_TAG);
#else
    new_op_array = zend_compile_string(code_str, string_name);
#endif
    CG(compiler_options) = original_compiler_options;

    if (new_op_array) {
        zval local_retval;

        EG(no_extensions)=1;

        new_op_array->scope = zend_get_executed_scope();

        zend_try {
            ZVAL_UNDEF(&local_retval);
            zend_execute(new_op_array, &local_retval);
        } zend_catch {
            destroy_op_array(new_op_array);
            efree_size(new_op_array, sizeof(zend_op_array));
            zend_bailout();
        } zend_end_try();

        if (Z_TYPE(local_retval) != IS_UNDEF) {
            if (retval_ptr) {
                ZVAL_COPY_VALUE(retval_ptr, &local_retval);
            } else {
                zval_ptr_dtor(&local_retval);
            }
        } else {
            if (retval_ptr) {
                ZVAL_NULL(retval_ptr);
            }
        }

        EG(no_extensions)=0;
        destroy_op_array(new_op_array);
        efree_size(new_op_array, sizeof(zend_op_array));
        retval = SUCCESS;
    } else {
        retval = FAILURE;
    }
    zend_string_release(code_str);
    return retval;
}
/* }}} */

int ngx_http_php_zend_eval_stringl_ex(char *str, size_t str_len, zval *retval_ptr, char *string_name, int handle_exceptions) /* {{{ */
{
    int result;

    result = ngx_http_php_zend_eval_stringl(str, str_len, retval_ptr, string_name);
    if (handle_exceptions && EG(exception)) {
        zend_exception_error(EG(exception), E_ERROR);
        result = FAILURE;
    }
    return result;
}
/* }}} */


int ngx_http_php__call_user_function_impl(zval *object, zval *function_name, zval *retval_ptr, uint32_t param_count, zval params[], HashTable *named_params) /* {{{ */
{
    zend_fcall_info fci;

    fci.size = sizeof(fci);
    fci.object = object ? Z_OBJ_P(object) : NULL;
    ZVAL_COPY_VALUE(&fci.function_name, function_name);
    fci.retval = retval_ptr;
    fci.param_count = param_count;
    fci.params = params;
    fci.named_params = named_params;

    return ngx_http_php_zend_call_function(&fci, NULL);
}

static int ngx_http_php_zend_call_function(zend_fcall_info *fci, zend_fcall_info_cache *fci_cache) /* {{{ */
{
    uint32_t i;
    zend_execute_data *call, dummy_execute_data;
    zend_fcall_info_cache fci_cache_local;
    zend_function *func;
    uint32_t call_info;
    void *object_or_called_scope;
    zend_class_entry *orig_fake_scope;

    ZVAL_UNDEF(fci->retval);

    if (!EG(active)) {
        return FAILURE; /* executor is already inactive */
    }

    if (EG(exception)) {
        return FAILURE; /* we would result in an instable executor otherwise */
    }

    ZEND_ASSERT(fci->size == sizeof(zend_fcall_info));

    /* Initialize execute_data */
    if (!EG(current_execute_data)) {
        /* This only happens when we're called outside any execute()'s
         * It shouldn't be strictly necessary to NULL execute_data out,
         * but it may make bugs easier to spot
         */
        memset(&dummy_execute_data, 0, sizeof(zend_execute_data));
        EG(current_execute_data) = &dummy_execute_data;
    } else if (EG(current_execute_data)->func &&
               ZEND_USER_CODE(EG(current_execute_data)->func->common.type) &&
               EG(current_execute_data)->opline->opcode != ZEND_DO_FCALL &&
               EG(current_execute_data)->opline->opcode != ZEND_DO_ICALL &&
               EG(current_execute_data)->opline->opcode != ZEND_DO_UCALL &&
               EG(current_execute_data)->opline->opcode != ZEND_DO_FCALL_BY_NAME) {
        /* Insert fake frame in case of include or magic calls */
        dummy_execute_data = *EG(current_execute_data);
        dummy_execute_data.prev_execute_data = EG(current_execute_data);
        dummy_execute_data.call = NULL;
        dummy_execute_data.opline = NULL;
        dummy_execute_data.func = NULL;
        EG(current_execute_data) = &dummy_execute_data;
    }

    if (!fci_cache || !fci_cache->function_handler) {
        char *error = NULL;

        if (!fci_cache) {
            fci_cache = &fci_cache_local;
        }

        if (!zend_is_callable_ex(&fci->function_name, fci->object, 0, NULL, fci_cache, &error)) {
            if (error) {
                zend_string *callable_name
                    = zend_get_callable_name_ex(&fci->function_name, fci->object);
                zend_error(E_WARNING, "Invalid callback %s, %s", ZSTR_VAL(callable_name), error);
                efree(error);
                zend_string_release_ex(callable_name, 0);
            }
            if (EG(current_execute_data) == &dummy_execute_data) {
                EG(current_execute_data) = dummy_execute_data.prev_execute_data;
            }
            return FAILURE;
        }

        ZEND_ASSERT(!error);
    }

    func = fci_cache->function_handler;
    if ((func->common.fn_flags & ZEND_ACC_STATIC) || !fci_cache->object) {
        fci->object = NULL;
        object_or_called_scope = fci_cache->called_scope;
        call_info = ZEND_CALL_TOP_FUNCTION | ZEND_CALL_DYNAMIC;
    } else {
        fci->object = fci_cache->object;
        object_or_called_scope = fci->object;
        call_info = ZEND_CALL_TOP_FUNCTION | ZEND_CALL_DYNAMIC | ZEND_CALL_HAS_THIS;
    }

    call = zend_vm_stack_push_call_frame(call_info,
        func, fci->param_count, object_or_called_scope);

    if (UNEXPECTED(func->common.fn_flags & ZEND_ACC_DEPRECATED)) {
        zend_deprecated_function(func);

        if (UNEXPECTED(EG(exception))) {
            zend_vm_stack_free_call_frame(call);
            if (EG(current_execute_data) == &dummy_execute_data) {
                EG(current_execute_data) = dummy_execute_data.prev_execute_data;
                zend_rethrow_exception(EG(current_execute_data));
            }
            return FAILURE;
        }
    }

    for (i=0; i<fci->param_count; i++) {
        zval *param = ZEND_CALL_ARG(call, i+1);
        zval *arg = &fci->params[i];
        if (UNEXPECTED(Z_ISUNDEF_P(arg))) {
            /* Allow forwarding undef slots. This is only used by Closure::__invoke(). */
            ZVAL_UNDEF(param);
            ZEND_ADD_CALL_FLAG(call, ZEND_CALL_MAY_HAVE_UNDEF);
            continue;
        }

        if (ARG_SHOULD_BE_SENT_BY_REF(func, i + 1)) {
            if (UNEXPECTED(!Z_ISREF_P(arg))) {
                if (!ARG_MAY_BE_SENT_BY_REF(func, i + 1)) {
                    /* By-value send is not allowed -- emit a warning,
                     * but still perform the call with a by-value send. */
                    ngx_http_php_zend_param_must_be_ref(func, i + 1);
                    if (UNEXPECTED(EG(exception))) {
                        ZEND_CALL_NUM_ARGS(call) = i;
cleanup_args:
                        zend_vm_stack_free_args(call);
                        zend_vm_stack_free_call_frame(call);
                        if (EG(current_execute_data) == &dummy_execute_data) {
                            EG(current_execute_data) = dummy_execute_data.prev_execute_data;
                        }
                        return FAILURE;
                    }
                }
            }
        } else {
            if (Z_ISREF_P(arg) &&
                !(func->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE)) {
                /* don't separate references for __call */
                arg = Z_REFVAL_P(arg);
            }
        }

        ZVAL_COPY(param, arg);
    }

    if (fci->named_params) {
        zend_string *name;
        zval *arg;
        uint32_t arg_num = ZEND_CALL_NUM_ARGS(call) + 1;
        zend_bool have_named_params = 0;
        ZEND_HASH_FOREACH_STR_KEY_VAL(fci->named_params, name, arg) {
            zval *target;
            if (name) {
                void *cache_slot[2] = {NULL, NULL};
                have_named_params = 1;
                target = ngx_http_php_zend_handle_named_arg(&call, name, &arg_num, cache_slot);
                if (!target) {
                    goto cleanup_args;
                }
            } else {
                if (have_named_params) {
                    zend_throw_error(NULL,
                        "Cannot use positional argument after named argument");
                    goto cleanup_args;
                }

                zend_vm_stack_extend_call_frame(&call, arg_num - 1, 1);
                target = ZEND_CALL_ARG(call, arg_num);
            }

            if (ARG_SHOULD_BE_SENT_BY_REF(func, arg_num)) {
                if (UNEXPECTED(!Z_ISREF_P(arg))) {
                    if (!ARG_MAY_BE_SENT_BY_REF(func, arg_num)) {
                        /* By-value send is not allowed -- emit a warning,
                         * but still perform the call with a by-value send. */
                        ngx_http_php_zend_param_must_be_ref(func, arg_num);
                        if (UNEXPECTED(EG(exception))) {
                            goto cleanup_args;
                        }
                    }
                }
            } else {
                if (Z_ISREF_P(arg) &&
                    !(func->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE)) {
                    /* don't separate references for __call */
                    arg = Z_REFVAL_P(arg);
                }
            }

            ZVAL_COPY(target, arg);
            if (!name) {
                ZEND_CALL_NUM_ARGS(call)++;
                arg_num++;
            }
        } ZEND_HASH_FOREACH_END();
    }

    if (UNEXPECTED(func->op_array.fn_flags & ZEND_ACC_CLOSURE)) {
        uint32_t call_info;

        GC_ADDREF(ZEND_CLOSURE_OBJECT(func));
        call_info = ZEND_CALL_CLOSURE;
        if (func->common.fn_flags & ZEND_ACC_FAKE_CLOSURE) {
            call_info |= ZEND_CALL_FAKE_CLOSURE;
        }
        ZEND_ADD_CALL_FLAG(call, call_info);
    }

    if (UNEXPECTED(ZEND_CALL_INFO(call) & ZEND_CALL_MAY_HAVE_UNDEF)) {
        if (zend_handle_undef_args(call) == FAILURE) {
            zend_vm_stack_free_args(call);
            zend_vm_stack_free_call_frame(call);
            if (EG(current_execute_data) == &dummy_execute_data) {
                EG(current_execute_data) = dummy_execute_data.prev_execute_data;
            }
            return SUCCESS;
        }
    }

    orig_fake_scope = EG(fake_scope);
    EG(fake_scope) = NULL;
    if (func->type == ZEND_USER_FUNCTION) {
        int call_via_handler = (func->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE) != 0;
        const zend_op *current_opline_before_exception = EG(opline_before_exception);
        uint32_t orig_jit_trace_num = EG(jit_trace_num);

        zend_init_func_execute_data(call, &func->op_array, fci->retval);
        zend_execute_ex(call);
        EG(jit_trace_num) = orig_jit_trace_num;
        EG(opline_before_exception) = current_opline_before_exception;
        if (call_via_handler) {
            /* We must re-initialize function again */
            fci_cache->function_handler = NULL;
        }
    } else {
        int call_via_handler = (func->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE) != 0;

        ZEND_ASSERT(func->type == ZEND_INTERNAL_FUNCTION);
        ZVAL_NULL(fci->retval);
        call->prev_execute_data = EG(current_execute_data);
        EG(current_execute_data) = call;
        if (EXPECTED(zend_execute_internal == NULL)) {
            /* saves one function call if zend_execute_internal is not used */
            func->internal_function.handler(call, fci->retval);
        } else {
            zend_execute_internal(call, fci->retval);
        }
        EG(current_execute_data) = call->prev_execute_data;
        zend_vm_stack_free_args(call);
        if (UNEXPECTED(ZEND_CALL_INFO(call) & ZEND_CALL_HAS_EXTRA_NAMED_PARAMS)) {
            zend_array_release(call->extra_named_params);
        }

        if (EG(exception)) {
            zval_ptr_dtor(fci->retval);
            ZVAL_UNDEF(fci->retval);
        }

        if (call_via_handler) {
            /* We must re-initialize function again */
            fci_cache->function_handler = NULL;
        }

        /* This flag is regularly checked while running user functions, but not internal
         * So see whether interrupt flag was set while the function was running... */
#if (PHP_MAJOR_VERSION >= 8 && PHP_MINOR_VERSION > 1) 
        if (zend_atomic_bool_load_ex(&EG(vm_interrupt))) {
            zend_atomic_bool_store_ex(&EG(vm_interrupt), false);
            if (zend_atomic_bool_load_ex(&EG(timed_out))) {
#else
        if (EG(vm_interrupt)) {
            EG(vm_interrupt) = 0;
            if (EG(timed_out)) {
#endif
                zend_timeout();
            } else if (zend_interrupt_function) {
                zend_interrupt_function(EG(current_execute_data));
            }
        }
    }
    EG(fake_scope) = orig_fake_scope;

    zend_vm_stack_free_call_frame(call);

    if (EG(current_execute_data) == &dummy_execute_data) {
        EG(current_execute_data) = dummy_execute_data.prev_execute_data;
    }

    if (UNEXPECTED(EG(exception))) {
        if (UNEXPECTED(!EG(current_execute_data))) {
            ngx_http_php_zend_throw_exception_internal(NULL);
        } else if (EG(current_execute_data)->func &&
                   ZEND_USER_CODE(EG(current_execute_data)->func->common.type)) {
            zend_rethrow_exception(EG(current_execute_data));
        }
    }

    return SUCCESS;
}
/* }}} */


static void ngx_http_php_zend_throw_exception_internal(zend_object *exception) /* {{{ */
{
#if HAVE_DTRACE
    if (DTRACE_EXCEPTION_THROWN_ENABLED()) {
        if (exception != NULL) {
            DTRACE_EXCEPTION_THROWN(ZSTR_VAL(exception->ce->name));
        } else {
            DTRACE_EXCEPTION_THROWN(NULL);
        }
    }
#endif /* HAVE_DTRACE */

    if (exception != NULL) {
        zend_object *previous = EG(exception);
        zend_exception_set_previous(exception, EG(exception));
        EG(exception) = exception;
        if (previous) {
            return;
        }
    }
    if (!EG(current_execute_data)) {
        if (exception && (exception->ce == zend_ce_parse_error || exception->ce == zend_ce_compile_error)) {
            return;
        }
        if (EG(exception)) {
            zend_exception_error(EG(exception), E_ERROR);
            zend_bailout();
        }

        // hack way !!!
        // zend_error_noreturn(E_CORE_ERROR, "Exception thrown without a stack frame");
    }

    if (zend_throw_exception_hook) {
        zend_throw_exception_hook(exception);
    }

    if (!EG(current_execute_data) || !EG(current_execute_data)->func ||
        !ZEND_USER_CODE(EG(current_execute_data)->func->common.type) ||
        EG(current_execute_data)->opline->opcode == ZEND_HANDLE_EXCEPTION) {
        /* no need to rethrow the exception */
        return;
    }
    EG(opline_before_exception) = EG(current_execute_data)->opline;
    EG(current_execute_data)->opline = EG(exception_op);
}

static zend_always_inline uint32_t ngx_http_php_zend_get_arg_offset_by_name(
        zend_function *fbc, zend_string *arg_name, void **cache_slot) {
    if (EXPECTED(*cache_slot == fbc)) {
        return *(uintptr_t *)(cache_slot + 1);
    }

    // TODO: Use a hash table?
    uint32_t num_args = fbc->common.num_args;
    if (EXPECTED(fbc->type == ZEND_USER_FUNCTION)
            || EXPECTED(fbc->common.fn_flags & ZEND_ACC_USER_ARG_INFO)) {
        for (uint32_t i = 0; i < num_args; i++) {
            zend_arg_info *arg_info = &fbc->op_array.arg_info[i];
            if (zend_string_equals(arg_name, arg_info->name)) {
                *cache_slot = fbc;
                *(uintptr_t *)(cache_slot + 1) = i;
                return i;
            }
        }
    } else {
        for (uint32_t i = 0; i < num_args; i++) {
            zend_internal_arg_info *arg_info = &fbc->internal_function.arg_info[i];
            size_t len = strlen(arg_info->name);
            if (len == ZSTR_LEN(arg_name) && !memcmp(arg_info->name, ZSTR_VAL(arg_name), len)) {
                *cache_slot = fbc;
                *(uintptr_t *)(cache_slot + 1) = i;
                return i;
            }
        }
    }

    if (fbc->common.fn_flags & ZEND_ACC_VARIADIC) {
        *cache_slot = fbc;
        *(uintptr_t *)(cache_slot + 1) = fbc->common.num_args;
        return fbc->common.num_args;
    }

    return (uint32_t) -1;
}

static void ZEND_FASTCALL ngx_http_php_zend_param_must_be_ref(const zend_function *func, uint32_t arg_num)
{
    const char *arg_name = get_function_arg_name(func, arg_num);

    zend_error(E_WARNING, "%s%s%s(): Argument #%d%s%s%s must be passed by reference, value given",
        func->common.scope ? ZSTR_VAL(func->common.scope->name) : "",
        func->common.scope ? "::" : "",
        ZSTR_VAL(func->common.function_name),
        arg_num,
        arg_name ? " ($" : "",
        arg_name ? arg_name : "",
        arg_name ? ")" : ""
    );
}

static zval * ZEND_FASTCALL ngx_http_php_zend_handle_named_arg(
        zend_execute_data **call_ptr, zend_string *arg_name,
        uint32_t *arg_num_ptr, void **cache_slot) {
    zend_execute_data *call = *call_ptr;
    zend_function *fbc = call->func;
    uint32_t arg_offset = ngx_http_php_zend_get_arg_offset_by_name(fbc, arg_name, cache_slot);
    if (UNEXPECTED(arg_offset == (uint32_t) -1)) {
        zend_throw_error(NULL, "Unknown named parameter $%s", ZSTR_VAL(arg_name));
        return NULL;
    }

    zval *arg;
    if (UNEXPECTED(arg_offset == fbc->common.num_args)) {
        /* Unknown named parameter that will be collected into a variadic. */
        if (!(ZEND_CALL_INFO(call) & ZEND_CALL_HAS_EXTRA_NAMED_PARAMS)) {
            ZEND_ADD_CALL_FLAG(call, ZEND_CALL_HAS_EXTRA_NAMED_PARAMS);
            call->extra_named_params = zend_new_array(0);
        }

        arg = zend_hash_add_empty_element(call->extra_named_params, arg_name);
        if (!arg) {
            zend_throw_error(NULL, "Named parameter $%s overwrites previous argument",
                ZSTR_VAL(arg_name));
            return NULL;
        }
        *arg_num_ptr = arg_offset + 1;
        return arg;
    }

    uint32_t current_num_args = ZEND_CALL_NUM_ARGS(call);
    // TODO: We may wish to optimize the arg_offset == current_num_args case,
    // which is probably common (if the named parameters are in order of declaration).
    if (arg_offset >= current_num_args) {
        uint32_t new_num_args = arg_offset + 1;
        ZEND_CALL_NUM_ARGS(call) = new_num_args;

        uint32_t num_extra_args = new_num_args - current_num_args;
        zend_vm_stack_extend_call_frame(call_ptr, current_num_args, num_extra_args);
        call = *call_ptr;

        arg = ZEND_CALL_VAR_NUM(call, arg_offset);
        if (num_extra_args > 1) {
            zval *zv = ZEND_CALL_VAR_NUM(call, current_num_args);
            do {
                ZVAL_UNDEF(zv);
                zv++;
            } while (zv != arg);
            ZEND_ADD_CALL_FLAG(call, ZEND_CALL_MAY_HAVE_UNDEF);
        }
    } else {
        arg = ZEND_CALL_VAR_NUM(call, arg_offset);
        if (UNEXPECTED(!Z_ISUNDEF_P(arg))) {
            zend_throw_error(NULL, "Named parameter $%s overwrites previous argument",
                ZSTR_VAL(arg_name));
            return NULL;
        }
    }

    *arg_num_ptr = arg_offset + 1;
    return arg;
}

zend_execute_data *zend_vm_stack_copy_call_frame(zend_execute_data *call, uint32_t passed_args, uint32_t additional_args) /* {{{ */
{
    zend_execute_data *new_call;
    int used_stack = (EG(vm_stack_top) - (zval*)call) + additional_args;

    /* copy call frame into new stack segment */
    new_call = zend_vm_stack_extend(used_stack * sizeof(zval));
    *new_call = *call;
    ZEND_ADD_CALL_FLAG(new_call, ZEND_CALL_ALLOCATED);

    if (passed_args) {
        zval *src = ZEND_CALL_ARG(call, 1);
        zval *dst = ZEND_CALL_ARG(new_call, 1);
        do {
            ZVAL_COPY_VALUE(dst, src);
            passed_args--;
            src++;
            dst++;
        } while (passed_args);
    }

    /* delete old call_frame from previous stack segment */
    EG(vm_stack)->prev->top = (zval*)call;

    /* delete previous stack segment if it became empty */
    if (UNEXPECTED(EG(vm_stack)->prev->top == ZEND_VM_STACK_ELEMENTS(EG(vm_stack)->prev))) {
        zend_vm_stack r = EG(vm_stack)->prev;

        EG(vm_stack)->prev = r->prev;
        efree(r);
    }

    return new_call;
}
/* }}} */

#endif

