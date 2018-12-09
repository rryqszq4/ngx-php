/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_SOCKETS_H_
#define _PHP_NGX_SOCKETS_H_

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <ngx_http.h>

#include "../../ngx_http_php_socket.h"

#define php_ngx_sockets_le_socket_name "ngx_socket"

typedef struct {
	ngx_http_php_socket_upstream_t  *upstream;
	int type;
	int error;
} php_ngx_socket;

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

PHP_FUNCTION(ngx_socket_create);
PHP_FUNCTION(ngx_socket_connect);
PHP_FUNCTION(ngx_socket_send);
PHP_FUNCTION(ngx_socket_recv);
PHP_FUNCTION(ngx_socket_read);
PHP_FUNCTION(ngx_socket_write);
PHP_FUNCTION(ngx_socket_close);

void php_impl_ngx_sockets_init(int module_number TSRMLS_DC);

#endif
