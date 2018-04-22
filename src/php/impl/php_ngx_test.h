/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_TEST_H
#define _PHP_NGX_TEST_H

#include <ngx_http.h>

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>

PHP_METHOD(ngx_test_generator, __construct);
PHP_METHOD(ngx_test_generator, yield);
PHP_METHOD(ngx_test_generator, send);
PHP_METHOD(ngx_test_generator, receive);

void php_impl_ngx_test_init(int module_number TSRMLS_DC);

#endif