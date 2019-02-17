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

#ifndef __NGX_HTTP_PHP_ZEND_UTHREAD_H__
#define __NGX_HTTP_PHP_ZEND_UTHREAD_H__

#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>

#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 3
#define zend_string_release_ex(s, persistent) \
	zend_string_release(s)
#endif

#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 2
#define zend_init_func_execute_data(ex, op_array, return_value) \
	zend_init_execute_data(ex, op_array, return_value)
#endif

#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 1
#define ngx_http_php_call_user_function(function_table, object, function_name, retval_ptr, param_count, params) \
	call_user_function(function_table, object, function_name, retval_ptr, param_count, params)
#else
#define ngx_http_php_call_user_function(function_table, object, function_name, retval_ptr, param_count, params) \
	ngx_http_php__call_user_function_ex(object, function_name, retval_ptr, param_count, params, 1)
#define ngx_http_php_call_user_function_ex(function_table, object, function_name, retval_ptr, param_count, params, no_separation, symbol_table) \
	ngx_http_php__call_user_function_ex(object, function_name, retval_ptr, param_count, params, no_separation)
#endif

void ngx_http_php_zend_uthread_rewrite_inline_routine(ngx_http_request_t *r);
void ngx_http_php_zend_uthread_access_inline_routine(ngx_http_request_t *r);
void ngx_http_php_zend_uthread_content_inline_routine(ngx_http_request_t *r);
void ngx_http_php_zend_uthread_log_inline_routine(ngx_http_request_t *r);
void ngx_http_php_zend_uthread_header_filter_inline_routine(ngx_http_request_t *r);
void ngx_http_php_zend_uthread_body_filter_inline_routine(ngx_http_request_t *r);

void ngx_http_php_zend_uthread_file_routine(ngx_http_request_t *r);

void ngx_http_php_zend_uthread_create(ngx_http_request_t *r, char *func_prefix);

void ngx_http_php_zend_uthread_resume(ngx_http_request_t *r);

void ngx_http_php_zend_uthread_exit(ngx_http_request_t *r);

#endif