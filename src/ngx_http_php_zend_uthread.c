/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#include "ngx_http_php_module.h"
#include "ngx_http_php_zend_uthread.h"

void 
ngx_http_php_zend_uthread_inline_routine(ngx_http_request_t *r)
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

        ngx_php_eval_code(r, pmcf->state, plcf->rewrite_inline_code);
    
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
        
        if (Z_TYPE(retval) == IS_TRUE) {
            ZVAL_STRING(&func_next, "next");
            call_user_function(NULL, closure, &func_next, &retval, 0, NULL TSRMLS_CC);
            zval_ptr_dtor(&func_next);
            ctx->phase_status = NGX_AGAIN;
        }else {
            efree(ctx->generator_closure);
            ctx->phase_status = NGX_OK;
            ngx_http_core_run_phases(r);
        }
    }zend_catch {

    }zend_end_try();
}


