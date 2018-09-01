/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_SOCKETS_H_
#define _PHP_NGX_SOCKETS_H_

#include <ngx_http.h>

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>

PHP_FUNCTION(ngx_socket_create);
PHP_FUNCTION(ngx_socket_connect);
PHP_FUNCTION(ngx_socket_send);
PHP_FUNCTION(ngx_socket_recv);
PHP_FUNCTION(ngx_socket_read);
PHP_FUNCTION(ngx_socket_write);
PHP_FUNCTION(ngx_socket_close);

void php_impl_ngx_sockets_init(int module_number TSRMLS_DC);

#endif