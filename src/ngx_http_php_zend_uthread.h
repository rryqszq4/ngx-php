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