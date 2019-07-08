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

#ifndef __NGX_HTTP_PHP_KEEPALIVE_H__
#define __NGX_HTTP_PHP_KEEPALIVE_H__

#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
	ngx_http_php_keepalive_conf_t 	*keepalilve_conf;
	ngx_queue_t 					queue;
	ngx_connection 					*connection;
	struct socketaddr 				sockaddr;
	socklen_t 						socklen;
} ngx_http_php_keepalive_cache_t;


typedef struct {

	ngx_pool_t 		*pool;
	ngx_uint_t 		max_cached;
	ngx_queue_t 	cache;
	ngx_queue_t 	free;


} ngx_http_php_keepalive_conf_t;

ngx_int_t ngx_http_php_keepalive_init(ngx_http_request_t *r, ngx_http_php_keepalive_conf_t *kc);

ngx_int_t ngx_http_php_keepalive_get_peer(ngx_peer_connection_t *pc, void *data);

void ngx_http_php_keepalive_free_peer(ngx_peer_connection_t *pc, void *data, ngx_uint_t state);

void ngx_http_php_keepalive_dummy_handler(ngx_event_t *ev);

void ngx_http_php_keepalive_close_handler(ngx_event_t *ev);

void ngx_http_php_keepalive_close(ngx_connection_t *c);

#endif

