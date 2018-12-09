/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#include "ngx_http_php_module.h"
#include "ngx_http_php_zend_uthread.h"
#include "ngx_http_php_util.h"

static int ngx_http_php_zend_eval_stringl(char *str, size_t str_len, zval *retval_ptr, char *string_name);
static int ngx_http_php_zend_eval_stringl_ex(char *str, size_t str_len, zval *retval_ptr, char *string_name, int handle_exceptions);

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

    ZVAL_STRINGL(&func_main, (char *)func_name.data, func_name.len);
    call_user_function(EG(function_table), NULL, &func_main, ctx->generator_closure, 0, NULL);
    zval_ptr_dtor(&func_main);

    if (Z_TYPE_P(ctx->generator_closure) == IS_OBJECT){

        ZVAL_STRING(&func_valid, "valid");
        if (call_user_function(NULL, ctx->generator_closure, &func_valid, &retval, 0, NULL) == FAILURE)
        {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed calling valid");
            return ;
        }
        zval_ptr_dtor(&func_valid);

        ngx_php_debug("r:%p, closure:%p, retval:%d", r, ctx->generator_closure, Z_TYPE(retval));

        if (Z_TYPE(retval) == IS_TRUE){
            /*
            ZVAL_STRING(&func_next, "next");

            call_user_function(NULL, ctx->generator_closure, &func_next, &retval, 0, NULL TSRMLS_CC);

            zval_ptr_dtor(&func_next);
            */
            ctx->phase_status = NGX_AGAIN;
        }else {
            ctx->phase_status = NGX_OK;
        }

    }else {
        ngx_php_debug("r:%p, closure:%p, retval:%d", r, ctx->generator_closure, Z_TYPE(retval));
        efree(ctx->generator_closure);
    }
}

void 
ngx_http_php_zend_uthread_resume(ngx_http_request_t *r)
{
    ngx_php_request = r;

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {
        
    }

    ngx_php_debug("ctx: %p", ctx);

    zend_try {
        zval *closure;
        zval func_next;
        zval func_valid;
        zval retval;

        closure = ctx->generator_closure;

        if (ctx->upstream && ctx->upstream->enabled_receive == 1) {
            ngx_php_debug("buf write in php var.");
            ZVAL_STRINGL(ctx->recv_buf, (char *)ctx->upstream->buffer.pos, ctx->upstream->buffer.last - ctx->upstream->buffer.pos);
        }

        ZVAL_STRING(&func_next, "next");
        call_user_function(NULL, closure, &func_next, &retval, 0, NULL TSRMLS_CC);
        zval_ptr_dtor(&func_next);

        ZVAL_STRING(&func_valid, "valid");
        call_user_function(NULL, closure, &func_valid, &retval, 0, NULL TSRMLS_CC);
        zval_ptr_dtor(&func_valid);

        ngx_php_debug("r:%p, closure:%p, retval:%d,%d", r, closure, Z_TYPE(retval), IS_TRUE);

        if (Z_TYPE(retval) == IS_TRUE) {
            ctx->phase_status = NGX_AGAIN;
        }else {
            ctx->phase_status = NGX_OK;
            ngx_http_core_run_phases(r);
            efree(ctx->generator_closure);
            ctx->generator_closure = NULL;
        }

    }zend_catch {

    }zend_end_try();
}

void 
ngx_http_php_zend_uthread_exit(ngx_http_request_t *r)
{
    ngx_http_php_ctx_t *ctx;

    ngx_php_request = r;

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {

    }

    if (ctx->generator_closure) {
        //ngx_http_php_zend_uthread_resume(r);
        ctx->phase_status = NGX_OK;
        ngx_http_core_run_phases(r);
        efree(ctx->generator_closure);
        ctx->generator_closure = NULL;
    }

}



