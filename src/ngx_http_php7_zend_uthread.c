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
#include "ngx_http_php7_zend_uthread.h"
#include "ngx_http_php_util.h"

#if PHP_MAJOR_VERSION == 7

#include <zend_closures.h>
#include <zend_dtrace.h>

#if HAVE_DTRACE
#define php_exception__thrown_semaphore 0
#endif /* HAVE_DTRACE */

#if (PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION > 0)
static int ngx_http_php_zend_call_function(zend_fcall_info *fci, zend_fcall_info_cache *fci_cache);
static void ngx_http_php_zend_throw_exception_internal(zval *exception);
#endif

int ngx_http_php_zend_eval_stringl(char *str, size_t str_len, zval *retval_ptr, char *string_name) /* {{{ */
{
    zval pv;
    zend_op_array *new_op_array;
    uint32_t original_compiler_options;
    int retval;

    if (retval_ptr) {
        ZVAL_NEW_STR(&pv, zend_string_alloc(str_len + sizeof("return ;")-1, 1));
        memcpy(Z_STRVAL(pv), "return ", sizeof("return ") - 1);
        memcpy(Z_STRVAL(pv) + sizeof("return ") - 1, str, str_len);
        Z_STRVAL(pv)[Z_STRLEN(pv) - 1] = ';';
        Z_STRVAL(pv)[Z_STRLEN(pv)] = '\0';
    } else {
        ZVAL_STRINGL(&pv, str, str_len);
    }

    /*printf("Evaluating '%s'\n", pv.value.str.val);*/

    original_compiler_options = CG(compiler_options);
    CG(compiler_options) = ZEND_COMPILE_DEFAULT_FOR_EVAL;
    new_op_array = zend_compile_string(&pv, string_name);
    CG(compiler_options) = original_compiler_options;

    if (new_op_array) {
        zval local_retval;

        EG(no_extensions)=1;
#if (PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION > 1)
    new_op_array->scope = zend_get_executed_scope();
#endif

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
    zval_dtor(&pv);
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

#if (PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION > 0)
int ngx_http_php__call_user_function_ex(zval *object, zval *function_name, zval *retval_ptr, uint32_t param_count, zval params[], int no_separation) /* {{{ */
{
    zend_fcall_info fci;

    fci.size = sizeof(fci);
    fci.object = object ? Z_OBJ_P(object) : NULL;
    ZVAL_COPY_VALUE(&fci.function_name, function_name);
    fci.retval = retval_ptr;
    fci.param_count = param_count;
    fci.params = params;
    fci.no_separation = (zend_bool) no_separation;

    return ngx_http_php_zend_call_function(&fci, NULL);
}

static int ngx_http_php_zend_call_function(zend_fcall_info *fci, zend_fcall_info_cache *fci_cache) /* {{{ */
{
    uint32_t i;
    zend_execute_data *call, dummy_execute_data;
    zend_fcall_info_cache fci_cache_local;
    zend_function *func;

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

        if (!zend_is_callable_ex(&fci->function_name, fci->object, IS_CALLABLE_CHECK_SILENT, NULL, fci_cache, &error)) {
            if (error) {
#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 2
                zend_error(E_WARNING, "Invalid callback %s, %s", "", error);
                efree(error);
#else
                zend_string *callable_name
                    = zend_get_callable_name_ex(&fci->function_name, fci->object);
                zend_error(E_WARNING, "Invalid callback %s, %s", ZSTR_VAL(callable_name), error);
                efree(error);
                zend_string_release_ex(callable_name, 0);
#endif
            }
            if (EG(current_execute_data) == &dummy_execute_data) {
                EG(current_execute_data) = dummy_execute_data.prev_execute_data;
            }
            return FAILURE;
        } else if (error) {
            /* Capitalize the first latter of the error message */
            if (error[0] >= 'a' && error[0] <= 'z') {
                error[0] += ('A' - 'a');
            }
            zend_error(E_DEPRECATED, "%s", error);
            efree(error);
            if (UNEXPECTED(EG(exception))) {
                if (EG(current_execute_data) == &dummy_execute_data) {
                    EG(current_execute_data) = dummy_execute_data.prev_execute_data;
                }
                return FAILURE;
            }
        }
    }

    func = fci_cache->function_handler;
     
#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 1
    fci->object = (func->common.fn_flags & ZEND_ACC_STATIC) ?
        NULL : fci_cache->object;

    call = zend_vm_stack_push_call_frame(ZEND_CALL_TOP_FUNCTION,
        func, fci->param_count, fci_cache->called_scope, fci->object);
#elif PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION >= 1 && PHP_MINOR_VERSION < 4
    fci->object = (func->common.fn_flags & ZEND_ACC_STATIC) ?
        NULL : fci_cache->object;

    call = zend_vm_stack_push_call_frame(ZEND_CALL_TOP_FUNCTION | ZEND_CALL_DYNAMIC,
        func, fci->param_count, fci_cache->called_scope, fci->object);
#else
    uint32_t call_info;
    void *object_or_called_scope;
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
#endif

    if (UNEXPECTED(func->common.fn_flags & ZEND_ACC_DEPRECATED)) {
        zend_error(E_DEPRECATED, "Function %s%s%s() is deprecated",
            func->common.scope ? ZSTR_VAL(func->common.scope->name) : "",
            func->common.scope ? "::" : "",
            ZSTR_VAL(func->common.function_name));
        if (UNEXPECTED(EG(exception))) {
            zend_vm_stack_free_call_frame(call);
            if (EG(current_execute_data) == &dummy_execute_data) {
                EG(current_execute_data) = dummy_execute_data.prev_execute_data;
            }
            return FAILURE;
        }
    }

    for (i=0; i<fci->param_count; i++) {
        zval *param;
        zval *arg = &fci->params[i];

        if (ARG_SHOULD_BE_SENT_BY_REF(func, i + 1)) {
            if (UNEXPECTED(!Z_ISREF_P(arg))) {
                if (!fci->no_separation) {
                    /* Separation is enabled -- create a ref */
                    ZVAL_NEW_REF(arg, arg);
                } else if (!ARG_MAY_BE_SENT_BY_REF(func, i + 1)) {
                    /* By-value send is not allowed -- emit a warning,
                     * but still perform the call with a by-value send. */
                    zend_error(E_WARNING,
                        "Parameter %d to %s%s%s() expected to be a reference, value given", i+1,
                        func->common.scope ? ZSTR_VAL(func->common.scope->name) : "",
                        func->common.scope ? "::" : "",
                        ZSTR_VAL(func->common.function_name));
                    if (UNEXPECTED(EG(exception))) {
                        ZEND_CALL_NUM_ARGS(call) = i;
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

        param = ZEND_CALL_ARG(call, i+1);
        ZVAL_COPY(param, arg);
    }

    if (UNEXPECTED(func->op_array.fn_flags & ZEND_ACC_CLOSURE)) {
        uint32_t call_info;

#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 3
        ZEND_ASSERT(GC_TYPE((zend_object*)func->op_array.prototype) == IS_OBJECT);
        GC_REFCOUNT((zend_object*)func->op_array.prototype)++;
#else
        GC_ADDREF(ZEND_CLOSURE_OBJECT(func));
#endif
        call_info = ZEND_CALL_CLOSURE;
#if (PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION > 1)      
        if (func->common.fn_flags & ZEND_ACC_FAKE_CLOSURE) {
            call_info |= ZEND_CALL_FAKE_CLOSURE;
        }
#endif
        ZEND_ADD_CALL_FLAG(call, call_info);
    }

    if (func->type == ZEND_USER_FUNCTION) {
        int call_via_handler = (func->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE) != 0;
        const zend_op *current_opline_before_exception = EG(opline_before_exception);

        zend_init_func_execute_data(call, &func->op_array, fci->retval);
        zend_execute_ex(call);

        EG(opline_before_exception) = current_opline_before_exception;
        if (call_via_handler) {
            /* We must re-initialize function again */
            fci_cache->function_handler = NULL;
        }
    } else if (func->type == ZEND_INTERNAL_FUNCTION) {
        int call_via_handler = (func->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE) != 0;
        ZVAL_NULL(fci->retval);
        call->prev_execute_data = EG(current_execute_data);
        call->return_value = NULL; /* this is not a constructor call */
        EG(current_execute_data) = call;
        if (EXPECTED(zend_execute_internal == NULL)) {
            /* saves one function call if zend_execute_internal is not used */
            func->internal_function.handler(call, fci->retval);
        } else {
            zend_execute_internal(call, fci->retval);
        }
        EG(current_execute_data) = call->prev_execute_data;
        zend_vm_stack_free_args(call);

        if (EG(exception)) {
            zval_ptr_dtor(fci->retval);
            ZVAL_UNDEF(fci->retval);
        }

        if (call_via_handler) {
            /* We must re-initialize function again */
            fci_cache->function_handler = NULL;
        }

    } else { /* ZEND_OVERLOADED_FUNCTION */
        ZVAL_NULL(fci->retval);

        /* Not sure what should be done here if it's a static method */
        if (fci->object) {
            call->prev_execute_data = EG(current_execute_data);
            EG(current_execute_data) = call;
            fci->object->handlers->call_method(func->common.function_name, fci->object, call, fci->retval);
            EG(current_execute_data) = call->prev_execute_data;
        } else {
            zend_throw_error(NULL, "Cannot call overloaded function for non-object");
        }

        zend_vm_stack_free_args(call);

        if (func->type == ZEND_OVERLOADED_FUNCTION_TEMPORARY) {
            zend_string_release_ex(func->common.function_name, 0);
        }
        efree(func);

        if (EG(exception)) {
            zval_ptr_dtor(fci->retval);
            ZVAL_UNDEF(fci->retval);
        }
    }

    zend_vm_stack_free_call_frame(call);

    if (EG(current_execute_data) == &dummy_execute_data) {
        EG(current_execute_data) = dummy_execute_data.prev_execute_data;
    }

    if (UNEXPECTED(EG(exception))) {
        if (UNEXPECTED(!EG(current_execute_data))) {
            ngx_http_php_zend_throw_exception_internal(NULL);
        }
// hack way !!!
#if 1
#if (PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION > 1)
        else if (EG(current_execute_data)->func &&
                   ZEND_USER_CODE(EG(current_execute_data)->func->common.type)) {
            zend_rethrow_exception(EG(current_execute_data));
        }
#endif
#endif
    }

    return SUCCESS;
}

static void ngx_http_php_zend_throw_exception_internal(zval *exception) /* {{{ */
{
#if HAVE_DTRACE
    if (DTRACE_EXCEPTION_THROWN_ENABLED()) {
        if (exception != NULL) {
            DTRACE_EXCEPTION_THROWN(ZSTR_VAL(Z_OBJ_P(exception)->ce->name));
        } else {
            DTRACE_EXCEPTION_THROWN(NULL);
        }
    }
#endif /* HAVE_DTRACE */

    if (exception != NULL) {
        zend_object *previous = EG(exception);
        zend_exception_set_previous(Z_OBJ_P(exception), EG(exception));
        EG(exception) = Z_OBJ_P(exception);
        if (previous) {
            return;
        }
    }
    if (!EG(current_execute_data)) {
#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 3
        if (exception && Z_OBJCE_P(exception) == zend_ce_parse_error)
#else
        if (exception && (Z_OBJCE_P(exception) == zend_ce_parse_error || Z_OBJCE_P(exception) == zend_ce_compile_error))
#endif
        {
            return;
        }
        if(EG(exception)) {
            zend_exception_error(EG(exception), E_ERROR);
        }
        
        // hack way !!!
        //zend_error_noreturn(E_CORE_ERROR, "Exception thrown without a stack frame");
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
#endif

#endif

