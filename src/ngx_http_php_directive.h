/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef NGX_HTTP_PHP_DIRECTIVE_H
#define NGX_HTTP_PHP_DIRECTIVE_H

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
char *ngx_http_php_content_async_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

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
