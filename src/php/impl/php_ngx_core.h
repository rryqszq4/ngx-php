/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_CORE_H_
#define _PHP_NGX_CORE_H_

#include <ngx_http.h>

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>

PHP_FUNCTION(ngx_sleep);

PHP_METHOD(ngx, _exit);
PHP_METHOD(ngx, query_args);
PHP_METHOD(ngx, post_args);
PHP_METHOD(ngx, sleep);

void php_impl_ngx_core_init(int module_number TSRMLS_DC);

#endif