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

#ifndef __NGX_HTTP_PHP_DIRECTIVE_H__
#define __NGX_HTTP_PHP_DIRECTIVE_H__

#include <ngx_conf_file.h>
#include <nginx.h>


char *ngx_http_php_ini_path(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

char *ngx_http_php_init_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_http_php_init_file_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

char *ngx_http_php_init_worker_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

char *ngx_http_php_rewrite_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_http_php_rewrite_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_http_php_access_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_http_php_access_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_http_php_content_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_http_php_content_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

char *ngx_http_php_opcode_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_http_php_stack_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

char *ngx_http_php_log_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_http_php_log_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

#if defined(NDK) && NDK

char *ngx_http_php_set_inline(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_http_php_set_run_inline(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_http_php_set_file(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_http_php_set_run_file(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

#endif

char *ngx_http_php_header_filter_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_http_php_body_filter_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

#endif
