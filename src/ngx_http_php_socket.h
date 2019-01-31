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

#ifndef __NGX_HTTP_PHP_SOCKET_H__
#define __NGX_HTTP_PHP_SOCKET_H__

#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>
#include <ngx_http.h>


/*typedef struct ngx_http_php_socket_pool_s {

};*/

typedef struct ngx_http_php_socket_upstream_s
        ngx_http_php_socket_upstream_t;

typedef void (*ngx_http_php_socket_upstream_handler_pt)(ngx_http_request_t *r, 
    ngx_http_php_socket_upstream_t *u);

struct ngx_http_php_socket_upstream_s {

    ngx_http_php_socket_upstream_handler_pt     read_event_handler;
    ngx_http_php_socket_upstream_handler_pt     write_event_handler;

    ngx_msec_t      connect_timeout;
    ngx_msec_t      read_timeout;
    ngx_msec_t      send_timeout;

    ngx_http_cleanup_pt     *cleanup;
    ngx_http_request_t      *request;
    ngx_peer_connection_t   peer;

    ngx_http_upstream_resolved_t    *resolved;

    ngx_buf_t       buffer;
    size_t          buffer_size;
    off_t           length;
    unsigned        enabled_receive:1;

    ngx_chain_t     *bufs_in;

    ngx_chain_t     *busy_bufs;
    ngx_chain_t     *free_bufs;

    ngx_err_t       socket_errno;

    ngx_int_t       (*input_filter)(void *data, ssize_t bytes);

    size_t          request_len;
    ngx_chain_t     *request_bufs;
    

};

ngx_int_t ngx_http_php_socket_connect(ngx_http_request_t *r);
void ngx_http_php_socket_close(ngx_http_request_t *r);

ngx_int_t ngx_http_php_socket_send(ngx_http_request_t *r);
ngx_int_t ngx_http_php_socket_recv(ngx_http_request_t *r);

void ngx_http_php_socket_clear(ngx_http_request_t *r);

#endif