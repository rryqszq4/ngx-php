/*
==============================================================================
Copyright (c) 2016-2019, rryqszq4 <rryqszq@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
==============================================================================
*/

#ifndef __PHP_NGX_TRACK_H__
#define __PHP_NGX_TRACK_H__

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