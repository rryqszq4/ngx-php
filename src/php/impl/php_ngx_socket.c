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
#include "php_ngx_socket.h"

static zend_class_entry *php_ngx_socket_class_entry;

ZEND_BEGIN_ARG_INFO_EX(ngx_socket_create_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_socket_connect_arginfo, 0, 0, 3)
    ZEND_ARG_INFO(0, socket)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_socket_send_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, socket)
    ZEND_ARG_INFO(0, buf)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_socket_recv_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, socket)
    ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_socket_close_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, socket)
ZEND_END_ARG_INFO()


PHP_METHOD(ngx_socket, create)
{

}

PHP_METHOD(ngx_socket, connect)
{
    ngx_http_request_t *r;
    ngx_http_php_ctx_t *ctx;

    zend_string *host_str;
    long port;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Sl", &host_str, &port) == FAILURE){
        RETURN_NULL();
    }

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ctx->host.data = ngx_palloc(r->pool, ZSTR_LEN(host_str) + 1);
    ctx->host.len = ZSTR_LEN(host_str);

    ngx_memcpy(ctx->host.data, (u_char *)ZSTR_VAL(host_str), ZSTR_LEN(host_str) + 1);
    ctx->host.data[ZSTR_LEN(host_str)] = '\0';

    ctx->port = port;

    ngx_http_php_socket_connect(r);

}

PHP_METHOD(ngx_socket, send)
{
    ngx_http_request_t              *r;
    ngx_http_php_ctx_t              *ctx;
    ngx_http_php_socket_upstream_t  *u;
    ngx_str_t                       ns;
    ngx_buf_t                       *b;
    ngx_chain_t                     *cl;

    zend_string *buf_str;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S", &buf_str) == FAILURE) {
        RETURN_NULL();
    }

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ns.data = (u_char *)ZSTR_VAL(buf_str);
    ns.len = ZSTR_LEN(buf_str);

    b = ngx_create_temp_buf(r->pool, ns.len + 1);

    cl = ngx_alloc_chain_link(r->pool);

    cl->buf = b;
    cl->next = NULL;

    u = ctx->upstream;
    u->request_bufs = cl;

    b->last = ngx_copy(b->last, ns.data, ns.len);

    ngx_http_php_socket_send(r);

}

PHP_METHOD(ngx_socket, recv)
{
    ngx_http_request_t              *r;
    ngx_http_php_ctx_t              *ctx;
    ngx_http_php_socket_upstream_t  *u;
    ngx_buf_t                       *b;

    long size = 1024;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &size) == FAILURE){
        RETURN_NULL();
    }

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    u = ctx->upstream;
    u->buffer_size = size;
    b = &u->buffer;

    ngx_http_php_socket_recv(r);

    ZVAL_STRINGL(return_value, (char *)b->pos, b->last - b->pos);

    u->enabled_receive = 0;
}

PHP_METHOD(ngx_socket, close)
{
    ngx_http_request_t *r;
    //ngx_http_php_ctx_t *ctx;

    r = ngx_php_request;
    //ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ngx_http_php_socket_close(r);
}

static const zend_function_entry php_ngx_socket_class_functions[] = {
    PHP_ME(ngx_socket, create, ngx_socket_create_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_socket, connect, ngx_socket_connect_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_socket, send, ngx_socket_send_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_socket, recv, ngx_socket_recv_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_socket, close, ngx_socket_close_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    {NULL, NULL, NULL, 0, 0}
};

void
php_impl_ngx_socket_init(int module_number TSRMLS_DC)
{
    zend_class_entry ngx_socket_class_entry;
    INIT_CLASS_ENTRY(ngx_socket_class_entry, "ngx_socket", php_ngx_socket_class_functions);
    php_ngx_socket_class_entry = zend_register_internal_class(&ngx_socket_class_entry TSRMLS_CC);
}
