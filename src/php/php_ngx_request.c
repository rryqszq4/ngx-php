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

PHP_METHOD(ngx_request, method)
{
    ngx_http_request_t *r;

    r = ngx_php_request;

    
    if (r->method == NGX_HTTP_GET) {
        ZVAL_STRINGL(return_value, (char *)"GET", sizeof("GET") - 1);
    }else if (r->method == NGX_HTTP_POST) {
        ZVAL_STRINGL(return_value, (char *)"POST", sizeof("POST") - 1);
    }else if (r->method == NGX_HTTP_PUT) {
        ZVAL_STRINGL(return_value, (char *)"PUT", sizeof("PUT") - 1);
    }else if (r->method == NGX_HTTP_HEAD) {
        ZVAL_STRINGL(return_value, (char *)"HEAD", sizeof("HEAD") - 1);
    }else if (r->method == NGX_HTTP_DELETE) {
        ZVAL_STRINGL(return_value, (char *)"DELETE", sizeof("DELETE") - 1);
    }
    
}

PHP_METHOD(ngx_request, document_root)
{
    ngx_http_request_t *r;
    ngx_http_php_loc_conf_t *plcf;

    r = ngx_php_request;
    plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);

    ZVAL_STRINGL(return_value, (char *)plcf->document_root.data, plcf->document_root.len);
}

PHP_METHOD(ngx_request, document_uri)
{
    ngx_http_request_t *r;

    r = ngx_php_request;

    if ((r->uri.data)[r->uri.len-1] == '/') {
        char *tmp_uri;
        tmp_uri = emalloc(r->uri.len + 9 + 1);
        ngx_cpystrn((u_char *)tmp_uri, (u_char *)r->uri.data, r->uri.len + 1);
        strncat(tmp_uri, "index.php", 9);
        ZVAL_STRINGL(return_value, (char *)tmp_uri, r->uri.len + 9);
        efree(tmp_uri);
    } else {
        ZVAL_STRINGL(return_value, (char *)r->uri.data, r->uri.len);
    }
}

PHP_METHOD(ngx_request, script_name)
{
    ngx_http_request_t *r;

    r = ngx_php_request;

    if ((r->uri.data)[r->uri.len-1] == '/') {
        ZVAL_NULL(return_value);
    } else {
        ZVAL_STRINGL(return_value, (char *)r->uri.data, r->uri.len);
    }
}

PHP_METHOD(ngx_request, script_filename)
{
    ngx_http_request_t *r;
    ngx_http_php_loc_conf_t *plcf;

    r = ngx_php_request;
    plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);

    if ((r->uri.data)[r->uri.len-1] == '/'){
        char *tmp_script;
        tmp_script = emalloc(plcf->document_root.len + r->uri.len + 9 + 1);
        ngx_cpystrn((u_char *)tmp_script, (u_char *)plcf->document_root.data, plcf->document_root.len+1);
        strncat(tmp_script, (char *)r->uri.data, r->uri.len);
        strncat(tmp_script, "index.php", 9);
        ZVAL_STRINGL(return_value, (char *)tmp_script, plcf->document_root.len + r->uri.len + 9);
        efree(tmp_script);
    } else {
        char *tmp_script;
        tmp_script = emalloc(plcf->document_root.len + r->uri.len + 1);
        ngx_cpystrn((u_char *)tmp_script, (u_char *)plcf->document_root.data, plcf->document_root.len+1);
        strncat(tmp_script, (char *)r->uri.data, r->uri.len);
        ZVAL_STRINGL(return_value, (char *)tmp_script, plcf->document_root.len + r->uri.len);
        efree(tmp_script);
    }
}

PHP_METHOD(ngx_request, query_string)
{
    ngx_http_request_t *r;

    r = ngx_php_request;

    if (r->args.len > 0){
        ZVAL_STRINGL(return_value, (char *)r->args.data, r->args.len);
    }else {
        ZVAL_STRINGL(return_value, (char *)"", 0);
    }
}

PHP_METHOD(ngx_request, request_uri)
{
    ngx_http_request_t *r;

    r = ngx_php_request;

    ZVAL_STRINGL(return_value, (char *)r->uri_start, strlen((char *)r->uri_start)-strlen((char *)r->uri_end));
}

PHP_METHOD(ngx_request, server_protocol)
{
    ngx_http_request_t *r;

    r = ngx_php_request;

    ZVAL_STRINGL(return_value, (char *)r->http_protocol.data, r->http_protocol.len);
}

PHP_METHOD(ngx_request, remote_addr)
{
    ngx_http_request_t *r;

    r = ngx_php_request;

    ZVAL_STRINGL(return_value, (char *)r->connection->addr_text.data, r->connection->addr_text.len);
}

PHP_METHOD(ngx_request, server_addr)
{
    ngx_http_request_t *r;
    ngx_str_t  server_address;
    u_char     server_addr[NGX_SOCKADDR_STRLEN];

    r = ngx_php_request;
    server_address.len = NGX_SOCKADDR_STRLEN;
    server_address.data = server_addr;

    if (ngx_connection_local_sockaddr(r->connection, &server_address, 0) != NGX_OK) {
        ZVAL_NULL(return_value);    
    }else {
        ZVAL_STRINGL(return_value, (char *)server_address.data, server_address.len);
    }
}

static const zend_function_entry php_ngx_request_class_functions[] = {
    PHP_ME(ngx_request, method, ngx_request_method_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_request, document_root, ngx_request_document_root_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_request, document_uri, ngx_request_document_uri_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_request, script_name, ngx_request_script_name_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_request, script_filename, ngx_request_script_filename_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_request, query_string, ngx_request_query_string_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_request, request_uri, ngx_request_request_uri_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_request, server_protocol, ngx_request_server_protocol_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_request, remote_addr, ngx_request_remote_addr_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_request, server_addr, ngx_request_server_addr_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    {NULL, NULL, NULL, 0, 0}
};

void
ext_php_ngx_request_init(int module_number TSRMLS_DC)
{
    zend_class_entry ngx_request_class_entry;
    INIT_CLASS_ENTRY(ngx_request_class_entry, "ngx_request", php_ngx_request_class_functions);
    php_ngx_request_class_entry = zend_register_internal_class(&ngx_request_class_entry TSRMLS_CC);
}