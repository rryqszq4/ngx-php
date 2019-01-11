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

#ifndef __PHP_NGX_REQUEST_H__
#define __PHP_NGX_REQUEST_H__

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <ngx_http.h>

ZEND_BEGIN_ARG_INFO_EX(ngx_request_method_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_request_document_root_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_request_document_uri_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_request_script_name_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_request_script_filename_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_request_query_string_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_request_request_uri_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_request_server_protocol_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_request_remote_addr_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_request_server_addr_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_request_remote_port_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_request_server_port_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_request_server_name_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_request_headers_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_FUNCTION(ngx_request_method);
PHP_FUNCTION(ngx_request_document_root);
PHP_FUNCTION(ngx_request_document_uri);
PHP_FUNCTION(ngx_request_script_name);
PHP_FUNCTION(ngx_request_script_filename);
PHP_FUNCTION(ngx_request_query_string);
PHP_FUNCTION(ngx_request_uri);
PHP_FUNCTION(ngx_request_server_protocol);
PHP_FUNCTION(ngx_request_remote_addr);
PHP_FUNCTION(ngx_request_server_addr);
PHP_FUNCTION(ngx_request_remote_port);
PHP_FUNCTION(ngx_request_server_port);
PHP_FUNCTION(ngx_request_server_name);
PHP_FUNCTION(ngx_request_headers);

PHP_METHOD(ngx_request, method);
PHP_METHOD(ngx_request, document_root);
PHP_METHOD(ngx_request, document_uri);
PHP_METHOD(ngx_request, script_name);
PHP_METHOD(ngx_request, script_filename);
PHP_METHOD(ngx_request, query_string);
PHP_METHOD(ngx_request, request_uri);
PHP_METHOD(ngx_request, server_protocol);
PHP_METHOD(ngx_request, remote_addr);
PHP_METHOD(ngx_request, server_addr);
PHP_METHOD(ngx_request, remote_port);
PHP_METHOD(ngx_request, server_port);
PHP_METHOD(ngx_request, server_name);
PHP_METHOD(ngx_request, headers);

void php_impl_ngx_request_init(int module_number TSRMLS_DC);

#endif
