
/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#include "ngx_http_php_module.h"
#include "ngx_http_php_request.h"
#include "ngx_http_php_core.h"
#include "ngx_http_php_handler.h"

int 
ngx_http_php_request_read_body(ngx_http_request_t *r)
{
    ngx_int_t rc;
    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (r->method != NGX_HTTP_POST && r->method != NGX_HTTP_PUT){
        //php_error(E_WARNING, "can't read body");
        return NGX_OK;
    }

    r->request_body_in_single_buf = 1;
    rc = ngx_http_read_client_request_body(r, ngx_http_php_request_read_body_cb);

    if (rc == NGX_ERROR || rc >= NGX_HTTP_SPECIAL_RESPONSE){
        //php_error(E_WARNING, "ngx_http_read_client_request_body failed");
        return rc;
    }

    if (rc == NGX_AGAIN){
        ctx->request_body_more = 1;

        return NGX_DONE;
    }

    return rc;
}

void 
ngx_http_php_request_read_body_cb(ngx_http_request_t *r)
{
    ngx_chain_t *cl;
    size_t len;
    u_char *p;
    u_char *buf;

    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (r->request_body == NULL || r->request_body->bufs == NULL){
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "This pahse don't have request_body");
        ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
        return ;
    }

    if (r->request_body->temp_file){
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "temp_file: %s", r->request_body->temp_file->file.name.data);
        ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
    }

    cl = r->request_body->bufs;

    if (cl->next == NULL){
        len = cl->buf->last - cl->buf->pos;
        if (len == 0){
            return ;
        }

        ctx->request_body_ctx.data = cl->buf->pos;
        ctx->request_body_ctx.len = len;
        ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, "request_body(%d|%d): %V", len, strlen((char *)ctx->request_body_ctx.data), &ctx->request_body_ctx);
        if (ctx->request_body_more){
            ctx->request_body_more = 0;
            ngx_http_finalize_request(r, ngx_http_php_content_post_handler(r));
        }else {
            ngx_http_finalize_request(r, NGX_DONE);
        }
        return ;
    }

    len = 0;
    for (; cl; cl = cl->next){
        len += cl->buf->last - cl->buf->pos;
    }

    if (len == 0){
        return ;
    }

    buf = ngx_palloc(r->pool, len);
    p = buf;
    for (cl = r->request_body->bufs; cl; cl = cl->next){
        p = ngx_copy(p, cl->buf->pos, cl->buf->last - cl->buf->pos);
    }

    ctx->request_body_ctx.data = buf;
    ctx->request_body_ctx.len = len;
    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, "multi request_body(%d|%d): %V", len, strlen((char *)ctx->request_body_ctx.data), &ctx->request_body_ctx);
    if (ctx->request_body_more){
        ctx->request_body_more = 0;
        ngx_http_finalize_request(r, ngx_http_php_content_post_handler(r));
    }else {
        ngx_http_finalize_request(r, NGX_DONE);
    }

    return ;

    /*ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
    u_char *p;
    u_char *data;
    size_t len;
    ngx_buf_t *buf, *next;
    ngx_chain_t *cl;
    if (r->request_body == NULL || r->request_body->bufs == NULL){
        ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
        return ;
    }
    if (r->request_body->temp_file){
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "temp_file: %s", r->request_body->temp_file->file.name.data);
        //body = r->request_body->temp_file->file.name;
        ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
        return ;
    }else {
        cl = r->request_body->bufs;
        buf = cl->buf;
        if (cl->next == NULL){
            len = buf->last - buf->pos;
            p = ngx_pnalloc(r->pool, len + 1);
            if (p == NULL){
                ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
                return ;
            }
            data = p;
            ngx_memcpy(p, buf->pos, len);
            data[len] = 0;
        } else {
            next = cl->next->buf;
            len = (buf->last - buf->pos) + (next->last - next->pos);
            p = ngx_pnalloc(r->pool, len + 1);
            data = p;
            if (p == NULL){
                ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
                return ;
            }
            p = ngx_cpymem(p, buf->pos, buf->last - buf->pos);
            ngx_memcpy(p, next->pos, next->last - next->pos);
            data[len] = 0;
        }
    }
    ctx->request_body_ctx.data = data;
    ctx->request_body_ctx.len = len;
    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, "multi request_body(%d|%d): %s", len, strlen((char *)ctx->request_body_ctx.data), ctx->request_body_ctx.data);
    if (ctx->request_body_more){
        ctx->request_body_more = 0;
        ngx_http_finalize_request(r, ngx_http_php_content_post_handler(r));
    }else {
        ngx_http_finalize_request(r, NGX_DONE);
    }*/
}

ngx_str_t 
ngx_http_php_request_get_request_body(ngx_http_request_t *r)
{
    u_char *p;
    u_char *data;
    size_t len;
    ngx_buf_t *buf, *next;
    ngx_chain_t *cl;
    ngx_str_t body = ngx_null_string;

    if (r->request_body == NULL || r->request_body->bufs == NULL){
        return body;
    }

    if (r->request_body->temp_file){
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "temp_file: %s", r->request_body->temp_file->file.name.data);
        body = r->request_body->temp_file->file.name;
        return body;
    }else {
        cl = r->request_body->bufs;
        buf = cl->buf;

        if (cl->next == NULL){
            len = buf->last - buf->pos;
            p = ngx_pnalloc(r->pool, len + 1);
            if (p == NULL){
                return body;
            }
            data = p;
            ngx_memcpy(p, buf->pos, len);
            data[len] = 0;
        } else {
            next = cl->next->buf;
            len = (buf->last - buf->pos) + (next->last - next->pos);
            p = ngx_pnalloc(r->pool, len + 1);
            data = p;
            if (p == NULL){
                return body;
            }
            p = ngx_cpymem(p, buf->pos, buf->last - buf->pos);
            ngx_memcpy(p, next->pos, next->last - next->pos);
            data[len] = 0;
        }
    }

    body.len = len;
    body.data = data;
    return body;
}

void 
ngx_http_php_request_init(ngx_http_request_t *r TSRMLS_DC)
{
    ngx_http_headers_in_t *headers_in;
    headers_in = &r->headers_in;

    ngx_list_part_t *part = &r->headers_in.headers.part;
    ngx_table_elt_t *header = part->elts;

    ngx_uint_t i;

    if (r->method == NGX_HTTP_GET){
        SG(request_info).request_method = "GET";
    } else if (r->method == NGX_HTTP_POST){
        SG(request_info).request_method = "POST";

        char *content_type = (char *)headers_in->content_type->value.data;
        SG(request_info).content_type = content_type;
    }

    if (r->args.len > 0){
        SG(request_info).query_string = emalloc(r->args.len+1);
        ngx_cpystrn((u_char *)SG(request_info).query_string, r->args.data, r->args.len+1);

    }

    ngx_http_php_request_context_t *context;
    context = emalloc(sizeof(ngx_http_php_request_context_t));
    context->cookie_data = NULL;

    for (i = 0; /* void */; i++){
        if (i >= part->nelts){
            if (part->next == NULL){
                break;
            }
            part = part->next;
            header = part->elts;
            i = 0;
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"cookie", header[i].key.len) == 0){
            context->cookie_data = emalloc(header[i].value.len + 1);
            ngx_cpystrn((u_char *)context->cookie_data, header[i].value.data, header[i].value.len + 1);
        }

    }

    //context->r = r;

    SG(server_context) = context;

    //ngx_http_php_request_read_body(r);
}

void 
ngx_http_php_request_clean(TSRMLS_D)
{
    if (SG(request_info).query_string){
        efree(SG(request_info).query_string);
        SG(request_info).query_string = NULL;
    }

    if (SG(server_context)){
        ngx_http_php_request_context_t *context;
        context = SG(server_context);
        if (context->cookie_data){
            efree(context->cookie_data);
            context->cookie_data = NULL;
        }
        
        efree(SG(server_context));
        SG(server_context) = NULL;
    }

}
