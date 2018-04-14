/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_VAR_H_
#define _PHP_NGX_VAR_H_

#include <ngx_http.h>
 
#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>

PHP_METHOD(ngx_var, get);
PHP_METHOD(ngx_var, set);

void php_impl_ngx_var_init(int module_number TSRMLS_DC);

#endif