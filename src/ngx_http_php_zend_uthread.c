/*
==============================================================================
Copyright (c) 2016-2019, rryqszq4 <rryqszq@gmail.com>
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
#include "ngx_http_php_zend_uthread.h"
#include "ngx_http_php_util.h"

#include <zend_closures.h>
#include <zend_dtrace.h>

#ifdef HAVE_DTRACE
#define php_exception__thrown_semaphore 0
#endif /* HAVE_DTRACE */

static int ngx_http_php_zend_eval_stringl(char *str, size_t str_len, zval *retval_ptr, char *string_name);
static int ngx_http_php_zend_eval_stringl_ex(char *str, size_t str_len, zval *retval_ptr, char *string_name, int handle_exceptions);

#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION > 0
static int ngx_http_php__call_user_function_ex(zval *object, zval *function_name, zval *retval_ptr, uint32_t param_count, zval params[], int no_separation);
static int ngx_http_php_zend_call_function(zend_fcall_info *fci, zend_fcall_info_cache *fci_cache);
static void ngx_http_php_zend_throw_exception_internal(zval *exception);
#endif

static int ngx_http_php_zend_eval_stringl(char *str, size_t str_len, zval *retval_ptr, char *string_name) /* {{{ */
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
#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION > 1
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

static int ngx_http_php_zend_eval_stringl_ex(char *str, size_t str_len, zval *retval_ptr, char *string_name, int handle_exceptions) /* {{{ */
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

#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION > 0
static int ngx_http_php__call_user_function_ex(zval *object, zval *function_name, zval *retval_ptr, uint32_t param_count, zval params[], int no_separation) /* {{{ */
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
    fci->object = (func->common.fn_flags & ZEND_ACC_STATIC) ?
        NULL : fci_cache->object;

    call = zend_vm_stack_push_call_frame( 
#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 1
        ZEND_CALL_TOP_FUNCTION,
#else
        ZEND_CALL_TOP_FUNCTION | ZEND_CALL_DYNAMIC,
#endif
        func, fci->param_count, fci_cache->called_scope, fci->object);

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
#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION > 1        
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
#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION > 1
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
#ifdef HAVE_DTRACE
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

void 
ngx_http_php_zend_uthread_rewrite_inline_routine(ngx_http_request_t *r)
{
    ngx_http_php_ctx_t *ctx;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t inline_code;

    plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ctx->phase_status = NGX_OK;

    ngx_php_request = r;

    ngx_php_set_request_status(NGX_DECLINED);

    inline_code.data = ngx_pnalloc(r->pool, sizeof("function ngx_rewrite_(){  }")-1 + ngx_strlen(plcf->rewrite_inline_code->code.string) + 32);

    inline_code.len = ngx_sprintf(inline_code.data, "function ngx_rewrite_%V(){ %*s }", 
                                        &(plcf->rewrite_inline_code->code_id), 
                                        ngx_strlen(plcf->rewrite_inline_code->code.string),
                                        plcf->rewrite_inline_code->code.string
                                    ) - inline_code.data;

    ngx_php_debug("%*s, %d", (int)inline_code.len, inline_code.data, (int)inline_code.len);

    zend_first_try {

        if (!plcf->enabled_rewrite_inline_compile){
            ngx_http_php_zend_eval_stringl_ex(
                (char *)inline_code.data, 
                inline_code.len, 
                NULL, 
                "ngx_php eval code", 
                1
            );
            plcf->enabled_rewrite_inline_compile = 1;
        }

        ngx_http_php_zend_uthread_create(r, "ngx_rewrite");

    }zend_end_try();
}

void 
ngx_http_php_zend_uthread_access_inline_routine(ngx_http_request_t *r)
{
    ngx_http_php_ctx_t *ctx;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t inline_code;

    plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ctx->phase_status = NGX_OK;

    ngx_php_request = r;

    ngx_php_set_request_status(NGX_DECLINED);

    inline_code.data = ngx_pnalloc(r->pool, sizeof("function ngx_access_(){  }")-1 + ngx_strlen(plcf->access_inline_code->code.string) + 32);

    inline_code.len = ngx_sprintf(inline_code.data, "function ngx_access_%V(){ %*s }", 
                                        &(plcf->access_inline_code->code_id), 
                                        ngx_strlen(plcf->access_inline_code->code.string),
                                        plcf->access_inline_code->code.string
                                    ) - inline_code.data;

    ngx_php_debug("%*s, %d", (int)inline_code.len, inline_code.data, (int)inline_code.len);

    zend_first_try {

        if (!plcf->enabled_access_inline_compile){
            ngx_http_php_zend_eval_stringl_ex(
                (char *)inline_code.data, 
                inline_code.len, 
                NULL, 
                "ngx_php eval code", 
                1
            );
            plcf->enabled_access_inline_compile = 1;
        }

        ngx_http_php_zend_uthread_create(r, "ngx_access");

    }zend_end_try();
}

void 
ngx_http_php_zend_uthread_content_inline_routine(ngx_http_request_t *r)
{
    ngx_http_php_ctx_t *ctx;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t inline_code;

    plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ctx->phase_status = NGX_OK;

    ngx_php_request = r;

    ngx_php_set_request_status(NGX_DECLINED);

    inline_code.data = ngx_pnalloc(r->pool, sizeof("function ngx_content_(){  }")-1 + ngx_strlen(plcf->content_inline_code->code.string) + 32);

    inline_code.len = ngx_sprintf(inline_code.data, "function ngx_content_%V(){ %*s }", 
                                        &(plcf->content_inline_code->code_id), 
                                        ngx_strlen(plcf->content_inline_code->code.string),
                                        plcf->content_inline_code->code.string
                                    ) - inline_code.data;

    ngx_php_debug("%*s, %d", (int)inline_code.len, inline_code.data, (int)inline_code.len);

    zend_first_try {

        if (!plcf->enabled_content_inline_compile){
            //inline_code.data = (u_char *)str_replace((char *)inline_code.data, "ngx::sleep", "yield ngx::sleep");
            //inline_code.len = strlen((char *)inline_code.data);
            ngx_http_php_zend_eval_stringl_ex(
                (char *)inline_code.data, 
                inline_code.len, 
                NULL, 
                "ngx_php eval code", 
                1
            );
            plcf->enabled_content_inline_compile = 1;
        }
        
        ngx_http_php_zend_uthread_create(r, "ngx_content");
    
    }zend_end_try();
}

void 
ngx_http_php_zend_uthread_log_inline_routine(ngx_http_request_t *r)
{
    //ngx_http_php_ctx_t *ctx;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t inline_code;

    plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
    //ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    //ctx->phase_status = NGX_OK;

    ngx_php_request = r;

    ngx_php_set_request_status(NGX_DECLINED);

    inline_code.data = ngx_pnalloc(r->pool, sizeof("function ngx_log_(){  }")-1 + ngx_strlen(plcf->log_inline_code->code.string) + 32);

    inline_code.len = ngx_sprintf(inline_code.data, "function ngx_log_%V(){ %*s }", 
                                        &(plcf->log_inline_code->code_id), 
                                        ngx_strlen(plcf->log_inline_code->code.string),
                                        plcf->log_inline_code->code.string
                                    ) - inline_code.data;

    ngx_php_debug("%*s, %d", (int)inline_code.len, inline_code.data, (int)inline_code.len);

    zend_first_try {

        if (!plcf->enabled_log_inline_compile){
            ngx_http_php_zend_eval_stringl_ex(
                (char *)inline_code.data, 
                inline_code.len, 
                NULL, 
                "ngx_php eval code", 
                1
            );
            plcf->enabled_log_inline_compile = 1;
        }
        
        ngx_http_php_zend_uthread_create(r, "ngx_log");
    
    }zend_end_try();
}

void 
ngx_http_php_zend_uthread_header_filter_inline_routine(ngx_http_request_t *r)
{
    //ngx_http_php_ctx_t          *ctx;
    ngx_http_php_loc_conf_t     *plcf;
    ngx_str_t                   inline_code;

    plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
    //ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ngx_php_request = r;

    inline_code.data = ngx_pnalloc(r->pool, sizeof("function ngx_header_filter_(){  }")-1 + ngx_strlen(plcf->header_filter_inline_code->code.string) + 32);

    inline_code.len = ngx_sprintf(inline_code.data, "function ngx_header_filter_%V(){ %*s }", 
                                        &(plcf->header_filter_inline_code->code_id), 
                                        ngx_strlen(plcf->header_filter_inline_code->code.string),
                                        plcf->header_filter_inline_code->code.string
                                    ) - inline_code.data;

    ngx_php_debug("%*s, %d", (int)inline_code.len, inline_code.data, (int)inline_code.len);

    zend_first_try {

        if (!plcf->enabled_header_filter_inline_compile) {
            ngx_http_php_zend_eval_stringl_ex(
                (char *)inline_code.data,
                inline_code.len,
                NULL,
                "ngx_php eval code",
                1
            );
            plcf->enabled_header_filter_inline_compile = 1;
        }

        ngx_http_php_zend_uthread_create(r, "ngx_header_filter");

    }zend_end_try();
}

void 
ngx_http_php_zend_uthread_body_filter_inline_routine(ngx_http_request_t *r)
{
    //ngx_http_php_ctx_t          *ctx;
    ngx_http_php_loc_conf_t     *plcf;
    ngx_str_t                   inline_code;

    plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
    //ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ngx_php_request = r;

    inline_code.data = ngx_pnalloc(r->pool, sizeof("function ngx_body_filter_(){  }")-1 + ngx_strlen(plcf->body_filter_inline_code->code.string) + 32);

    inline_code.len = ngx_sprintf(inline_code.data, "function ngx_body_filter_%V(){ %*s }", 
                                        &(plcf->body_filter_inline_code->code_id), 
                                        ngx_strlen(plcf->body_filter_inline_code->code.string),
                                        plcf->body_filter_inline_code->code.string
                                    ) - inline_code.data;

    ngx_php_debug("%*s, %d", (int)inline_code.len, inline_code.data, (int)inline_code.len);

    zend_first_try {

        if (!plcf->enabled_body_filter_inline_compile) {
            ngx_http_php_zend_eval_stringl_ex(
                (char *)inline_code.data,
                inline_code.len,
                NULL,
                "ngx_php eval code",
                1
            );
            plcf->enabled_body_filter_inline_compile = 1;
        }

        ngx_http_php_zend_uthread_create(r, "ngx_body_filter");

    }zend_end_try();
}

void 
ngx_http_php_zend_uthread_file_routine(ngx_http_request_t *r)
{   
    ngx_http_php_ctx_t *ctx;
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;

    pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
    plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ctx->phase_status = NGX_OK;

    ngx_php_request = r;

    ngx_php_set_request_status(NGX_DECLINED);
    zend_first_try {

        ngx_php_eval_file(r, pmcf->state, plcf->rewrite_code);
        
    }zend_end_try();
}

void 
ngx_http_php_zend_uthread_create(ngx_http_request_t *r, char *func_prefix)
{
    zval func_main;
    //zval func_next;
    zval func_valid;
    zval retval;
    ngx_http_php_ctx_t *ctx;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t func_name;

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {
        return ;
    }
    
    ctx->generator_closure = (zval *)emalloc(sizeof(zval));

    plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);

    //func_name.data = ngx_pnalloc(r->pool, strlen(func_prefix)+sizeof("_18446744073709551616")-1+NGX_TIME_T_LEN);
    //func_name.len = ngx_sprintf(func_name.data, "%s_%V", func_prefix, &(plcf->content_inline_code->code_id)) - func_name.data;

    func_name.data = ngx_pnalloc(r->pool, strlen(func_prefix) + 32);

    if (strcmp(func_prefix, "ngx_rewrite") == 0) {
        func_name.len = ngx_sprintf(func_name.data, "%s_%V", func_prefix, &(plcf->rewrite_inline_code->code_id)) - func_name.data;
    }else if (strcmp(func_prefix, "ngx_access") == 0) {
        func_name.len = ngx_sprintf(func_name.data, "%s_%V", func_prefix, &(plcf->access_inline_code->code_id)) - func_name.data;
    }else if (strcmp(func_prefix, "ngx_content") == 0) {
        func_name.len = ngx_sprintf(func_name.data, "%s_%V", func_prefix, &(plcf->content_inline_code->code_id)) - func_name.data;
    }else if (strcmp(func_prefix, "ngx_log") == 0) {
        func_name.len = ngx_sprintf(func_name.data, "%s_%V", func_prefix, &(plcf->log_inline_code->code_id)) - func_name.data;
    }else if (strcmp(func_prefix, "ngx_header_filter") == 0) {
        func_name.len = ngx_sprintf(func_name.data, "%s_%V", func_prefix, &(plcf->header_filter_inline_code->code_id)) - func_name.data;
    }else if (strcmp(func_prefix, "ngx_body_filter") == 0) {
        func_name.len = ngx_sprintf(func_name.data, "%s_%V", func_prefix, &(plcf->body_filter_inline_code->code_id)) - func_name.data;
    }else {
        func_name.len = 0;
    }

    ngx_php_debug("%*s", (int)func_name.len, func_name.data);

    zend_try {
        ZVAL_STRINGL(&func_main, (char *)func_name.data, func_name.len);
        ngx_http_php_call_user_function(EG(function_table), NULL, &func_main, ctx->generator_closure, 0, NULL);
        zval_ptr_dtor(&func_main);

        if ( !ctx->generator_closure ) {
            return ;
        }

        if (Z_TYPE_P(ctx->generator_closure) == IS_OBJECT){

            ZVAL_STRING(&func_valid, "valid");
            if (ngx_http_php_call_user_function(NULL, ctx->generator_closure, &func_valid, &retval, 0, NULL) == FAILURE)
            {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed calling valid");
                return ;
            }
            zval_ptr_dtor(&func_valid);

            ngx_php_debug("r:%p, closure:%p, retval:%d", r, ctx->generator_closure, Z_TYPE(retval));

            if (Z_TYPE(retval) == IS_TRUE){
                /*
                ZVAL_STRING(&func_next, "next");

                ngx_http_php_call_user_function(NULL, ctx->generator_closure, &func_next, &retval, 0, NULL TSRMLS_CC);

                zval_ptr_dtor(&func_next);
                */
                ctx->phase_status = NGX_AGAIN;
            }else {
                ctx->phase_status = NGX_OK;
            }

        }else {
            ngx_php_debug("r:%p, closure:%p, retval:%d", r, ctx->generator_closure, Z_TYPE(retval));
            zval_ptr_dtor(ctx->generator_closure);
            efree(ctx->generator_closure);
            ctx->generator_closure = NULL;
        }
    }zend_catch {
        zval_ptr_dtor(&func_main);
        if ( ctx && ctx->generator_closure ){
            zval_ptr_dtor(ctx->generator_closure);
            efree(ctx->generator_closure);
            ctx->generator_closure = NULL;
        }
    }zend_end_try();
}

void 
ngx_http_php_zend_uthread_resume(ngx_http_request_t *r)
{
    ngx_php_request = r;

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ngx_php ctx is nil at zend_uthread_resume");
        return ;
    }

    ngx_php_debug("ctx: %p", ctx);

    zend_try {
        zval *closure;
        zval func_next;
        zval func_valid;
        zval retval;

        closure = ctx->generator_closure;
        ngx_php_debug("closure: %p", closure);
        if (!closure) {
            if (ctx->upstream) {
                ngx_http_php_socket_clear(r);
            }
            return ;
        }

        // ngx_php_debug("uthread resume before.");

        ZVAL_STRING(&func_next, "next");
        if ( ngx_http_php_call_user_function(NULL, closure, &func_next, &retval, 0, NULL TSRMLS_CC) == FAILURE )
        {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed calling next");
            return ;
        }
        zval_ptr_dtor(&func_next);

        /*
        错误：变量‘ctx’能为‘longjmp’或‘vfork’所篡改 [-Werror=clobbered]
        错误：实参‘r’可能为‘longjmp’或‘vfork’所篡改 [-Werror=clobbered]
        */
        //r = ngx_php_request;
        //ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
        ngx_php_debug("%d, %p, %p", Z_TYPE_P(closure), r, ctx);
        if ( ctx->end_of_request ) {
            //zval_ptr_dtor(closure);
            //efree(closure);
            //closure = NULL;
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "End of request and zend uthread has be shutdown");
            return;
        }

        ZVAL_STRING(&func_valid, "valid");
        if ( ngx_http_php_call_user_function(NULL, closure, &func_valid, &retval, 0, NULL TSRMLS_CC) == FAILURE )
        {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed calling valid");
            return ;
        }
        zval_ptr_dtor(&func_valid);

        ngx_php_debug("r:%p, closure:%p, retval:%d,%d", r, closure, Z_TYPE(retval), IS_TRUE);

        // ngx_php_debug("uthread resume after.");

        if (Z_TYPE(retval) == IS_TRUE) {
            ctx->phase_status = NGX_AGAIN;
        }else {
            ctx->phase_status = NGX_OK;
            
            if ( ctx->generator_closure ) {
                zval_ptr_dtor(ctx->generator_closure);
                efree(ctx->generator_closure);
                ctx->generator_closure = NULL;
            }
            
            ngx_http_core_run_phases(r);
        }

    }zend_catch {
        if ( ctx && ctx->generator_closure ){
            zval_ptr_dtor(ctx->generator_closure);
            efree(ctx->generator_closure);
            ctx->generator_closure = NULL;
        }
    }zend_end_try();
}

void 
ngx_http_php_zend_uthread_exit(ngx_http_request_t *r)
{
    ngx_http_php_ctx_t *ctx;

    ngx_php_request = r;

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if ( ctx && ctx->generator_closure ) {
        //ngx_http_php_zend_uthread_resume(r);
        ctx->phase_status = NGX_OK;
        zval_ptr_dtor(ctx->generator_closure);
        efree(ctx->generator_closure);
        ctx->generator_closure = NULL;
    }

    if ( ctx && ctx->upstream ) {
        ngx_http_php_socket_clear(r);
    }

    if ( ctx && ctx->php_socket ) {
        efree(ctx->php_socket);
        ctx->php_socket = NULL;
    }

    ngx_http_core_run_phases(r);

}



