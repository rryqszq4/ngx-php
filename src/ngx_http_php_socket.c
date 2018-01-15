/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#include "ngx_php_debug.h"
#include "ngx_http_php_module.h"
#include "ngx_http_php_socket.h"
#include "ngx_http_php_zend_uthread.h"

static void ngx_http_php_socket_handler(ngx_event_t *event);
static void ngx_http_php_socket_dummy_handler(ngx_http_request_t *r);
static void ngx_http_php_socket_resolve_handler(ngx_resolver_ctx_t *ctx);

static void ngx_http_php_socket_resolve_retval_handler(ngx_http_request_t *r, ngx_http_php_socket_upstream_t *u);

static void
ngx_http_php_socket_handler(ngx_event_t *ev)
{
    ngx_connection_t *c;
    ngx_http_request_t *r;

    r = ev->data;

    ngx_http_php_zend_uthread_resume(r);

}

static void 
ngx_http_php_socket_dummy_handler(ngx_http_request_t *r)
{
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, 
                    "ngx_php tcp socket dummy handler.");
}

static void 
ngx_http_php_socket_resolve_handler(ngx_resolver_ctx_t *ctx)
{
    ngx_http_request_t              *r;
    ngx_connection_t                *c;
    ngx_http_upstream_resolved_t    *ur;
    ngx_http_php_socket_upstream_t  *u;
    ngx_http_php_ctx_t              *php_ctx;

    u = ctx->data;
    r = u->request;
    c = r->connection;
    ur = u->resolved;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, c->log, 0,
                   "php socket resolve handler.");

    if (ctx->state) {

    }

    ur->naddrs = ctx->naddrs;
    ur->addrs = ctx->addrs;

#if (NGX_DEBUG)
    {
    u_char      text[NGX_SOCKADDR_STRLEN];
    ngx_str_t   addr;
    ngx_uint_t  i;
    }

    addr.data = text;

    for (i = 0; i < ctx->naddrs; i++ ) {
        addr.len = ngx_sock_ntop(ur->addrs[i].sockaddr, ur->addrs[i].socklen,
                                 text, NGX_SOCKADDR_STRLEN, 0);

        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, 
                       "name was resolved to %V", &addr);
    }
    }
#endif

    socklen = ur->addrs[i].socklen;

    sockaddr = ngx_palloc(r->pool, socklen);
    if (sockaddr == NULL) {

    }

    ngx_memcpy(sockaddr, ur->addrs[i].sockaddr, socklen);

    switch (sockaddr->sa_family) {
#if (NGX_HAVE_INET6)
    case AF_INET6:
        ((struct sockaddr_in6 *) sockaddr)->sin6_port = htons(ur->port);
        break;
#endif
    default: /* AF_INET */
        ((struct sockaddr_in *) sockaddr)->sin_port = htons(ur->port);
    }

    p = ngx_pnalloc(r->pool, NGX_SOCKADDR_STRLEN);
    if (p == NULL) {

    }

    len = ngx_sock_ntop(sockaddr, socklen, p, NGX_SOCKADDR_STRLEN, 1);
    ur->sockaddr = sockaddr;
    ur->socklen = socklen;

    ur->host.data = p;
    ur->host.len = len;
    ur->naddrs = 1;

    ngx_resolve_name_done(ctx);
    ur->ctx = NULL;



}

static void 
ngx_http_php_socket_resolve_retval_handler(ngx_http_request_t *r, ngx_http_php_socket_upstream_t *u)
{
    ngx_int_t               rc;
    ngx_http_php_ctx_t      *ctx;
    ngx_peer_connection_t   *pc;
    ngx_connection_t        *c;

    pc = &u->peer;

    ur = u->resolved;

    if (ur->sockaddr) {
        pc->sockaddr = ur->sockaddr;
        pc->socklen = ur->socklen;
        pc->name = &ur->host;
    }

    rc = ngx_event_connect_peer(pc);

    if (rc == NGX_ERROR) {

    }

    if (rc == NGX_BUSY) {

    }

    if (rc == NGX_DECLINED) {

    }

    /* rc == NGX_OK || rc == NGX_AGAIN || rc == NGX_DONE */

    c = pc->connection;
    c->data = u;

    c->write->handler = ngx_http_php_socket_handler;
    c->read->handler = ngx_http_php_socket_handler;

    u->write_event_handler = ngx_http_php_socket_connect_handler;
    u->read_event_handler = ngx_http_php_socket_connect_handler;

    c->sendfile &= r->connection->sendfile;

    if (c->pool == NULL) {

        /* we need separate pool here to be able to cache SSL connections */

        c->pool = ngx_create_pool(128, r->connection->log);
        if (c->pool == NULL) {

        }
    }

    c->log = r->connection->log;
    c->pool->log = c->log;
    c->read->log = c->log;
    c->write->log = c->log;

    /* init or reinit the ngx_output_chain() and ngx_chain_writer() contexts */

        

}

void
ngx_http_php_socket_connect(ngx_http_request_t *r)
{
    ngx_http_php_ctx_t                      *ctx;
    ngx_http_php_loc_conf_t                 *plcf;
    ngx_str_t                               host;
    int                                     port;
    ngx_resolver_ctx_t                      *rctx, temp;
    ngx_http_core_loc_conf_t                *clcf;

    ngx_int_t rc;
    ngx_peer_connection_t *peer;

    ngx_http_php_socket_upstream_t *u;

    u = ngx_pcalloc(r->pool, sizeof(ngx_http_php_socket_upstream_t));

    u->request = r;

    peer = &u->peer;
    peer->log = r->connection->log;
    peer->log_error = NGX_ERROR_ERR;

    ngx_memzero(&url, size_t);

    url.url.len = host.len;
    url.url.data = host.data;
    url.default_port = (in_port_t) port;
    url.no_resolve = 1;

    if (ngx_parse_url(r->pool, &url) != NGX_OK) {
        if (u.err) {

        }
    }

    if (u->peer.connection) {

    }

    u->resolved = ngx_pcalloc(r->pool, sizeof(ngx_http_upstream_resolved_t));
    if (u->resolved == NULL) {

    }

    if (url.addrs && url.addr[0].sockaddr) {
        u->resolved->sockaddr = url.addrs[0].sockaddr;
        u->resolved->socklen = url.addrs[0].socklen;
        u->resolved->naddrs = 1;
        u->resolved->host = url.addrs[0].name;
    } else {
        u->resolved->host = host;
        u->resolved->port = (in_port_t) port;
    }

    if (u->resolved->sockaddr) {
        rc = ngx_http_php_socket_resolve_retval_handler(r, u);

    }

    clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);

    temp.name = host;
    rctx = ngx_resolve_start(clcf->resolver, &temp);
    if (rctx == NULL) {

    }

    if (rctx == NGX_NO_RESOLVER) {

    }

    rctx->name = host;
    rctx->handler = ngx_http_php_socket_resolve_handler;
    rctx->data = u;
    rctx->timeout = clcf->resolver_timeout;

    u->resolved->ctx = rctx;

    if (ngx_resolve_name(rctx) != NGX_OK) {
        
    }


}

void 
ngx_http_php_socket_close(ngx_http_php_socker_t *s)
{
    return ;
}

void
ngx_http_php_socket_send(ngx_http_request_t *r)
{
    size_t size;
    ssize_t n;
    ngx_event_t *wev;
    ngx_connection_t *c;
    ngx_buf_t *b;

    wev = c->write;

    b = u->request_bufs->buf;

    for (;;) {
        size = b->last - b->pos;
        n = c->send(c, b->pos, size);

        if (n => 0) {
            b->pos += n;

            if (b->pos == b->last) {

                
            }
        }
    }


}





