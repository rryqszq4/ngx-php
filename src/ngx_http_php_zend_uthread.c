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
#include "ngx_http_php_zend_uthread.h"
#include "ngx_http_php_util.h"

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

#if 0 && (NGX_DEBUG)
    if (plcf->content_inline_code->code_id.data != NULL) {
        ngx_pfree(r->pool, plcf->content_inline_code->code_id.data);

        plcf->content_inline_code->code_id.data = ngx_pnalloc(r->pool, 32);
    if (plcf->content_inline_code->code_id.data == NULL) {
        // todo error log
        return ;
    }
    ngx_sprintf(plcf->content_inline_code->code_id.data, "%08xD%08xD%08xD%08xD",
                (uint32_t) ngx_random(), (uint32_t) ngx_random(),
                (uint32_t) ngx_random(), (uint32_t) ngx_random());
    }
#endif

    inline_code.data = ngx_pnalloc(r->pool, sizeof("function ngx_content_(){  }")-1 + ngx_strlen(plcf->content_inline_code->code.string) + 32);

    inline_code.len = ngx_sprintf(inline_code.data, "function ngx_content_%V(){ %*s }", 
                                        &(plcf->content_inline_code->code_id), 
                                        ngx_strlen(plcf->content_inline_code->code.string),
                                        plcf->content_inline_code->code.string
                                    ) - inline_code.data;

    ngx_php_debug("%*s, %d", (int)inline_code.len, inline_code.data, (int)inline_code.len);

    zend_first_try {
#if 0 && (NGX_DEBUG)
        ngx_http_php_zend_eval_stringl_ex(
            (char *)inline_code.data, 
            inline_code.len, 
            NULL, 
            "ngx_php eval code", 
            1
        );
            plcf->enabled_content_inline_compile = 1;
#else
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
#endif        
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
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ngx_php ctx is nil at zend_uthread_create");
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
                php_error_docref(NULL , E_WARNING, "Failed calling valid");
                return ;
            }
            zval_ptr_dtor(&func_valid);

            ngx_php_debug("r:%p, closure:%p, retval:%d", r, ctx->generator_closure, Z_TYPE(retval));

            if (Z_TYPE(retval) == IS_TRUE){
                /*
                ZVAL_STRING(&func_next, "next");

                ngx_http_php_call_user_function(NULL, ctx->generator_closure, &func_next, &retval, 0, NULL );

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
        if ( ngx_http_php_call_user_function(NULL, closure, &func_next, &retval, 0, NULL ) == FAILURE )
        {
            php_error_docref(NULL , E_WARNING, "Failed calling next");
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
        if ( ngx_http_php_call_user_function(NULL, closure, &func_valid, &retval, 0, NULL ) == FAILURE )
        {
            php_error_docref(NULL , E_WARNING, "Failed calling valid");
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

    if (ctx == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ngx_php ctx is nil at zend_uthread_exit");
        return ;
    }

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



