/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#include "php_ngx_request.h"
#include "../ngx_http_php_module.h"

static zend_class_entry *php_ngx_request_class_entry;

ZEND_BEGIN_ARG_INFO_EX(ngx_request_method_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(ngx_request, method)
{
    ngx_http_request_t *r = PHP_NGX_G(global_r);

    if (r->method == NGX_HTTP_GET) {
        ZVAL_STRINGL(return_value, "GET", sizeof("GET") - 1);
    }else if (r->method == NGX_HTTP_POST) {
        ZVAL_STRINGL(return_value, "POST", sizeof("POST") - 1);
    }
}

static const zend_function_entry php_ngx_request_class_functions[] = {
    PHP_ME(ngx_request, method, ngx_request_method_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    {NULL, NULL, NULL, 0, 0}
};

void
php_ngx_request_init(int module_number TSRMLS_DC)
{
    zend_class_entry ngx_request_class_entry;
    INIT_CLASS_ENTRY(ngx_request_class_entry, "ngx_request", php_ngx_request_class_functions);
    php_ngx_request_class_entry = zend_register_internal_class_entry TSRMLS_CC);
}