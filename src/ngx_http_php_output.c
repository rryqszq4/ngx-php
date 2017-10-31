/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#include "ngx_http_php_module.h"
#include "ngx_http_php_request.h"
#include "ngx_http_php_output.h"

void
ngx_http_php_set_output_chain(ngx_http_request_t *r, char *buffer, int buffer_len)
{
    ngx_buf_t *b;
    ngx_http_php_rputs_chain_list_t *chain;
    ngx_http_php_ctx_t *ctx;

    u_char *u_str;
    ngx_str_t ns;

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ns.data = (u_char *)buffer;
    ns.len = buffer_len;

    if (ctx->rputs_chain == NULL){
        chain = ngx_pcalloc(r->pool, sizeof(ngx_http_php_rputs_chain_list_t));
        chain->out = ngx_alloc_chain_link(r->pool);
        chain->last = &chain->out;
    }else {
        chain = ctx->rputs_chain;
        (*chain->last)->next = ngx_alloc_chain_link(r->pool);
        chain->last = &(*chain->last)->next;
    }

    b = ngx_calloc_buf(r->pool);
    (*chain->last)->buf = b;
    (*chain->last)->next = NULL;

    u_str = ngx_pstrdup(r->pool, &ns);
    //u_str[ns.len] = '\0';
    (*chain->last)->buf->pos = u_str;
    (*chain->last)->buf->last = u_str + ns.len;
    (*chain->last)->buf->memory = 1;
    ctx->rputs_chain = chain;
    ngx_http_set_ctx(r, ctx, ngx_http_php_module);

    if (r->headers_out.content_length_n == -1){
        r->headers_out.content_length_n += ns.len + 1;
    }else {
        r->headers_out.content_length_n += ns.len;
    }
}

void 
ngx_http_php_check_output_chain_empty(ngx_http_request_t)
{
    ngx_http_php_rputs_chain_list_t *chain;
    ngx_http_php_ctx_t *ctx;

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    chain = ctx->rputs_chain;

    if (ctx->rputs_chain == NULL){
        ngx_buf_t *b;
        ngx_str_t ns;
        u_char *u_str;
        ns.data = (u_char *)" ";
        ns.len = 1;
        
        chain = ngx_pcalloc(r->pool, sizeof(ngx_http_php_rputs_chain_list_t));
        chain->out = ngx_alloc_chain_link(r->pool);
        chain->last = &chain->out;
    
        b = ngx_calloc_buf(r->pool);
        (*chain->last)->buf = b;
        (*chain->last)->next = NULL;

        u_str = ngx_pstrdup(r->pool, &ns);
        //u_str[ns.len] = '\0';
        (*chain->last)->buf->pos = u_str;
        (*chain->last)->buf->last = u_str + ns.len;
        (*chain->last)->buf->memory = 1;
        ctx->rputs_chain = chain;

        if (r->headers_out.content_length_n == -1){
            r->headers_out.content_length_n += ns.len + 1;
        }else {
            r->headers_out.content_length_n += ns.len;
        }
    }
}