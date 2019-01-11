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

#ifndef __NGX_HTTP_PHP_MODULE_H__
#define __NGX_HTTP_PHP_MODULE_H__

#include <php.h>
#include <ngx_config.h>

#include "ngx_php_debug.h"
#include "ngx_http_php_core.h"
#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>

#if defined(NDK) && NDK
#include <ndk.h>
#endif


#define NGX_HTTP_PHP_MODULE_NAME "ngx_php"
#define NGX_HTTP_PHP_MODULE_VERSION  "0.0.15"

extern ngx_module_t ngx_http_php_module;
ngx_http_request_t *ngx_php_request;

typedef struct ngx_http_php_main_conf_s {

    ngx_str_t ini_path;
    ngx_http_php_code_t *init_code;
    ngx_http_php_code_t *init_inline_code;

    ngx_http_php_code_t *init_worker_inline_code;

    unsigned enabled_init_worker_handler:1;

    unsigned enabled_rewrite_handler:1;
    unsigned enabled_access_handler:1;
    unsigned enabled_content_handler:1;
    unsigned enabled_log_handler:1;

    unsigned enabled_opcode_handler:1;
    unsigned enabled_stack_handler:1;

    unsigned enabled_header_filter:1;
    unsigned enabled_body_filter:1;

    ngx_http_php_state_t *state;

} ngx_http_php_main_conf_t;

typedef struct ngx_http_php_loc_conf_s {
    ngx_str_t document_root;

    ngx_http_php_code_t *rewrite_code;
    ngx_http_php_code_t *rewrite_inline_code;
    ngx_http_php_code_t *access_code;
    ngx_http_php_code_t *access_inline_code;
    ngx_http_php_code_t *content_code;
    ngx_http_php_code_t *content_inline_code;
    ngx_http_php_code_t *log_code;
    ngx_http_php_code_t *log_inline_code;

    ngx_http_php_code_t *opcode_inline_code;
    ngx_http_php_code_t *stack_inline_code;

    ngx_http_php_code_t *header_filter_code;
    ngx_http_php_code_t *header_filter_inline_code;
    ngx_http_php_code_t *body_filter_code;
    ngx_http_php_code_t *body_filter_inline_code;

    ngx_int_t (*rewrite_handler)(ngx_http_request_t *r);
    ngx_int_t (*access_handler)(ngx_http_request_t *r);
    ngx_int_t (*content_handler)(ngx_http_request_t *r);
    ngx_int_t (*log_handler)(ngx_http_request_t *r);

    ngx_int_t (*opcode_handler)(ngx_http_request_t *r);
    ngx_int_t (*stack_handler)(ngx_http_request_t *r);

    ngx_http_handler_pt header_filter_handler;
    ngx_http_output_body_filter_pt body_filter_handler;

    unsigned enabled_rewrite_inline_compile:1;
    unsigned enabled_access_inline_compile:1;
    unsigned enabled_content_inline_compile:1;
    unsigned enabled_log_inline_compile:1;
    unsigned enabled_header_filter_inline_compile:1;
    unsigned enabled_body_filter_inline_compile:1;
    
    ngx_flag_t log_socket_errors;

} ngx_http_php_loc_conf_t;

#endif
