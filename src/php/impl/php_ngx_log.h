/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_LOG_H_
#define _PHP_NGX_LOG_H_

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <ngx_http.h>

ZEND_BEGIN_ARG_INFO_EX(ngx_log_error_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, level)
    ZEND_ARG_INFO(0, log)
ZEND_END_ARG_INFO()

PHP_FUNCTION(ngx_log_error);

PHP_METHOD(ngx_log, error);

void php_impl_ngx_log_init(int module_number TSRMLS_DC);

#endif
