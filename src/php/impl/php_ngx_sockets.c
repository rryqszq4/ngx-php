/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#include "../../ngx_http_php_module.h"
#include "../../ngx_http_php_socket.h"
#include "php_ngx_sockets.h"

static int le_socket;
#define le_socket_name php_ngx_sockets_le_socket_name

//static int php_ngx_socket_le_socket(void);
static php_ngx_socket *php_ngx_socket_create(void);
static void php_ngx_socket_destroy(zend_resource *rsrc);

/*static int php_ngx_socket_le_socket(void)
{
    return le_socket;
}*/

static php_ngx_socket *php_ngx_socket_create(void)
{
    ngx_http_request_t              *r;
    ngx_http_php_ctx_t              *ctx;
    ngx_http_php_socket_upstream_t  *u;
    php_ngx_socket                  *ngx_sock;

    ngx_sock = emalloc(sizeof(php_ngx_socket));
    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    u = ctx->upstream;
    ngx_sock->upstream = u;
    ngx_sock->type = 0;
    ngx_sock->error = 0;

    return ngx_sock;
}

static void php_ngx_socket_destroy(zend_resource *rsrc)
{
    php_ngx_socket  *ngx_sock;

    ngx_sock = rsrc->ptr;

    efree(ngx_sock);
}

PHP_FUNCTION(ngx_socket_create)
{
    zend_long       arg1, arg2, arg3;
    php_ngx_socket  *ngx_sock;

    arg1 = AF_INET;
    arg2 = SOCK_STREAM;
    arg3 = SOL_TCP;
    ngx_sock = php_ngx_socket_create();

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|lll", &arg1, &arg2, &arg3) == FAILURE) {
        return ;
    }

    ngx_sock->type = arg1;

    RETURN_RES(zend_register_resource(ngx_sock, le_socket));
}

PHP_FUNCTION(ngx_socket_connect)
{
    zval                *arg1;
    php_ngx_socket      *ngx_sock;
    char                *addr;
    size_t              addr_len;
    zend_long           port = 0;
    //int                 retval;

    ngx_http_request_t  *r;
    ngx_http_php_ctx_t  *ctx;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs|l", &arg1, &addr, &addr_len, &port) == FAILURE) {
        return ;
    }

    if ((ngx_sock = (php_ngx_socket *)zend_fetch_resource(Z_RES_P(arg1), le_socket_name, le_socket)) == NULL) {
        RETURN_FALSE;
    }

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    switch(ngx_sock->type) {
        case AF_INET: {
            ctx->host.data = ngx_palloc(r->pool, addr_len + 1);
            ctx->host.len = addr_len;

            ngx_memcpy(ctx->host.data, (u_char *)addr, addr_len + 1);
            ctx->host.data[addr_len] = '\0';

            ctx->port = port;
            ngx_http_php_socket_connect(r);
            break;
        }

        default:
            php_error_docref(NULL, E_WARNING, "Unsupported socket type %d", ngx_sock->type);
            RETURN_FALSE;
    }

    /*if (retval != NGX_OK) {
        php_error_docref(NULL, E_WARNING, "unable to connect");
        RETURN_FALSE;
    }*/

    RETURN_TRUE;
}

PHP_FUNCTION(ngx_socket_close)
{
    zval                *arg1;
    php_ngx_socket      *ngx_sock;

    ngx_http_request_t  *r;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &arg1) == FAILURE) {
        return ;
    }

    if ((ngx_sock = (php_ngx_socket *)zend_fetch_resource(Z_RES_P(arg1), le_socket_name, le_socket)) == NULL) {
        RETURN_FALSE;
    }

    r = ngx_php_request;

    ngx_http_php_socket_close(r);

    RETURN_TRUE;
}

PHP_FUNCTION(ngx_socket_send)
{
    zval                            *arg1;
    php_ngx_socket                  *ngx_sock;
    size_t                          buf_len, retval;
    zend_long                       len;
    char                            *buf;

    ngx_http_request_t              *r;
    ngx_http_php_ctx_t              *ctx;
    ngx_http_php_socket_upstream_t  *u;
    ngx_str_t                       ns;
    ngx_buf_t                       *b;
    ngx_chain_t                     *cl;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rsl", &arg1, &buf, &buf_len, &len) == FAILURE) {
        return ;
    }

    if (len < 0) {
        php_error_docref(NULL, E_WARNING, "Length cannot be negative");
        RETURN_FALSE;
    }

    if ((ngx_sock = (php_ngx_socket *)zend_fetch_resource(Z_RES_P(arg1), le_socket_name, le_socket)) == NULL) {
        RETURN_FALSE;
    }

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ns.data = (u_char *)buf;
    ns.len = buf_len;

    b = ngx_create_temp_buf(r->pool, ns.len + 1);

    cl = ngx_alloc_chain_link(r->pool);

    cl->buf = b;
    cl->next = NULL;

    u = ctx->upstream;
    u->request_bufs = cl;

    b->last = ngx_copy(b->last, ns.data, ns.len);

    retval = ngx_http_php_socket_send(r);

    RETURN_LONG(retval);
}

PHP_FUNCTION(ngx_socket_recv)
{
    zval                            *arg1, *buf;
    //zend_string                   *recv_buf;
    php_ngx_socket                  *ngx_sock;
    int                             retval;
    zend_long                       len = 1024;

    ngx_http_request_t              *r;
    ngx_http_php_ctx_t              *ctx;
    ngx_http_php_socket_upstream_t  *u;
    //ngx_buf_t                       *b;
    //long                            size = 1024;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rz/|l", &arg1, &buf, &len) == FAILURE) {
        return ;
    }

    if ((ngx_sock = (php_ngx_socket *)zend_fetch_resource(Z_RES_P(arg1), le_socket_name, le_socket)) == NULL) {
        RETURN_FALSE;
    }

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    u = ctx->upstream;
    u->buffer_size = len;
    //b = &u->buffer;

    ctx->recv_buf = buf;

    retval = ngx_http_php_socket_recv(r);

    //ZVAL_STRINGL(buf, (char *)b->pos, b->last - b->pos);

    /*if (retval != NGX_OK) {
        php_error_docref(NULL, E_WARNING, "unable to read from socket");
        RETURN_FALSE;
    }*/

    RETURN_LONG(retval);
}

void php_impl_ngx_sockets_init(int module_number TSRMLS_DC)
{
    le_socket = zend_register_list_destructors_ex(php_ngx_socket_destroy, NULL, le_socket_name, module_number);
}

