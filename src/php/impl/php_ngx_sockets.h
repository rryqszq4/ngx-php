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

#ifndef __PHP_NGX_SOCKETS_H__
#define __PHP_NGX_SOCKETS_H__

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <ngx_http.h>

#include "../../ngx_http_php_socket.h"

#define php_ngx_sockets_le_socket_name "ngx_socket"

typedef struct {
	//ngx_http_php_socket_upstream_t  *upstream;
	int type;
	int error;
} php_ngx_socket;

typedef struct php_ngx_socket_s {

	int type;
	int error;

} php_ngx_socket_t;

ZEND_BEGIN_ARG_INFO_EX(arginfo_ngx_socket_create, 0, 0, 0)
	ZEND_ARG_INFO(0, domain)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_INFO(0, protocol)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ngx_socket_connect, 0, 0, 2)
	ZEND_ARG_INFO(0, socket)
	ZEND_ARG_INFO(0, addr)
	ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ngx_socket_close, 0, 0, 1)
	ZEND_ARG_INFO(0, socket)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ngx_socket_send, 0, 0, 3)
	ZEND_ARG_INFO(0, socket)
	ZEND_ARG_INFO(0, buf)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ngx_socket_recv, 0, 0, 2)
	ZEND_ARG_INFO(0, socket)
	ZEND_ARG_INFO(1, buf)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ngx_socket_set_timeout, 0, 0, 1)
	ZEND_ARG_INFO(0, time)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ngx_socket_set_keepalive, 0, 0, 1)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

PHP_FUNCTION(ngx_socket_create);
PHP_FUNCTION(ngx_socket_connect);
PHP_FUNCTION(ngx_socket_send);
PHP_FUNCTION(ngx_socket_recv);
PHP_FUNCTION(ngx_socket_read);
PHP_FUNCTION(ngx_socket_write);
PHP_FUNCTION(ngx_socket_close);
PHP_FUNCTION(ngx_socket_set_timeout);
PHP_FUNCTION(ngx_socket_set_keepalive);

void php_impl_ngx_sockets_init(int module_number TSRMLS_DC);

#endif
