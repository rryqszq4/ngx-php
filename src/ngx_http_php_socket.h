/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef NGX_HTTP_PHP_SOCKET_H
#define NGX_HTTP_PHP_SOCKET_H

#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>
#include <ngx_http.h>


/*typedef struct ngx_http_php_socket_pool_s {

};*/

typedef struct ngx_http_php_socket_upstream_s
        ngx_http_php_socket_upstream_t;

typedef void (*ngx_http_php_socket_upstream_handler_pt)(ngx_http_request_t *r, 
    ngx_http_php_socket_upstream_t *u);

struct ngx_http_php_socket_upstream_s {

    ngx_http_php_socket_upstream_handler_pt     read_event_handler;
    ngx_http_php_socket_upstream_handler_pt     write_event_handler;

    ngx_msec_t      connect_timeout;
    ngx_msec_t      read_timeout;
    ngx_msec_t      send_timeout;

    ngx_http_cleanup_pt     *cleanup;
    ngx_http_request_t      *request;
    ngx_peer_connection_t   peer;

    ngx_http_upstream_resolved_t    *resolved;

    ngx_buf_t       buffer;
    size_t          buffer_size;
    off_t           length;
    unsigned        enabled_receive:1;

    ngx_chain_t     *bufs_in;

    ngx_chain_t     *busy_bufs;
    ngx_chain_t     *free_bufs;

    ngx_err_t       socket_errno;

    ngx_int_t       (*input_filter)(void *data, ssize_t bytes);

    size_t          request_len;
    ngx_chain_t     *request_bufs;
    

};

ngx_int_t ngx_http_php_socket_connect(ngx_http_request_t *r);
void ngx_http_php_socket_close(ngx_http_request_t *r);

ngx_int_t ngx_http_php_socket_send(ngx_http_request_t *r);
ngx_int_t ngx_http_php_socket_recv(ngx_http_request_t *r);



#endif