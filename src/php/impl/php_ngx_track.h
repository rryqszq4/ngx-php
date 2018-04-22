/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_TRACK_H_
#define _PHP_NGX_TRACK_H_

#include <php.h>
#include <SAPI.h>
#include <php_main.h>
#include <php_variables.h>
#include <php_ini.h>
#include <zend_ini.h>
#include <zend_exceptions.h>
#include <zend_generators.h>
#include <ext/standard/php_standard.h>
#include <ext/standard/info.h>

zend_op_array* (*ori_compile_file)(zend_file_handle* file_handle, int type TSRMLS_DC);
zend_op_array* ngx_compile_file(zend_file_handle* file_handle, int type TSRMLS_DC);

zend_op_array* (*ori_compile_string)(zval *source_string, char *filename TSRMLS_DC);
zend_op_array* ngx_compile_string(zval *source_string, char *filename TSRMLS_DC);

void (*ori_execute_ex)(zend_execute_data *execute_data TSRMLS_DC);
void ngx_execute_ex(zend_execute_data *execute_data TSRMLS_DC);
void ngx_execute_internal(zend_execute_data *execute_data, zval *return_value TSRMLS_DC);

void ngx_track_zend_execute_data(zend_execute_data *execute_data);
void ngx_track_zend_generator(zend_generator *generator, int tabs_len);
void ngx_track_zend_generator_node(zend_generator_node *node, int tabs_len);

#endif