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

#include "ngx_http_php_header.h"

ngx_str_t *
ngx_http_php_output_header_get(ngx_http_request_t *r, const u_char *key_data, size_t key_len)
{
	ngx_str_t 			*value;
	ngx_list_part_t     *part;
	ngx_table_elt_t 	*header;
	ngx_uint_t 			i;

	value = NULL;

	if (ngx_strncasecmp((u_char *)key_data, (u_char *)"content-type", 12) == 0){
    	value = &r->headers_out.content_type;
    }else {
        part = &r->headers_out.headers.part;
        header = part->elts;

        for ( i = 0; /* void */; i++) {
            if ( i >= part->nelts ) {
                if ( part->next == NULL ) {
                    break;
                }
                part = part->next;
                header = part->elts;
                i = 0;
            }

            if ( ngx_strncasecmp((u_char *)key_data, header[i].key.data, header[i].key.len) == 0 ) {
            	value = &header[i].value;
                break;
            }
        }
    }

    return value;
}

ngx_int_t 
ngx_http_php_output_header_set(ngx_http_request_t *r, 
	const u_char *key_data, size_t key_len, 
	const u_char *value_data, size_t value_len)
{
	ngx_table_elt_t     *h;

	if ( ngx_strncasecmp((u_char *)key_data, (u_char *)"content-type", 12) == 0 ){
        r->headers_out.content_type.data = (u_char *)value_data;
        r->headers_out.content_type.len = value_len;
        r->headers_out.content_type_len = value_len;
    }else if ( ngx_strncasecmp((u_char *)key_data, (u_char *)"content-length", 14) == 0 ) {
        r->headers_out.content_length_n = value_len;
    }else {
        h = ngx_list_push(&r->headers_out.headers);

        if ( h == NULL ) {
            return 0;
        }

        h->hash = 1;
        h->key.len = key_len;
        h->key.data = (u_char *)key_data;
        h->value.len = value_len;
        h->value.data = (u_char *)value_data;
    }

    return 1;
}



