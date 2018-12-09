/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_SOCKET_H_
#define _PHP_NGX_SOCKET_H_

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <ngx_http.h>

PHP_METHOD(ngx_socket, create);
PHP_METHOD(ngx_socket, connect);
PHP_METHOD(ngx_socket, send);
PHP_METHOD(ngx_socket, recv);
PHP_METHOD(ngx_socket, close);
PHP_METHOD(ngx_socket, settimeout);
PHP_METHOD(ngx_socket, setkeepalive);

void php_impl_ngx_socket_init(int module_number TSRMLS_DC);

#endif
