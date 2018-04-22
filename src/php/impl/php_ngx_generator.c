/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#include "../../ngx_php_debug.h"
#include "php_ngx_generator.h"
#include "../../ngx_http_php_module.h"

static zend_class_entry *php_ngx_generator_class_entry;
static zend_class_entry *php_ngx_php_class_entry;

ZEND_BEGIN_ARG_INFO_EX(arginfo_ngx_generstor_run, 0, 0, 1)
    ZEND_ARG_INFO(0, closure)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_ngx_generstor_next, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_ngx_php_main, 0, 0, 1)
    ZEND_ARG_INFO(0, closure)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_ngx_php_next, 0, 0, 0)
ZEND_END_ARG_INFO()

/*
PHP_METHOD(ngx_generator, run)
{

    zval *closure = NULL;

    zend_function *fptr;
    zval val;

    ngx_http_request_t *r;

    zval func_next;
    zval func_valid;
    zval retval;

    if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "O", &closure, zend_ce_closure) == SUCCESS) {
        fptr = (zend_function*)zend_get_closure_method_def(closure TSRMLS_CC);
        Z_ADDREF_P(closure);
    }else {
        return ;
    }

    zend_create_closure(&val, fptr, NULL, NULL, NULL);

    zval generator_closure;
    object_init_ex(&generator_closure, zend_ce_generator);

    if (call_user_function(EG(function_table), NULL, closure, &generator_closure, 0, NULL TSRMLS_CC) == FAILURE)
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed calling closure");
        return ;
    }

    r = ngx_php_request;

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {
        return ;
    }

    ZVAL_STRING(&func_valid, "valid");
    call_user_function(NULL, closure, &func_valid, &retval, 0, NULL);
    zval_ptr_dtor(&func_valid);

    if (Z_TYPE(retval) == IS_FALSE){
        ZVAL_STRING(&func_next, "next");

        call_user_function(NULL, &(generator_closure), &func_next, &retval, 0, NULL TSRMLS_CC);

        zval_ptr_dtor(&func_next);

        ctx->generator_closure = &(generator_closure);
        ctx->rewrite_phase = 0;
        ctx->phase_status = NGX_AGAIN;
    }

}

PHP_METHOD(ngx_generator, next)
{
    zval *closure;
    //zval *func_valid;
    zval func_next;
    zval func_valid;
    zval retval;

    //zval *object = getThis();

    ngx_http_request_t *r;

    r = ngx_php_request;

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {

    }

    //closure = zend_read_static_property(
    //    php_ngx_generator_class_entry, "closure", sizeof("closure")-1, 0 TSRMLS_CC
    //);

    closure = ctx->generator_closure;

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "rewrite_phase: %d r:%p closure:%p", ctx->rewrite_phase,r,closure);

    ZVAL_STRING(&func_next, "next");

    call_user_function(NULL, closure, &func_next, &retval, 0, NULL);

    zval_ptr_dtor(&func_next);

    ZVAL_STRING(&func_valid, "valid");

    call_user_function(NULL, closure, &func_valid, &retval, 0, NULL);
    zval_ptr_dtor(&func_valid);

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,"bool: %d",Z_BVAL(retval));

    if (Z_TYPE(retval) == IS_FALSE) {
        ctx->rewrite_phase = 0;
        ctx->phase_status = NGX_OK;
    }else {
        ctx->rewrite_phase = 0;
        ctx->phase_status = NGX_AGAIN;
    }

}
*/

PHP_METHOD(ngx_php, main)
{

    zval *closure = NULL;

    //zend_function *fptr;
    //zval val;

    ngx_http_request_t *r;

    zval func_next;
    zval func_valid;
    zval retval;

    if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "O", &closure, zend_ce_closure) == SUCCESS) {
        //fptr = (zend_function*)zend_get_closure_method_def(closure TSRMLS_CC);
        //Z_ADDREF_P(closure);
    }else {
        return ;
    }

    r = ngx_php_request;

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {
        return ;
    }

    //zend_create_closure(&val, fptr, NULL, NULL, NULL);

    //zval generator_closure;
    //object_init_ex(return_value, zend_ce_generator);
    
    ctx->generator_closure = (zval *)emalloc(sizeof(zval));
    //object_init_ex(ctx->generator_closure, zend_ce_generator);

    if (call_user_function(EG(function_table), NULL, closure, ctx->generator_closure, 0, NULL TSRMLS_CC) == FAILURE)
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed calling closure");
        return ;
    }

    if (Z_TYPE_P(ctx->generator_closure) == IS_OBJECT){

        ZVAL_STRING(&func_valid, "valid");
        if (call_user_function(NULL, ctx->generator_closure, &func_valid, &retval, 0, NULL) == FAILURE)
        {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed calling valid");
            return ;
        }
        zval_ptr_dtor(&func_valid);

        //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "rewrite_phase: %d r:%p closure:%p", ctx->rewrite_phase,r,ctx->generator_closure);
        ngx_php_debug("r:%p, closure:%p, retval:%d", r, ctx->generator_closure, Z_TYPE(retval));

        if (Z_TYPE(retval) == IS_TRUE){
            ZVAL_STRING(&func_next, "next");

            call_user_function(NULL, ctx->generator_closure, &func_next, &retval, 0, NULL TSRMLS_CC);

            zval_ptr_dtor(&func_next);

            //ctx->rewrite_phase = 0;
            ctx->phase_status = NGX_AGAIN;
        }else {
            ctx->phase_status = NGX_OK;
        }

    }else {
        efree(ctx->generator_closure);
    }

}

PHP_METHOD(ngx_php, next)
{
    zval *closure;
    //zval *func_valid;
    zval func_next;
    zval func_valid;
    zval retval;

    //zval *object = getThis();

    ngx_http_request_t *r;

    r = ngx_php_request;

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {

    }

    /*closure = zend_read_static_property(
        php_ngx_generator_class_entry, "closure", sizeof("closure")-1, 0 TSRMLS_CC
    );*/

    closure = ctx->generator_closure;

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "rewrite_phase: %d r:%p closure:%p", ctx->rewrite_phase,r,closure);

    ZVAL_STRING(&func_next, "next");

    call_user_function(NULL, closure, &func_next, &retval, 0, NULL TSRMLS_CC);

    zval_ptr_dtor(&func_next);

    ZVAL_STRING(&func_valid, "valid");

    call_user_function(NULL, closure, &func_valid, &retval, 0, NULL TSRMLS_CC);
    zval_ptr_dtor(&func_valid);

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,"bool: %d",Z_BVAL(retval));

    if (Z_TYPE(retval) == IS_FALSE) {
        ctx->rewrite_phase = 0;
        ctx->phase_status = NGX_OK;
    }else {
        ctx->rewrite_phase = 0;
        ctx->phase_status = NGX_AGAIN;
    }
}

static const zend_function_entry php_ngx_generator_class_functions[]={
    //PHP_ME(ngx_generator, run, arginfo_ngx_generstor_run, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    //PHP_ME(ngx_generator, next, arginfo_ngx_generstor_next, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    {NULL, NULL, NULL, 0, 0}
};

static const zend_function_entry php_ngx_php_class_functions[] = {
    PHP_ME(ngx_php, main, arginfo_ngx_php_main, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_php, next, arginfo_ngx_php_next, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    {NULL, NULL, NULL, 0, 0}
};

void 
php_ngx_generator_init(int module_number TSRMLS_DC)
{
    zend_class_entry ngx_generator_class_entry;
    INIT_CLASS_ENTRY(ngx_generator_class_entry, "ngx_generator", php_ngx_generator_class_functions);
    php_ngx_generator_class_entry = zend_register_internal_class(&ngx_generator_class_entry TSRMLS_CC);

    //zend_declare_property_null(php_ngx_generator_class_entry, "closure",  sizeof("closure")-1,  ZEND_ACC_STATIC | ZEND_ACC_PUBLIC TSRMLS_CC);

    zend_class_entry ngx_php_class_entry;
    INIT_CLASS_ENTRY(ngx_php_class_entry, "ngx_php", php_ngx_php_class_functions);
    php_ngx_php_class_entry = zend_register_internal_class(&ngx_php_class_entry TSRMLS_CC);
}