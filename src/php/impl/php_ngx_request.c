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

#include "php_ngx_request.h"
#include "../../ngx_http_php_module.h"

static zend_class_entry *php_ngx_request_class_entry;

PHP_FUNCTION(ngx_request_method)
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

PHP_FUNCTION(ngx_request_document_root)
{
    ngx_http_request_t *r;
    ngx_http_php_loc_conf_t *plcf;

    r = ngx_php_request;
    plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);

    ZVAL_STRINGL(return_value, (char *)plcf->document_root.data, plcf->document_root.len);
}

PHP_FUNCTION(ngx_request_document_uri)
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

PHP_FUNCTION(ngx_request_script_name)
{
    ngx_http_request_t *r;

    r = ngx_php_request;

    if ((r->uri.data)[r->uri.len-1] == '/') {
        ZVAL_NULL(return_value);
    } else {
        ZVAL_STRINGL(return_value, (char *)r->uri.data, r->uri.len);
    }
}

PHP_FUNCTION(ngx_request_script_filename)
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

PHP_FUNCTION(ngx_request_query_string)
{
    ngx_http_request_t *r;

    r = ngx_php_request;

    if (r->args.len > 0){
        ZVAL_STRINGL(return_value, (char *)r->args.data, r->args.len);
    }else {
        ZVAL_STRINGL(return_value, (char *)"", 0);
    }
}

PHP_FUNCTION(ngx_request_uri)
{
    ngx_http_request_t *r;

    r = ngx_php_request;

    ZVAL_STRINGL(return_value, (char *)r->uri_start, strlen((char *)r->uri_start)-strlen((char *)r->uri_end));
}

PHP_FUNCTION(ngx_request_server_protocol)
{
    ngx_http_request_t *r;

    r = ngx_php_request;

    ZVAL_STRINGL(return_value, (char *)r->http_protocol.data, r->http_protocol.len);
}

PHP_FUNCTION(ngx_request_remote_addr)
{
    ngx_http_request_t *r;

    r = ngx_php_request;

    ZVAL_STRINGL(return_value, (char *)r->connection->addr_text.data, r->connection->addr_text.len);
}

PHP_FUNCTION(ngx_request_server_addr)
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

PHP_FUNCTION(ngx_request_remote_port)
{
    ngx_http_request_t *r;
    ngx_uint_t port;
    struct sockaddr_in  *sin;
    //char *tmp_port;

    r = ngx_php_request;
    //tmp_port = emalloc(sizeof("65535") - 1);
    sin = (struct sockaddr_in *) r->connection->local_sockaddr;
    port = ntohs(sin->sin_port);
    //ngx_sprintf((u_char *)tmp_port, "%ui", port);

    //ZVAL_STRING(return_value, (char *)tmp_port);
    ZVAL_LONG(return_value, port);
    //efree(tmp_port);
}

PHP_FUNCTION(ngx_request_server_port)
{
    ngx_http_request_t *r;
    ngx_uint_t port;
    struct sockaddr_in  *sin;
    //char *tmp_port;

    r = ngx_php_request;
    //tmp_port = emalloc(sizeof("65535") - 1);
    sin = (struct sockaddr_in *) r->connection->local_sockaddr;
    port = ntohs(sin->sin_port);
    //ngx_sprintf((u_char *)tmp_port, "%ui", port);

    //ZVAL_STRING(return_value, (char *)tmp_port);
    ZVAL_LONG(return_value, port);
    //efree(tmp_port);
}

PHP_FUNCTION(ngx_request_server_name)
{
    ngx_http_request_t *r;
    ngx_http_core_srv_conf_t *cscf;

    r = ngx_php_request;
    cscf = ngx_http_get_module_srv_conf(r, ngx_http_core_module);

    ZVAL_STRINGL(return_value, (char *)cscf->server_name.data, cscf->server_name.len);
}

PHP_FUNCTION(ngx_request_headers)
{
    ngx_http_request_t *r;
    ngx_list_part_t *part;
    ngx_table_elt_t *header;
    ngx_uint_t i;

    r = ngx_php_request;
    part = &r->headers_in.headers.part;
    header = part->elts;

    array_init(return_value);

    for (i = 0; /* void */; i++){
        if (i >= part->nelts){
            if (part->next == NULL){
                break;
            }
            part = part->next;
            header = part->elts;
            i = 0;
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"content-type", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "content_type", (char *)header[i].value.data, header[i].value.len);
        }else {
            add_assoc_stringl(return_value, "content_type", (char *)"", 0);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"content-length", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "content_length", (char *)header[i].value.data, header[i].value.len);
        }else {
            add_assoc_stringl(return_value, "content_length", (char *)"", 0);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"accept", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "accept", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"host", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "host", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"connection", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "connection", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"user-agent", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "user_agent", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"accept-encoding", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "accept_encoding", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"accept-language", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "accept_language", (char *)header[i].value.data, header[i].value.len);
        }
        
        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"referer", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "referer", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"pragma", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "pragma", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"cache-control", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "cache_control", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"upgrade-insecure-requests", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "upgrade_insecure_requests", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"cookie", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "cookie", (char *)header[i].value.data, header[i].value.len);
        }

    }
}

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

PHP_METHOD(ngx_request, remote_port)
{
    ngx_http_request_t *r;
    ngx_uint_t port;
    struct sockaddr_in  *sin;
    //char *tmp_port;

    r = ngx_php_request;
    //tmp_port = emalloc(sizeof("65535") - 1);
    sin = (struct sockaddr_in *) r->connection->local_sockaddr;
    port = ntohs(sin->sin_port);
    //ngx_sprintf((u_char *)tmp_port, "%ui", port);

    //ZVAL_STRING(return_value, (char *)tmp_port);
    ZVAL_LONG(return_value, port);
    //efree(tmp_port);
}

PHP_METHOD(ngx_request, server_port)
{
    ngx_http_request_t *r;
    ngx_uint_t port;
    struct sockaddr_in  *sin;
    //char *tmp_port;

    r = ngx_php_request;
    //tmp_port = emalloc(sizeof("65535") - 1);
    sin = (struct sockaddr_in *) r->connection->local_sockaddr;
    port = ntohs(sin->sin_port);
    //ngx_sprintf((u_char *)tmp_port, "%ui", port);

    //ZVAL_STRING(return_value, (char *)tmp_port);
    ZVAL_LONG(return_value, port);
    //efree(tmp_port);
}

PHP_METHOD(ngx_request, server_name)
{
    ngx_http_request_t *r;
    ngx_http_core_srv_conf_t *cscf;

    r = ngx_php_request;
    cscf = ngx_http_get_module_srv_conf(r, ngx_http_core_module);

    ZVAL_STRINGL(return_value, (char *)cscf->server_name.data, cscf->server_name.len);
}

PHP_METHOD(ngx_request, headers)
{
    ngx_http_request_t *r;
    ngx_list_part_t *part;
    ngx_table_elt_t *header;
    ngx_uint_t i;

    r = ngx_php_request;
    part = &r->headers_in.headers.part;
    header = part->elts;

    array_init(return_value);

    for (i = 0; /* void */; i++){
        if (i >= part->nelts){
            if (part->next == NULL){
                break;
            }
            part = part->next;
            header = part->elts;
            i = 0;
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"content-type", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "content_type", (char *)header[i].value.data, header[i].value.len);
        }else {
            add_assoc_stringl(return_value, "content_type", (char *)"", 0);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"content-length", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "content_length", (char *)header[i].value.data, header[i].value.len);
        }else {
            add_assoc_stringl(return_value, "content_length", (char *)"", 0);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"accept", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "accept", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"host", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "host", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"connection", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "connection", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"user-agent", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "user_agent", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"accept-encoding", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "accept_encoding", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"accept-language", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "accept_language", (char *)header[i].value.data, header[i].value.len);
        }
        
        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"referer", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "referer", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"pragma", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "pragma", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"cache-control", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "cache_control", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"upgrade-insecure-requests", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "upgrade_insecure_requests", (char *)header[i].value.data, header[i].value.len);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"cookie", header[i].key.len) == 0){
            add_assoc_stringl(return_value, "cookie", (char *)header[i].value.data, header[i].value.len);
        }

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
    PHP_ME(ngx_request, remote_port, ngx_request_remote_port_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_request, server_port, ngx_request_server_port_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_request, server_name, ngx_request_server_name_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_request, headers, ngx_request_headers_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    {NULL, NULL, NULL, 0, 0}
};

void
php_impl_ngx_request_init(int module_number TSRMLS_DC)
{
    zend_class_entry ngx_request_class_entry;
    INIT_CLASS_ENTRY(ngx_request_class_entry, "ngx_request", php_ngx_request_class_functions);
    php_ngx_request_class_entry = zend_register_internal_class(&ngx_request_class_entry TSRMLS_CC);
}