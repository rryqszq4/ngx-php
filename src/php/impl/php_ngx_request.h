/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_REQUEST_H_
#define _PHP_NGX_REQUEST_H_

#include <ngx_http.h>

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>

PHP_METHOD(ngx_request, method);
PHP_METHOD(ngx_request, document_root);
PHP_METHOD(ngx_request, document_uri);
PHP_METHOD(ngx_request, script_name);
PHP_METHOD(ngx_request, script_filename);
PHP_METHOD(ngx_request, query_string);
PHP_METHOD(ngx_request, request_uri);
PHP_METHOD(ngx_request, server_protocol);
PHP_METHOD(ngx_request, remote_addr);
PHP_METHOD(ngx_request, server_addr);
PHP_METHOD(ngx_request, remote_port);
PHP_METHOD(ngx_request, server_port);
PHP_METHOD(ngx_request, server_name);
PHP_METHOD(ngx_request, headers);

void php_impl_ngx_request_init(int module_number TSRMLS_DC);

#endif