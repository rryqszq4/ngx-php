/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#ifndef NGX_HTTP_PHP_SOCKET_H
#include NGX_HTTP_PHP_SOCKET_H

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>
#include <ngx_http.h>


typedef struct ngx_http_php_socket_pool_s {

};

typedef struct ngx_http_php_socket_upstream_s {

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
    off_t           length;

    ngx_chain_t     *bufs_in

    ngx_err_t       socket_errno;

    ngx_int_t       (*input_filter)(void *data, ssize_t bytes);

    size_t          request_len;
    ngx_chain_t     *request_bufs;
    

} ngx_http_php_socket_upstream_t;

void ngx_http_php_socket_connect(ngx_http_php_socket_t *s);
void ngx_http_php_socket_close(ngx_http_php_socker_t *s);

void ngx_http_php_socket_send();
void ngx_http_php_socket_recv();



#endif