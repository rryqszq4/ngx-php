/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_CORE_H_
#define _PHP_NGX_CORE_H_

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <ngx_http.h>

ZEND_BEGIN_ARG_INFO_EX(ngx_exit_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, status)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_query_args_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_post_args_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_sleep_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, time)
ZEND_END_ARG_INFO()

PHP_FUNCTION(ngx_exit);
PHP_FUNCTION(ngx_query_args);
PHP_FUNCTION(ngx_post_args);
PHP_FUNCTION(ngx_sleep);

PHP_METHOD(ngx, _exit);
PHP_METHOD(ngx, query_args);
PHP_METHOD(ngx, post_args);
PHP_METHOD(ngx, sleep);

void php_impl_ngx_core_init(int module_number TSRMLS_DC);

#endif
