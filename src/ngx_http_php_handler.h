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

#ifndef __NGX_HTTP_PHP_HANDLER_H__
#define __NGX_HTTP_PHP_HANDLER_H__

#include <nginx.h>
#include <ngx_http.h>

#include "ngx_http_php_module.h"

// handler
//ngx_int_t ngx_http_php_init_handler(ngx_conf_t *cf, ngx_http_php_main_conf_t *pmcf);

ngx_int_t ngx_http_php_post_read_handler(ngx_http_request_t *r);
void ngx_http_php_request_cleanup_handler(void *data);

ngx_int_t ngx_http_php_rewrite_handler(ngx_http_request_t *r);
ngx_int_t ngx_http_php_rewrite_file_handler(ngx_http_request_t *r);
ngx_int_t ngx_http_php_rewrite_inline_handler(ngx_http_request_t *r);

ngx_int_t ngx_http_php_access_handler(ngx_http_request_t *r);
ngx_int_t ngx_http_php_access_file_handler(ngx_http_request_t *r);
ngx_int_t ngx_http_php_access_inline_handler(ngx_http_request_t *r);

ngx_int_t ngx_http_php_content_handler(ngx_http_request_t *r);
ngx_int_t ngx_http_php_content_file_handler(ngx_http_request_t *r);
ngx_int_t ngx_http_php_content_inline_handler(ngx_http_request_t *r);
ngx_int_t ngx_http_php_content_post_handler(ngx_http_request_t *r);

ngx_int_t ngx_http_php_opcode_handler(ngx_http_request_t *r);
ngx_int_t ngx_http_php_opcode_inline_handler(ngx_http_request_t *r);

ngx_int_t ngx_http_php_stack_handler(ngx_http_request_t *r);
ngx_int_t ngx_http_php_stack_inline_handler(ngx_http_request_t *r);

ngx_int_t ngx_http_php_log_handler(ngx_http_request_t *r);
ngx_int_t ngx_http_php_log_file_handler(ngx_http_request_t *r);
ngx_int_t ngx_http_php_log_inline_handler(ngx_http_request_t *r);

// filter functions
void ngx_http_php_header_filter_init(void);
void ngx_http_php_body_filter_init(void);

ngx_int_t ngx_http_php_header_filter(ngx_http_request_t *r);
ngx_int_t ngx_http_php_header_filter_inline_handler(ngx_http_request_t *r);

ngx_int_t ngx_http_php_body_filter(ngx_http_request_t *r, ngx_chain_t *in);
ngx_int_t ngx_http_php_body_filter_inline_handler(ngx_http_request_t *r, ngx_chain_t *in);

#endif