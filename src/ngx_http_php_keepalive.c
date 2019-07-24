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

#include "ngx_http_php_keepalive.h"


ngx_int_t 
ngx_http_php_keepalive_init(ngx_http_request_t *r, ngx_http_php_keepalive_conf_t *kc)
{
	ngx_http_php_keepalive_cache_t 	*cached;
	ngx_uint_t 						i;

	cached = ngx_pcalloc(kc->pool, sizeof(ngx_http_php_keepalive_cache_t * kc->max_cached));

	if ( cached == NULL ) {
		return NGX_ERROR;
	}

	ngx_queue_init(&kc->cache);
	ngx_queue_init(&kc->free);

	for (i = 0; i < kc->max_cached; i++) {
		ngx_queue_insert_head(&kc->free, &cached[i].queue);
		cached[i].keepalilve_conf = kc;
	}

	return NGX_OK;
}

ngx_int_t 
ngx_http_php_keepalive_get_peer(ngx_peer_connection_t *pc, void *data)
{
	ngx_http_php_keepalive_cache_t 	*item;
	ngx_http_php_keepalive_conf_t 	*kc = data;
	ngx_queue_t 					*q, *cache;
	ngx_connection_t 				*c;

	cache = &kc->cache;

	for (q = ngx_queue_head(cache); 
		 q != ngx_queue_sentinel(cache);
		 q = ngx_queue_next(q)) 
	{
		item = ngx_queue_data(q, ngx_http_php_keepalive_cache_t, queue);
		c = item->connection;

		if (ngx_memn2cmp((u_char *) &item->sockaddr, (u_char *) pc->sockaddr, item->socklen, pc->socklen) == 0)
		{
			ngx_queue_remove(q);
			ngx_queue_insert_head(&kc->free, q);

			c->idle = 0;
	        c->log = pc->log;
#if defined(nginx_version) && (nginx_version >= 1001004)
	        c->pool->log = pc->log;
#endif
	        c->read->log = pc->log;
	        c->write->log = pc->log;

	        pc->connection = c;
	        pc->cached = 1;

	        return NGX_DONE;
		}
	}

	return NGX_DECLINED;
}

void 
ngx_http_php_keepalive_free_peer(ngx_peer_connection_t *pc, void *data, ngx_uint_t state)
{
	ngx_http_php_keepalive_cache_t 	*item;
	ngx_http_php_keepalive_conf_t 	*kc = data;
	ngx_queue_t 					*q;
	ngx_connection_t 				*c;

	if (state & NGX_PEER_FAILED) {

	}

	c = pc->connection;

	if (ngx_queue_empty(&kc->free)) {
		q = ngx_queue_last(&kc->cache);
		ngx_queue_remove(q);

		item = ngx_queue_data(q, ngx_http_php_keepalive_cache_t, queue);

		ngx_http_php_keepalive_close(item->connection);
	}else {
		q = ngx_queue_head(&kc->free);
		ngx_queue_remove(q);

		item = ngx_queue_data(q, ngx_http_php_keepalive_cache_t, queue);
	}

	ngx_queue_insert_head(&kc->cache, q);
	item->connection = c;

	if (c->read->timer_set) {
		ngx_del_timer(c->read);
	}
	if (c->write->timer_set) {
		ngx_del_timer(c->write);
	}

	c->write->handler = ngx_http_php_keepalive_dummy_handler;
	c->read->handler = ngx_http_php_keepalive_close_handler;

	c->data = item;
	c->idle = 1;
	c->log = ngx_cycle->log;
	c->read->log = ngx_cycle->log;
	c->write->log = ngx_cycle->log;
	c->pool->log = ngx_cycle->log;

	item->socklen = pc->socklen;
	ngx_memcpy(&item->sockaddr, pc->sockaddr);

}

void 
ngx_http_php_keepalive_dummy_handler(ngx_event_t *ev)
{
	ngx_log_debug0(NGX_LOG_DEBUG_HTTP, ev->log, 0, 
					"php keepalive dummy handler");
	ngx_php_debug("php keepalive dummy handler");
}

void 
ngx_http_php_keepalive_close_handler(ngx_event_t *ev)
{
	ngx_http_php_keepalive_cache_t 	*item;
	ngx_http_php_keepalive_conf_t 	*kc;

	int 				n;
   	char 				buf[1];
	ngx_connection_t 	*c;

	c = ev->data;

	if (c->close) {
		goto close;
	}

	n = recv(c->fd, buf, 1, MSG_PEEK);

    if (n == -1 && ngx_socket_errno == NGX_EAGAIN) {
        ev->ready = 0;

        if (ngx_handle_read_event(c->read, 0, NGX_FUNC_LINE) != NGX_OK) {
            goto close;
        }

        return;
    }

close:
	item = c->data;
	kc = item->keepalilve_conf;

	ngx_http_php_keepalive_close(c);

	ngx_queue_remove(&item->queue);
	ngx_queue_insert_head(&kc->free, &item->queue);
}

void 
ngx_http_php_keepalive_close(ngx_connection_t *c)
{
	ngx_destroy_pool(c->pool);
	ngx_close_connection(c);
}





