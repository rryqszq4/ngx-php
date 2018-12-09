/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_VAR_H_
#define _PHP_NGX_VAR_H_

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <ngx_http.h>

ZEND_BEGIN_ARG_INFO_EX(ngx_var_get_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_var_set_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_FUNCTION(ngx_var_get);
PHP_FUNCTION(ngx_var_set);

PHP_METHOD(ngx_var, get);
PHP_METHOD(ngx_var, set);

void php_impl_ngx_var_init(int module_number TSRMLS_DC);

#endif
