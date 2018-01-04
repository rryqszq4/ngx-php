/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_GENERATOR_H_
#define _PHP_NGX_GENERATOR_H_

#include <ngx_http.h>

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <zend_closures.h>

extern ZEND_API zend_class_entry *zend_ce_generator;

PHP_METHOD(ngx_generator, run);
PHP_METHOD(ngx_generator, next);

PHP_METHOD(ngx_php, main);
PHP_METHOD(ngx_php, next);

void php_ngx_generator_init(int module_number TSRMLS_DC);

#endif