/*
==============================================================================
Copyright (c) 2016-2020, rryqszq4 <rryqszq@gmail.com>
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

#include "php_ngx_cookie.h"
#include "../../ngx_http_php_module.h"

PHP_FUNCTION(ngx_cookie_get_all)
{
    ngx_http_request_t  *r;
    ngx_list_part_t     *part;
    ngx_table_elt_t     *header;
    ngx_uint_t          i;

    r = ngx_php_request;
    part = &r->headers_in.headers.part;
    header = part->elts;

    for (i = 0; /* void */; i++) {
        if (i >= part->nelts){
            if (part->next == NULL){
                break;
            }
            part = part->next;
            header = part->elts;
            i = 0;
        }

        if (ngx_strncasecmp(header[i].lowcase_key, (u_char *)"cookie", header[i].key.len) == 0){
            ZVAL_STRINGL(return_value, (char *)header[i].value.data, header[i].value.len);
        }
    }
}

PHP_FUNCTION(ngx_cookie_get)
{
    zend_string         *key_str;
    ngx_http_request_t  *r;
    ngx_list_part_t     *part;
    ngx_table_elt_t     *header;
    ngx_uint_t          i;
    unsigned            found;

    u_char              *start, *p, *last;
    char                *query_key=NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "S", &key_str) == FAILURE) {
        RETURN_NULL();
    }

    r = ngx_php_request;
    found = 0;

    part = &r->headers_in.headers.part;
    header = part->elts;

    for (i = 0; /* void */; i++) {
        if ( i >= part->nelts ) {
            if ( part->next == NULL) {
                break;
            }
            part = part->next;
            header = part->elts;
            i = 0;
        }

        if ( ngx_strncasecmp(header[i].lowcase_key, (u_char *)"cookie", header[i].key.len) == 0 ) {
            start = header[i].value.data;
            last = header[i].value.data + header[i].value.len;

            p = start;
            while ( start != last ) {
                if ( *start == '=' ) {
                    query_key = ngx_pnalloc(r->pool, start - p + 1);
                    ngx_memzero(query_key, start - p + 1);
                    ngx_memcpy(query_key, p, start - p);
                    start++;
                    p = start;

                    if (ngx_strncasecmp((u_char *)ZSTR_VAL(key_str), (u_char *)query_key, ZSTR_LEN(key_str)) == 0) {
                        found = 1;
                    }
                }else if ( *start == ';') {
                    if (found == 1) {
                        ZVAL_STRINGL(return_value, (char *)p, start - p);
                        ngx_pfree(r->pool, query_key);
                        found = 0;
                        break;
                    }
                    ngx_pfree(r->pool, query_key);

                    start++;
                    if ( *start == ' ') { start++; }
                    p = start;
                }

                if ( start != last ) {
                    start++;
                }
            }

            if ( found == 1 ) {
                ZVAL_STRINGL(return_value, (char *)p, start - p);
                found = 0;
            }
        }
    }

}

PHP_FUNCTION(ngx_cookie_set)
{
    ngx_http_request_t  *r;
    ngx_table_elt_t     *h;
    zend_string         *value_str;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "S", &value_str) == FAILURE) {
        RETURN_NULL();
    }

    r = ngx_php_request;
    
    h = ngx_list_push(&r->headers_out.headers);

    if ( h == NULL ) {
        RETURN_FALSE;
    }

    h->hash = 1;
    h->key.len = 10;
    h->key.data = (u_char *)"Set-Cookie";
    h->value.len = ZSTR_LEN(value_str);
    h->value.data = (u_char *)ZSTR_VAL(value_str);

    RETURN_TRUE;
}

void 
ngx_impl_ngx_cookie_init(int module_number )
{

}