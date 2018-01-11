/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#include "ngx_php_debug.h"
#include "ngx_http_php_module.h"
#include "ngx_http_php_zend_uthread.h"

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

void 
ngx_http_php_zend_uthread_inline_routine(ngx_http_request_t *r)
{
    ngx_http_php_ctx_t *ctx;
    //ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;

    //pmcf = ngx_http_get_module_main_conf(r, ngx_http_php_module);
    plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ctx->phase_status = NGX_OK;

    ngx_php_request = r;

    ngx_php_set_request_status(NGX_DECLINED);
    zend_first_try {

        //ngx_php_eval_code(r, pmcf->state, plcf->rewrite_inline_code);
        ngx_http_php_zend_eval_stringl_ex(plcf->rewrite_inline_code->code.string, ngx_strlen(plcf->rewrite_inline_code->code.string), NULL, "ngx_php eval code", 1);
    
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
ngx_http_php_zend_uthread_resume(ngx_http_request_t *r)
{
    ngx_php_request = r;

    zend_try {
        zval *closure;
        zval func_next;
        zval func_valid;
        zval retval;

        ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

        if (ctx == NULL) {

        }

        closure = ctx->generator_closure;

        /*ZVAL_STRING(&func_next, "next");
        call_user_function(NULL, closure, &func_next, &retval, 0, NULL TSRMLS_CC);
        zval_ptr_dtor(&func_next);
        */
        //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,"closure: %p", closure);

        ZVAL_STRING(&func_valid, "valid");

        call_user_function(NULL, closure, &func_valid, &retval, 0, NULL TSRMLS_CC);
        zval_ptr_dtor(&func_valid);

        //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,"closure: %p %d", closure, Z_TYPE(retval));
        ngx_php_debug("r:%p, closure:%p, retval:%d", r, closure, Z_TYPE(retval));

        if (Z_TYPE(retval) == IS_TRUE) {
            ZVAL_STRING(&func_next, "next");
            call_user_function(NULL, closure, &func_next, &retval, 0, NULL TSRMLS_CC);
            zval_ptr_dtor(&func_next);
            ctx->phase_status = NGX_AGAIN;
        }else {
            ctx->phase_status = NGX_OK;
            ngx_http_core_run_phases(r);
            efree(ctx->generator_closure);
        }
    }zend_catch {

    }zend_end_try();
}


