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

#include "ngx_http_php_sleep.h"
#include "ngx_http_php_zend_uthread.h"

static void ngx_http_php_sleep_cleanup(void *data);

static void ngx_http_php_sleep_handler(ngx_event_t *ev);

static void
ngx_http_php_sleep_cleanup(void *data)
{
    ngx_http_request_t *r = data;
    ngx_http_php_ctx_t *ctx;

    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {
        return ;
    }

    if (ctx->sleep.timer_set) {
        ngx_del_timer(&ctx->sleep);
        return ;
    }
}

ngx_int_t
ngx_http_php_sleep(ngx_http_request_t *r) 
{
    ngx_http_cleanup_t *cln;
    ngx_http_php_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    
    ctx->phase_status = NGX_AGAIN;

    //ngx_memzero(&ctx->sleep, sizeof(ngx_event_t));

    ctx->sleep.handler = ngx_http_php_sleep_handler;
    ctx->sleep.log = r->connection->log;
    ctx->sleep.data = r;

    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,"%p %p %d", r, &ctx->sleep, ctx->delay_time);

    ngx_php_debug("r:%p, &ctx->sleep:%p, ctx->delay_time:%d", r, &ctx->sleep, (int)ctx->delay_time);

    ngx_add_timer(&ctx->sleep, (ngx_msec_t) ctx->delay_time);

    cln = ngx_http_cleanup_add(r, 0);
    if (cln == NULL) {
        return NGX_ERROR;
    }

    cln->handler = ngx_http_php_sleep_cleanup;
    cln->data = r;

    r->keepalive = 0;

    return NGX_OK;
}

static void 
ngx_http_php_sleep_handler(ngx_event_t *ev)
{
    ngx_http_request_t *r;

    r = ev->data;
    
    // todo uthread_resume
    ngx_http_php_zend_uthread_resume(r);

    //ngx_http_core_run_phases(r);
}


