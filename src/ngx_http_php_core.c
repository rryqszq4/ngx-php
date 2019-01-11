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

#include "ngx_http_php_module.h"
#include "ngx_http_php_request.h"
#include "ngx_http_php_core.h"
#include "ngx_http_php_zend_uthread.h"

ngx_http_php_code_t *
ngx_http_php_code_from_file(ngx_pool_t *pool, ngx_str_t *code_file_path)
{
    ngx_http_php_code_t *code;
    size_t len;
    u_char *p;

    code = ngx_pcalloc(pool, sizeof(*code));
    if (code == NULL){
        return NGX_CONF_UNSET_PTR;
    }

    len = ngx_strlen((char *)code_file_path->data);
    if (len == 0){
        return NGX_CONF_UNSET_PTR;
    }else if (code_file_path->data[0] == '/' || code_file_path->data[0] == '$'){
        code->code.file = ngx_pcalloc(pool, len + 1);
        if (code->code.file == NULL){
            return NGX_CONF_UNSET_PTR;
        }
        ngx_cpystrn((u_char *)code->code.file, (u_char *)code_file_path->data, code_file_path->len + 1);    
    }else {
        code->code.file = ngx_pcalloc(pool, ngx_cycle->conf_prefix.len + len + 1);
        if (code->code.file == NULL){
            return NGX_CONF_UNSET_PTR;
        }
        p = ngx_cpystrn((u_char *)code->code.file, (u_char *)ngx_cycle->conf_prefix.data, ngx_cycle->conf_prefix.len + 1);
        ngx_cpystrn(p, (u_char *)code_file_path->data, code_file_path->len + 1);
    }
    code->code_type = NGX_HTTP_PHP_CODE_TYPE_FILE;

    //code->code_id.data = ngx_pnalloc(pool, sizeof("18446744073709551616")-1+NGX_TIME_T_LEN);
    //code->code_id.len = ngx_sprintf(code->code_id.data, "%ul%T", ngx_random(), ngx_time()) - code->code_id.data;

    code->code_id.len = 32;
    code->code_id.data = ngx_pnalloc(pool, 32);
    if (code->code_id.data == NULL) {
        return NGX_CONF_UNSET_PTR;
    }
    ngx_sprintf(code->code_id.data, "%08xD%08xD%08xD%08xD",
                (uint32_t) ngx_random(), (uint32_t) ngx_random(),
                (uint32_t) ngx_random(), (uint32_t) ngx_random());

    return code;
}

ngx_http_php_code_t *
ngx_http_php_code_from_string(ngx_pool_t *pool, ngx_str_t *code_str)
{
    ngx_http_php_code_t *code;
    size_t len;

    code = ngx_pcalloc(pool, sizeof(*code));
    if (code == NULL){
        return NGX_CONF_UNSET_PTR;
    }

    len = ngx_strlen(code_str->data);
    code->code.string = ngx_pcalloc(pool, len + 1);
    if (code->code.string == NULL){
        return NGX_CONF_UNSET_PTR;
    }
    ngx_cpystrn((u_char *)code->code.string, code_str->data, len + 1);
    code->code_type = NGX_HTTP_PHP_CODE_TYPE_STRING;

    //code->code_id.data = ngx_pnalloc(pool, sizeof("18446744073709551616")-1+NGX_TIME_T_LEN);
    //code->code_id.len = ngx_sprintf(code->code_id.data, "%ul%T", ngx_random(), ngx_time()) - code->code_id.data;

    code->code_id.len = 32;
    code->code_id.data = ngx_pnalloc(pool, 32);
    if (code->code_id.data == NULL) {
        return NGX_CONF_UNSET_PTR;
    }
    ngx_sprintf(code->code_id.data, "%08xD%08xD%08xD%08xD",
                (uint32_t) ngx_random(), (uint32_t) ngx_random(),
                (uint32_t) ngx_random(), (uint32_t) ngx_random());

    return code;
}

void 
ngx_php_error_cb(int type, 
    const char *error_filename, const uint error_lineno, const char *format, va_list args)
{
    TSRMLS_FETCH();
    char *buffer;
    int buffer_len, display;

    buffer_len = vspprintf(&buffer, PG(log_errors_max_len), format, args);
    
    /* check for repeated errors to be ignored */
    if (PG(ignore_repeated_errors) && PG(last_error_message)) {
        /* no check for PG(last_error_file) is needed since it cannot
         * be NULL if PG(last_error_message) is not NULL */
        if (strcmp(PG(last_error_message), buffer)
            || (!PG(ignore_repeated_source)
                && ((PG(last_error_lineno) != (int)error_lineno)
                    || strcmp(PG(last_error_file), error_filename)))) {
            display = 1;
        } else {
            display = 0;
        }
    } else {
        display = 1;
    }

    /* store the error if it has changed */
    if (display) {
        if (PG(last_error_message)) {
            free(PG(last_error_message));
            PG(last_error_message) = NULL;
        }
        if (PG(last_error_file)) {
            free(PG(last_error_file));
            PG(last_error_file) = NULL;
        }
        if (!error_filename) {
            error_filename = "Unknown";
        }
        PG(last_error_type) = type;
        PG(last_error_message) = strdup(buffer);
        PG(last_error_file) = strdup(error_filename);
        PG(last_error_lineno) = error_lineno;
    }

    /* according to error handling mode, suppress error, throw exception or show it */
    if (EG(error_handling) != EH_NORMAL) {
        switch (type) {
            case E_ERROR:
            case E_CORE_ERROR:
            case E_COMPILE_ERROR:
            case E_USER_ERROR:
            case E_PARSE:
                /* fatal errors are real errors and cannot be made exceptions */
                break;
            case E_STRICT:
            case E_DEPRECATED:
            case E_USER_DEPRECATED:
                /* for the sake of BC to old damaged code */
                break;
            case E_NOTICE:
            case E_USER_NOTICE:
                /* notices are no errors and are not treated as such like E_WARNINGS */
                break;
            default:
                /* throw an exception if we are in EH_THROW mode
                 * but DO NOT overwrite a pending exception
                 */
                if (EG(error_handling) == EH_THROW && !EG(exception)) {
                    zend_throw_error_exception(EG(exception_class), buffer, 0, type TSRMLS_CC);
                }
                efree(buffer);
                return;
        }
    }

    /* display/log the error if necessary */
    if (display && (EG(error_reporting) & type || (type & E_CORE))
        && (PG(log_errors) || PG(display_errors) ) ) {

        char *error_type_str;

        switch (type) {
            case E_ERROR:
            case E_CORE_ERROR:
            case E_COMPILE_ERROR:
            case E_USER_ERROR:
                error_type_str = "Fatal error";
                break;
            case E_RECOVERABLE_ERROR:
                error_type_str = "Catchable fatal error";
                break;
            case E_WARNING:
            case E_CORE_WARNING:
            case E_COMPILE_WARNING:
            case E_USER_WARNING:
                error_type_str = "Warning";
                break;
            case E_PARSE:
                error_type_str = "Parse error";
                break;
            case E_NOTICE:
            case E_USER_NOTICE:
                error_type_str = "Notice";
                break;
            case E_STRICT:
                error_type_str = "Strict Standards";
                break;
            case E_DEPRECATED:
            case E_USER_DEPRECATED:
                error_type_str = "Deprecated";
                break;
            default:
                error_type_str = "Unknown error";
                break;
        }
        buffer_len = spprintf(&buffer, 0, "\n%s: %s in %s on line %d\n", error_type_str, buffer, error_filename, error_lineno);

        ngx_buf_t *b;
        ngx_http_php_rputs_chain_list_t *chain;
        ngx_http_php_ctx_t *ctx;
        ngx_http_request_t *r;
        u_char *u_str;
        ngx_str_t ns;

        r = ngx_php_request;
        ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

        ns.data = (u_char *)buffer;
        ns.len = buffer_len;

        if (ctx->rputs_chain == NULL){
            chain = ngx_pcalloc(r->pool, sizeof(ngx_http_php_rputs_chain_list_t));
            chain->out = ngx_alloc_chain_link(r->pool);
            chain->last = &chain->out;
        }else {
            chain = ctx->rputs_chain;
            (*chain->last)->next = ngx_alloc_chain_link(r->pool);
            chain->last = &(*chain->last)->next;
        }

        b = ngx_calloc_buf(r->pool);
        (*chain->last)->buf = b;
        (*chain->last)->next = NULL;

        u_str = ngx_pstrdup(r->pool, &ns);
        //u_str[ns.len] = '\0';
        (*chain->last)->buf->pos = u_str;
        (*chain->last)->buf->last = u_str + ns.len;
        (*chain->last)->buf->memory = 1;
        ctx->rputs_chain = chain;
        ngx_http_set_ctx(r, ctx, ngx_http_php_module);

        if (r->headers_out.content_length_n == -1){
            r->headers_out.content_length_n += ns.len + 1;
        }else {
            r->headers_out.content_length_n += ns.len;
        }

        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, " %s: %s in %s on line %d \n", error_type_str, buffer, error_filename, error_lineno);

        ngx_http_php_zend_uthread_exit(r);

        efree(buffer);
        zend_bailout();
        return ;
    }

    /* Log if necessary */
    if (!display) {
        efree(buffer);
        return;
    }

    efree(buffer);
}

size_t ngx_http_php_code_ub_write(const char *str, size_t str_length TSRMLS_DC)
{
    ngx_buf_t *b;
    ngx_http_php_rputs_chain_list_t *chain;
    ngx_http_php_ctx_t *ctx;
    ngx_http_request_t *r;
    u_char *u_str;
    ngx_str_t ns;

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx->output_type & OUTPUT_CONTENT){

        ns.data = (u_char *)str;
        ns.len = str_length;

        if (ctx->rputs_chain == NULL){
            chain = ngx_pcalloc(r->pool, sizeof(ngx_http_php_rputs_chain_list_t));
            chain->out = ngx_alloc_chain_link(r->pool);
            chain->last = &chain->out;
        }else {
            chain = ctx->rputs_chain;
            (*chain->last)->next = ngx_alloc_chain_link(r->pool);
            chain->last = &(*chain->last)->next;
        }

        b = ngx_calloc_buf(r->pool);
        (*chain->last)->buf = b;
        (*chain->last)->next = NULL;

        u_str = ngx_pstrdup(r->pool, &ns);
        //u_str[ns.len] = '\0';
        (*chain->last)->buf->pos = u_str;
        (*chain->last)->buf->last = u_str + ns.len;
        (*chain->last)->buf->memory = 1;
        ctx->rputs_chain = chain;
        ngx_http_set_ctx(r, ctx, ngx_http_php_module);

        if (r->headers_out.content_length_n == -1){
            r->headers_out.content_length_n += ns.len + 1;
        }else {
            r->headers_out.content_length_n += ns.len;
        }

    }
    return r->headers_out.content_length_n;
}

void 
ngx_http_php_code_flush(void *server_context)
{
    
}

void ngx_http_php_code_log_message(char *message)
{
    

}

void ngx_http_php_code_register_server_variables(zval *track_vars_array TSRMLS_DC)
{
    /*php_import_environment_variables(track_vars_array TSRMLS_CC);

    ngx_http_request_t *r;
    r = ngx_php_request;

    //ngx_http_headers_in_t *headers_in;
    //headers_in = &r->headers_in;

    ngx_list_part_t *part = &r->headers_in.headers.part;
    ngx_table_elt_t *header = part->elts;

    ngx_uint_t i;

    ngx_http_php_loc_conf_t *plcf = ngx_http_get_module_loc_conf(r, ngx_http_php_module);

    if (r->method == NGX_HTTP_GET){
        php_register_variable("REQUEST_METHOD", (char *)"GET", track_vars_array TSRMLS_CC);
    } else if (r->method == NGX_HTTP_POST){
        php_register_variable("REQUEST_METHOD", (char *)"POST", track_vars_array TSRMLS_CC);
    }

    //php_register_variable_safe("REQUEST_LINE", (char *)r->request_line.data, r->request_line.len, track_vars_array TSRMLS_CC);

    php_register_variable_safe("DOCUMENT_ROOT", (char *)plcf->document_root.data, plcf->document_root.len, track_vars_array TSRMLS_CC);

    if ((r->uri.data)[r->uri.len-1] == '/'){
        char *tmp_uri;
        tmp_uri = emalloc(r->uri.len + 9 + 1);
        ngx_cpystrn((u_char *)tmp_uri, (u_char *)r->uri.data, r->uri.len + 1);
        strncat(tmp_uri, "index.php", 9);
        php_register_variable_safe("DOCUMENT_URI", (char *)tmp_uri, r->uri.len + 9, track_vars_array TSRMLS_CC);
        efree(tmp_uri);

        char *tmp_script;
        tmp_script = emalloc(plcf->document_root.len + r->uri.len + 9 + 1);
        ngx_cpystrn((u_char *)tmp_script, (u_char *)plcf->document_root.data, plcf->document_root.len+1);
        strncat(tmp_script, (char *)r->uri.data, r->uri.len);
        strncat(tmp_script, "index.php", 9);
        php_register_variable_safe("SCRIPT_FILENAME", (char *)tmp_script, plcf->document_root.len + r->uri.len + 9, track_vars_array TSRMLS_CC);
        efree(tmp_script);
    } else {
        php_register_variable_safe("DOCUMENT_URI", (char *)r->uri.data, r->uri.len, track_vars_array TSRMLS_CC);
        php_register_variable_safe("SCRIPT_NAME", (char *)r->uri.data, r->uri.len, track_vars_array TSRMLS_CC);

        char *tmp_script;
        tmp_script = emalloc(plcf->document_root.len + r->uri.len + 1);
        ngx_cpystrn((u_char *)tmp_script, (u_char *)plcf->document_root.data, plcf->document_root.len+1);
        strncat(tmp_script, (char *)r->uri.data, r->uri.len);
        php_register_variable_safe("SCRIPT_FILENAME", (char *)tmp_script, plcf->document_root.len + r->uri.len, track_vars_array TSRMLS_CC);
        efree(tmp_script);
    }

    if (r->args.len > 0){
        php_register_variable_safe("QUERY_STRING", (char *)r->args.data, r->args.len, track_vars_array TSRMLS_CC);
    }else {
        php_register_variable_safe("QUERY_STRING", (char *)"", 0, track_vars_array TSRMLS_CC);
    }

    php_register_variable_safe("REQUEST_URI", (char *)r->uri_start, strlen((char *)r->uri_start)-strlen((char *)r->uri_end),track_vars_array TSRMLS_CC);
    
    php_register_variable_safe("SERVER_PROTOCOL", (char *)r->http_protocol.data, r->http_protocol.len, track_vars_array TSRMLS_CC);

    php_register_variable_safe("REMOTE_ADDR", (char *)r->connection->addr_text.data, r->connection->addr_text.len, track_vars_array TSRMLS_CC);

    ngx_str_t  server_address;
    u_char     server_addr[NGX_SOCKADDR_STRLEN];
    server_address.len = NGX_SOCKADDR_STRLEN;
    server_address.data = server_addr;
    if (ngx_connection_local_sockaddr(r->connection, &server_address, 0) != NGX_OK) {
    }
    php_register_variable_safe("SERVER_ADDR", (char *)server_address.data, server_address.len, track_vars_array TSRMLS_CC);

    ngx_uint_t port;
    struct sockaddr_in  *sin;
    char *tmp_port;
    tmp_port = emalloc(sizeof("65535") - 1);
    sin = (struct sockaddr_in *) r->connection->local_sockaddr;
    port = ntohs(sin->sin_port);
    ngx_sprintf((u_char *)tmp_port, "%ui", port);
    php_register_variable("SERVER_PORT", (char *)tmp_port, track_vars_array TSRMLS_CC);
    efree(tmp_port);

    tmp_port = emalloc(sizeof("65535") - 1);
    sin = (struct sockaddr_in *) r->connection->sockaddr;
    port = ntohs(sin->sin_port);
    ngx_sprintf((u_char *)tmp_port, "%ui", port);
    php_register_variable("REMOTE_PORT", (char *)tmp_port, track_vars_array TSRMLS_CC);
    efree(tmp_port);

    ngx_http_core_srv_conf_t  *cscf;
    cscf = ngx_http_get_module_srv_conf(r, ngx_http_core_module);
    php_register_variable_safe("SERVER_NAME", (char *)cscf->server_name.data, cscf->server_name.len, track_vars_array TSRMLS_CC);
*/
    //for (i = 0; /* void */; i++){
    /*    if (i >= part->nelts){
            if (part->next == NULL){
                break;
            }
            part = part->next;
            header = part->elts;
            i = 0;
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"content-type", header[i].key.len) == 0){
            php_register_variable_safe("CONTENT_TYPE", (char *)header[i].value.data, header[i].value.len, track_vars_array TSRMLS_CC);
        }else {
            php_register_variable_safe("CONTENT_TYPE", (char *)"", 0, track_vars_array TSRMLS_CC);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"content-length", header[i].key.len) == 0){
            php_register_variable_safe("CONTENT_LENGTH", (char *)header[i].value.data, header[i].value.len, track_vars_array TSRMLS_CC);
        }else {
            php_register_variable_safe("CONTENT_LENGTH", (char *)"", 0, track_vars_array TSRMLS_CC);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"accept", header[i].key.len) == 0){
            php_register_variable_safe("HTTP_ACCEPT", (char *)header[i].value.data, header[i].value.len, track_vars_array TSRMLS_CC);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"host", header[i].key.len) == 0){
            php_register_variable_safe("HTTP_HOST", (char *)header[i].value.data, header[i].value.len, track_vars_array TSRMLS_CC);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"connection", header[i].key.len) == 0){
            php_register_variable_safe("HTTP_CONNECTION", (char *)header[i].value.data, header[i].value.len, track_vars_array TSRMLS_CC);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"user-agent", header[i].key.len) == 0){
            php_register_variable_safe("HTTP_USER_AGENT", (char *)header[i].value.data, header[i].value.len, track_vars_array TSRMLS_CC);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"accept-encoding", header[i].key.len) == 0){
            php_register_variable_safe("HTTP_ACCEPT_ENCODING", (char *)header[i].value.data, header[i].value.len, track_vars_array TSRMLS_CC);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"accept-language", header[i].key.len) == 0){
            php_register_variable_safe("HTTP_ACCEPT_LANGUAGE", (char *)header[i].value.data, header[i].value.len, track_vars_array TSRMLS_CC);
        }
        
        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"referer", header[i].key.len) == 0){
            php_register_variable_safe("HTTP_REFERER", (char *)header[i].value.data, header[i].value.len, track_vars_array TSRMLS_CC);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"pragma", header[i].key.len) == 0){
            php_register_variable_safe("HTTP_PRAGMA", (char *)header[i].value.data, header[i].value.len, track_vars_array TSRMLS_CC);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"cache-control", header[i].key.len) == 0){
            php_register_variable_safe("HTTP_CACHE_CONTROL", (char *)header[i].value.data, header[i].value.len, track_vars_array TSRMLS_CC);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"upgrade-insecure-requests", header[i].key.len) == 0){
            php_register_variable_safe("HTTP_UPGRADE_INSECURE_REQUESTS", (char *)header[i].value.data, header[i].value.len, track_vars_array TSRMLS_CC);
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"cookie", header[i].key.len) == 0){
            php_register_variable_safe("HTTP_COOKIE", (char *)header[i].value.data, header[i].value.len, track_vars_array TSRMLS_CC);
        }

    }*/

}

int 
ngx_http_php_code_read_post(char *buffer, uint count_bytes TSRMLS_DC)
{
    uint read_bytes = 0;
    int tmp_read_bytes;

    ngx_http_request_t *r = ngx_php_request;
    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ngx_http_headers_in_t *headers_in;
    headers_in = &r->headers_in;

    int content_length_n;

    if (headers_in->content_length){
        char *content_length = (char *)headers_in->content_length->value.data;
        content_length_n = atoi(content_length);
    }else {
        content_length_n = 0;
    }

    while ((int)ctx->request_body_ctx.len < content_length_n){
        /* waiting event */
    }
    //ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, "buffer : %d %d %d", count_bytes, content_length_n, SG(read_post_bytes));

    count_bytes = MIN(count_bytes, (uint)content_length_n - SG(read_post_bytes));

    while (read_bytes < count_bytes){
        ngx_memcpy(buffer+read_bytes, (char *)ctx->request_body_ctx.data + SG(read_post_bytes), count_bytes - read_bytes);
        tmp_read_bytes = count_bytes - read_bytes;
        read_bytes += tmp_read_bytes;
    }
    
    return read_bytes;
}

char *
ngx_http_php_code_read_cookies(TSRMLS_D)
{
    ngx_http_php_request_context_t *context;
    context = SG(server_context);
    return (char *)context->cookie_data;
}

int 
ngx_http_php_code_header_handler(sapi_header_struct *sapi_header, 
    sapi_header_op_enum op, sapi_headers_struct *sapi_headers TSRMLS_DC)
{
    ngx_http_request_t *r;
    ngx_table_elt_t *h;
    char *tmp;

    if ((tmp = ngx_strstr(sapi_header->header, ": ")) == NULL){
        return NGX_ERROR;
    }

    r = ngx_php_request;
    h = ngx_list_push(&r->headers_out.headers);

    if (h == NULL){
        return NGX_ERROR;
    }

    h->hash = 1;
    h->key.len = tmp - sapi_header->header;
    h->key.data = ngx_pcalloc(r->pool, h->key.len);
    ngx_cpystrn((u_char *)h->key.data, (u_char *) sapi_header->header, h->key.len + 1);

    h->value.len = sapi_header->header_len - h->key.len - 2;
    h->value.data = ngx_pcalloc(r->pool, h->value.len);
    ngx_cpystrn((u_char *) h->value.data, (u_char *)(tmp + 2), h->value.len + 1);

    if (ngx_strncmp(h->key.data, "Location", sizeof("Location") - 1) == 0){
        r->headers_out.status = NGX_HTTP_MOVED_TEMPORARILY;
    }

    return 0;
}

/*ngx_int_t 
ngx_php_embed_run(ngx_http_request_t *r, ngx_http_php_code_t *code)
{

    php_embed_module.ub_write = ngx_http_php_code_ub_write;
    php_embed_module.flush = ngx_http_php_code_flush;
    php_embed_module.php_ini_path_override = "/usr/local/php/etc/php.ini";
    PHP_EMBED_START_BLOCK(0, NULL);
        zend_eval_string_ex(code->code.string, NULL, "ngx_php run code", 1 TSRMLS_CC);
    PHP_EMBED_END_BLOCK();

    return 0;
}*/

ngx_int_t
ngx_php_ngx_run(ngx_http_request_t *r, ngx_http_php_state_t *state, ngx_http_php_code_t *code)
{
    TSRMLS_FETCH();

    if (code->code_type == NGX_HTTP_PHP_CODE_TYPE_STRING){

        zend_eval_string_ex(code->code.string, NULL, "ngx_php run code", 1 TSRMLS_CC);

    }else if (code->code_type == NGX_HTTP_PHP_CODE_TYPE_FILE){

        zend_file_handle file_handle;

        file_handle.type = ZEND_HANDLE_FP;
        file_handle.opened_path = NULL;
        file_handle.free_filename = 0;
        file_handle.filename = code->code.file;
        if (!(file_handle.handle.fp = VCWD_FOPEN(file_handle.filename, "rb"))) {
            php_printf("Could not open input file: %s\n", file_handle.filename);
            return FAILURE;
        }
        php_execute_script(&file_handle TSRMLS_CC);

    }else {
    }

    return 0;
}

ngx_int_t 
ngx_php_eval_code(ngx_http_request_t *r, ngx_http_php_state_t *state, ngx_http_php_code_t *code)
{

    if (code->code_type == NGX_HTTP_PHP_CODE_TYPE_STRING) {

        zend_eval_string_ex(code->code.string, NULL, "ngx_php eval code", 1);

    }

    return 0;
}

ngx_int_t 
ngx_php_eval_file(ngx_http_request_t *r, ngx_http_php_state_t *state, ngx_http_php_code_t *code)
{
    if (code->code_type == NGX_HTTP_PHP_CODE_TYPE_FILE){

        zend_file_handle file_handle;

        file_handle.type = ZEND_HANDLE_FP;
        file_handle.opened_path = NULL;
        file_handle.free_filename = 0;
        file_handle.filename = code->code.file;
        if (!(file_handle.handle.fp = VCWD_FOPEN(file_handle.filename, "rb"))) {
            php_printf("Could not open input file: %s\n", file_handle.filename);
            return FAILURE;
        }
        php_execute_script(&file_handle);

    }

    return 0;
}

ngx_int_t 
ngx_php_get_request_status()
{
    return EG(exit_status);
}

ngx_int_t ngx_php_set_request_status(ngx_int_t rc)
{
    EG(exit_status) = rc;

    return 0;
}


