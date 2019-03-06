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

#include "../../ngx_http_php_module.h"
#include "../../ngx_http_php_socket.h"
#include "php_ngx_sockets.h"

//static int le_socket;
#define le_socket_name php_ngx_sockets_le_socket_name

//static int php_ngx_socket_le_socket(void);
static php_ngx_socket_t *php_ngx_socket_create(void);
//static void php_ngx_socket_destroy(zend_resource *rsrc);

/*static int php_ngx_socket_le_socket(void)
{
    return le_socket;
}*/

static php_ngx_socket_t *php_ngx_socket_create(void)
{
    php_ngx_socket_t    *php_sock;

    php_sock = emalloc(sizeof(php_ngx_socket_t));
    php_sock->type = 0;
    php_sock->error = 0;

    return php_sock;
}

/*static void php_ngx_socket_destroy(zend_resource *rsrc)
{
    php_ngx_socket_t  *php_sock;

    php_sock = rsrc->ptr;

    efree(php_sock);
    efree(rsrc);
}*/

PHP_FUNCTION(ngx_socket_create)
{
    zend_long       arg1, arg2, arg3;
    php_ngx_socket_t  *php_sock;

    ngx_http_request_t  *r;
    ngx_http_php_ctx_t  *ctx;

    arg1 = AF_INET;
    arg2 = SOCK_STREAM;
#ifndef SOL_TCP
#define SOL_TCP IPPROTO_TCP
#endif
    arg3 = SOL_TCP;
    php_sock = php_ngx_socket_create();

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|lll", &arg1, &arg2, &arg3) == FAILURE) {
        RETURN_FALSE;
    }

    php_sock->type = arg1;

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
    ctx->php_socket = php_sock;
    ngx_http_set_ctx(r, ctx, ngx_http_php_module);

    RETURN_TRUE;
}

PHP_FUNCTION(ngx_socket_connect)
{
    zval                *arg1;
    //php_ngx_socket_t    *php_sock;
    char                *addr;
    size_t              addr_len;
    zend_long           port = 0;
    //int                 retval;

    ngx_http_request_t  *r;
    ngx_http_php_ctx_t  *ctx;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "zs|l", &arg1, &addr, &addr_len, &port) == FAILURE) {
        RETURN_FALSE;
    }

    //php_sock = Z_RES_P(arg1)->ptr;

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if ( !ctx ) {
        RETURN_FALSE;
    } 

    switch(ctx->php_socket->type) {
        case AF_INET: {
            ctx->host.data = ngx_palloc(r->pool, addr_len + 1);
            ctx->host.len = addr_len;

            ngx_memcpy(ctx->host.data, (u_char *)addr, addr_len + 1);
            ctx->host.data[addr_len] = '\0';

            ctx->port = port;
            ngx_http_php_socket_connect(r);
            break;
        }

        default:
            //php_error_docref(NULL, E_WARNING, "Unsupported socket type %d", ctx->php_socket->type);
            RETURN_FALSE;
    }

    /*if (retval != NGX_OK) {
        php_error_docref(NULL, E_WARNING, "unable to connect");
        RETURN_FALSE;
    }*/

    RETURN_TRUE;
}

PHP_FUNCTION(ngx_socket_close)
{
    zval                *arg1;
    //php_ngx_socket_t    *php_sock;

    ngx_http_request_t  *r;
    ngx_http_php_ctx_t  *ctx;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &arg1) == FAILURE) {
        RETURN_FALSE;
    }

    /*if ((ngx_sock = (php_ngx_socket *)zend_fetch_resource(Z_RES_P(arg1), le_socket_name, le_socket)) == NULL) {
        RETURN_FALSE;
    }*/
    //php_sock = Z_RES_P(arg1)->ptr;
    //efree(php_sock);


    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);
    
    if ( !ctx ) {
        RETURN_FALSE;
    } 

    efree(ctx->php_socket);
    ctx->php_socket = NULL;

    ngx_http_php_socket_close(r);

    RETURN_TRUE;
}

PHP_FUNCTION(ngx_socket_send)
{
    zval                            *arg1;
    //php_ngx_socket_t                *php_sock;
    size_t                          buf_len, retval;
    zend_long                       len;
    char                            *buf;

    ngx_http_request_t              *r;
    ngx_http_php_ctx_t              *ctx;
    ngx_http_php_socket_upstream_t  *u;
    ngx_str_t                       ns;
    ngx_buf_t                       *b;
    ngx_chain_t                     *cl;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "zsl", &arg1, &buf, &buf_len, &len) == FAILURE) {
        RETURN_FALSE;
    }

    if (len < 0) {
        //php_error_docref(NULL, E_WARNING, "Length cannot be negative");
        RETURN_FALSE;
    }

    /*if ((ngx_sock = (php_ngx_socket *)zend_fetch_resource(Z_RES_P(arg1), le_socket_name, le_socket)) == NULL) {
        RETURN_FALSE;
    }*/
    //php_sock = Z_RES_P(arg1)->ptr;

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if ( !ctx ) {
        RETURN_FALSE;
    } 

    ns.data = (u_char *)buf;
    ns.len = buf_len;

    b = ngx_create_temp_buf(r->pool, ns.len + 1);

    cl = ngx_alloc_chain_link(r->pool);

    cl->buf = b;
    cl->next = NULL;

    u = ctx->upstream;
    u->request_bufs = cl;

    b->last = ngx_copy(b->last, ns.data, ns.len);

    retval = ngx_http_php_socket_send(r);

    RETURN_LONG(retval);
}

PHP_FUNCTION(ngx_socket_recv)
{
    zval                            *arg1, *buf;
    //zend_string                   *recv_buf;
    //php_ngx_socket_t                *php_sock;
    int                             retval;
    zend_long                       len = 1024;

    ngx_http_request_t              *r;
    ngx_http_php_ctx_t              *ctx;
    ngx_http_php_socket_upstream_t  *u;
    //ngx_buf_t                       *b;
    //long                            size = 1024;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "zz/|l", &arg1, &buf, &len) == FAILURE) {
        RETURN_FALSE;
    }

    /*if ((ngx_sock = (php_ngx_socket *)zend_fetch_resource(Z_RES_P(arg1), le_socket_name, le_socket)) == NULL) {
        RETURN_FALSE;
    }*/
    //php_sock = Z_RES_P(arg1)->ptr;

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ngx_php_debug("ctx: %p", ctx);
    if ( !ctx ) {
        // maybe memory leak.
        //zval_ptr_dtor(buf);
        //efree(buf);
        RETURN_FALSE;
    }

    u = ctx->upstream;
    u->buffer_size = len;
    //b = &u->buffer;

    ctx->recv_buf = buf;
    zval_ptr_dtor(ctx->recv_buf);

    retval = ngx_http_php_socket_recv(r);

    //ZVAL_STRINGL(buf, (char *)b->pos, b->last - b->pos);

    /*if (retval != NGX_OK) {
        php_error_docref(NULL, E_WARNING, "unable to read from socket");
        RETURN_FALSE;
    }*/

    RETURN_LONG(retval);
}

PHP_FUNCTION(ngx_socket_set_timeout)
{
    ngx_http_request_t              *r;
    ngx_http_php_ctx_t              *ctx;
    ngx_http_php_socket_upstream_t  *u;
    long                            timeout;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &timeout) == FAILURE) {
        RETURN_FALSE;
    }

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {
        RETURN_FALSE;
    }

    u = ctx->upstream;
    u->connect_timeout = (ngx_msec_t) timeout;
    u->read_timeout = (ngx_msec_t) timeout;
    u->write_timeout = (ngx_msec_t) timeout;

    RETURN_TRUE;
}

void php_impl_ngx_sockets_init(int module_number TSRMLS_DC)
{
    //le_socket = zend_register_list_destructors_ex(php_ngx_socket_destroy, NULL, le_socket_name, module_number);
}

