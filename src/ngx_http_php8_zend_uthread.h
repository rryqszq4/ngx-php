/*
==============================================================================
Copyright (c) 2016-2020, rryqszq4 <rryqszq@gmail.com>
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

#ifndef __NGX_HTTP_PHP8_ZEND_UTHREAD_H__
#define __NGX_HTTP_PHP8_ZEND_UTHREAD_H__

#if PHP_MAJOR_VERSION >= 8

#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>

#define ngx_http_php_call_user_function(function_table, object, function_name, retval_ptr, param_count, params) \
	ngx_http_php__call_user_function_impl(object, function_name, retval_ptr, param_count, params, NULL)

#define ngx_http_php_call_user_function_named(function_table, object, function_name, retval_ptr, param_count, params, named_params) \
	ngx_http_php__call_user_function_impl(object, function_name, retval_ptr, param_count, params, named_params)

zend_execute_data *zend_vm_stack_copy_call_frame(zend_execute_data *call, uint32_t passed_args, uint32_t additional_args);

int ngx_http_php__call_user_function_impl(zval *object, zval *function_name, zval *retval_ptr, uint32_t param_count, zval params[], HashTable *named_params);

#endif

#endif