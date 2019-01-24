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

#include "php_ngx_header.h"
#include "../../ngx_http_php_module.h"

PHP_FUNCTION(ngx_header_set)
{
	ngx_http_request_t 	*r;
    zend_string *key_str;
    zval *value;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Sz", &key_str, &value) == FAILURE) {
        RETURN_NULL();
    }

    r = ngx_php_request;

    if (ngx_strncasecmp((u_char *)ZSTR_VAL(key_str), (u_char *)"content-type", 12) == 0){
        r->headers_out.content_type.data = (u_char *)Z_STRVAL_P(value);
        r->headers_out.content_type.len = Z_STRLEN_P(value);
        r->headers_out.content_type_len = Z_STRLEN_P(value);
    }

    if ( ngx_strncasecmp((u_char *)ZSTR_VAL(key_str), (u_char *)"content-length", 14) == 0 ) {
        php_printf("%d", (int)Z_LVAL_P(value));
        r->headers_out.content_length_n = Z_LVAL_P(value);
    }

}

PHP_FUNCTION(ngx_header_get)
{
	ngx_http_request_t 	*r;
    zend_string *key_str;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S", &key_str) == FAILURE){
        RETURN_NULL();
    }

    r = ngx_php_request;

    if (ngx_strncasecmp((u_char *)ZSTR_VAL(key_str), (u_char *)"content-type", 12) == 0){
    	ZVAL_STRINGL(return_value, (char *)r->headers_out.content_type.data, r->headers_out.content_type.len);
    }
}

PHP_FUNCTION(ngx_header_gets)
{
	ngx_http_request_t 	*r;
	ngx_list_part_t 	*part;
	ngx_table_elt_t 	*header;
	ngx_uint_t 			i;

	r = ngx_php_request;
	part = &r->headers_out.headers.part;
	header = part->elts;

	array_init(return_value);

	for ( i = 0; /* void */; i++ ) {
		if ( i >= part->nelts ) {
			if ( part->next == NULL ) {
				break;
			}
			part = part->next;
			header = part->elts;
			i = 0;
		}

		add_assoc_stringl_ex(return_value, (char *)header[i].key.data, header[i].key.len, (char *)header[i].value.data, header[i].value.len);
	}
}

void 
php_impl_ngx_header_init(int module_number TSRMLS_DC)
{

}
