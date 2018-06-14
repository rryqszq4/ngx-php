/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef NGX_HTTP_PHP_ZEND_UTHREAD_H
#define NGX_HTTP_PHP_ZEND_UTHREAD_H

#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_config.h>
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