/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#include "php_ngx_sockets.h"
#include "../../ngx_http_php_module.h"
#include "../../ngx_http_php_socket.h"


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

ZEND_BEGIN_ARG_INFO_EX(arginfo_ngx_socket_recv, 0, 0, 0, 3)
	ZEND_ARG_INFO(0, socket)
	ZEND_ARG_INFO(0, buf)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO()

PHP_FUNCTION(ngx_socket_create)
{
	zend_long arg1, arg2, arg3;
}

PHP_FUNCTION(ngx_socket_connect)
{

}

PHP_FUNCTION(ngx_socket_close)
{

}

PHP_FUNCTION(ngx_socket_send)
{

}

PHP_FUNCTION(ngx_socket_recv)
{

}

