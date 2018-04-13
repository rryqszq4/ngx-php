/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef NGX_HTTP_PHP_VARIABLE_H
#define NGX_HTTP_PHP_VARIABLE_H

#include "ngx_http_php_module.h"

ngx_http_variable_value_t *ngx_http_php_var_get(const char *z_name, size_t z_len);

int ngx_http_php_var_set(char *z_k, size_t z_k_len, char *z_v, size_t z_v_len);

#endif