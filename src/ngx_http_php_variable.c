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

#include "ngx_http_php_variable.h"

ngx_http_variable_value_t *
ngx_http_php_var_get(const char *z_name, size_t z_len)
{
    ngx_http_request_t          *r;
    ngx_http_variable_value_t   *var;
    ngx_str_t                   name;
    ngx_uint_t                  hash;

    r = ngx_php_request;

    name.len = z_len;
    name.data = (u_char *)z_name;

    hash = ngx_hash_strlow(name.data, name.data, name.len);
    var = ngx_http_get_variable(r, &name, hash);

    if (var == NULL || var->not_found) {
        return NULL;
    }else {
        ngx_php_debug("%*s", (int)var->len, var->data);
        return var;
    }
}

int
ngx_http_php_var_set(char *z_k, size_t z_k_len, char *z_v, size_t z_v_len)
{
    ngx_http_request_t              *r;
    ngx_http_variable_t             *v;
    ngx_http_variable_value_t       *vv;
    ngx_http_core_main_conf_t       *cmcf;
    ngx_uint_t                      hash;
    ngx_str_t                       key;
    ngx_str_t                       val;
    u_char                          *valp;

    r = ngx_php_request;

    key.data = (u_char *)z_k;
    key.len = z_k_len;
    val.data = (u_char *)z_v;
    val.len = z_v_len;

    valp = ngx_palloc(r->pool, val.len + 1);
    if (valp == NULL) {
        return 1;
    }
    ngx_cpystrn(valp, val.data, val.len + 1);

    hash = ngx_hash_strlow(key.data, key.data, key.len);

    cmcf = ngx_http_get_module_main_conf(r, ngx_http_core_module);

    v = ngx_hash_find(&cmcf->variables_hash, hash, key.data, key.len);

    if (v) {
        if (!(v->flags & NGX_HTTP_VAR_CHANGEABLE)) {
            return 1;
        }

    if (v->set_handler) {
        vv = ngx_palloc(r->pool, sizeof(ngx_http_variable_value_t));
        if (vv == NULL) {
            return 1;
        }

        vv->valid = 1;
        vv->not_found = 0;
        vv->no_cacheable = 0;
        vv->data = valp;
        vv->len = val.len;

        v->set_handler(r, vv, v->data);
            return 0;
        }

        if (v->flags & NGX_HTTP_VAR_INDEXED) {
            vv = &r->variables[v->index];

            vv->valid = 1;
            vv->not_found = 0;
            vv->no_cacheable = 0;
            vv->data = valp;
            vv->len = val.len;

            return 0;
        }

    }

    return 1;

}


