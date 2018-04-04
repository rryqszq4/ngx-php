/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#ifndef NGX_HTTP_PHP_CORE_H
#define NGX_HTTP_PHP_CORE_H

#include <ngx_http.h>
#include <php_embed.h>
#include "php/impl/php_ngx.h"

#include "ngx_http_php_socket.h"

#define OUTPUT_CONTENT  1<<0
#define OUTPUT_OPCODE   1<<1
#define OUTPUT_STACK    1<<2

extern ngx_http_request_t *ngx_php_request;

typedef struct ngx_http_php_state_t {
    unsigned php_init;
    unsigned php_shutdown;
} ngx_http_php_state_t;

typedef enum code_type_t {
    NGX_HTTP_PHP_CODE_TYPE_FILE,
    NGX_HTTP_PHP_CODE_TYPE_STRING
} code_type_t;

typedef struct ngx_http_php_code_t {
    union code {
        char *file;
        char *string;
    } code;
    code_type_t code_type;
    ngx_str_t code_id;
} ngx_http_php_code_t;

#if defined(NDK) && NDK
typedef struct {
    size_t size;
    ngx_str_t var_name;
    ngx_str_t script;
    ngx_http_php_code_t *code;
    ngx_str_t result;
} ngx_http_php_set_var_data_t;
#endif

typedef struct ngx_http_php_rputs_chain_list_t {
    ngx_chain_t **last;
    ngx_chain_t *out;
} ngx_http_php_rputs_chain_list_t;

typedef struct ngx_http_php_capture_node_t {
    ngx_str_t capture_uri;
    ngx_buf_t *capture_buf;
    ngx_str_t capture_str;
} ngx_http_php_capture_node_t;

typedef struct ngx_http_php_ctx_t {
    ngx_http_php_rputs_chain_list_t *rputs_chain;
    size_t body_length;
    ngx_str_t request_body_ctx;
    unsigned request_body_more : 1;
    unsigned read_request_body_done : 1;

    unsigned enable_async : 1;
    unsigned enable_thread : 1;
    unsigned is_capture_multi : 1;
    unsigned is_capture_multi_complete : 1;

    ngx_str_t capture_uri;
    ngx_buf_t *capture_buf;
    ngx_str_t capture_str;
    zval *closure;

    ngx_array_t *capture_multi;
    ngx_uint_t capture_multi_complete_total;

    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t pthread_id;

    ngx_int_t error;

    unsigned output_type;
    unsigned opcode_logo;
    unsigned stack_logo;
    ngx_uint_t stack_depth;
    
    unsigned rewrite_phase : 1;
    unsigned access_phase : 1;
    unsigned content_phase : 1;

    ngx_int_t phase_status;

    zval *generator_closure;

    ngx_int_t delay_time;
    ngx_event_t sleep;

    ngx_http_php_socket_upstream_t  *upstream;
    ngx_str_t   host;
    in_port_t   port;

} ngx_http_php_ctx_t;


ngx_http_php_code_t *ngx_http_php_code_from_file(ngx_pool_t *pool, ngx_str_t *code_file_path);
ngx_http_php_code_t *ngx_http_php_code_from_string(ngx_pool_t *pool, ngx_str_t *code_str);

#define NGX_HTTP_PHP_NGX_INIT ngx_http_php_request_init(r TSRMLS_CC);   \
        php_ngx_request_init(TSRMLS_C);                                 \
        zend_first_try {

#define NGX_HTTP_PHP_NGX_SHUTDOWN } zend_catch {        \
        } zend_end_try();                               \
        ngx_http_php_request_clean(TSRMLS_C);           \
        php_ngx_request_shutdown(TSRMLS_C);

// php_ngx run
//ngx_int_t ngx_php_embed_run(ngx_http_request_t *r, ngx_http_php_code_t *code);
ngx_int_t ngx_php_ngx_run(ngx_http_request_t *r, ngx_http_php_state_t *state, ngx_http_php_code_t *code);

ngx_int_t ngx_php_eval_code(ngx_http_request_t *r, ngx_http_php_state_t *state, ngx_http_php_code_t *code);
ngx_int_t ngx_php_eval_file(ngx_http_request_t *r, ngx_http_php_state_t *state, ngx_http_php_code_t *code);

ngx_int_t ngx_php_get_request_status();
ngx_int_t ngx_php_set_request_status(ngx_int_t rc);

// php_ngx sapi call_back
size_t ngx_http_php_code_ub_write(const char *str, size_t str_length TSRMLS_DC);
void ngx_http_php_code_flush(void *server_context);
void ngx_http_php_code_log_message(char *message);
void ngx_http_php_code_register_server_variables(zval *track_vars_array TSRMLS_DC);
int ngx_http_php_code_read_post(char *buffer, uint count_bytes TSRMLS_DC);
char *ngx_http_php_code_read_cookies(TSRMLS_D);
int ngx_http_php_code_header_handler(sapi_header_struct *sapi_header, sapi_header_op_enum op, sapi_headers_struct *sapi_headers TSRMLS_DC);

void (*old_zend_error_cb)(int, const char *, const uint, const char *, va_list);
void ngx_php_error_cb(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);

#endif