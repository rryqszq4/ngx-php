# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: ngx_constants
ngx_constants
--- config
location = /ngx_constants {
    content_by_php '
        $ngx_const = get_defined_constants();
        foreach ($ngx_const as $k => $v) {
            if (!strncmp($k, "NGX_", 4)) {
                echo "{$k} = {$v}\n";
            }
        }
    ';
}
--- request
GET /ngx_constants
--- response_body
NGX_HTTP_PHP_MODULE_VERSION = 0.0.28
NGX_HTTP_PHP_MODULE_NAME = ngx_php
NGX_OK = 0
NGX_ERROR = -1
NGX_AGAIN = -2
NGX_BUSY = -3
NGX_DONE = -4
NGX_DECLINED = -5
NGX_ABORT = -6
NGX_HTTP_CONTINUE = 100
NGX_HTTP_SWITCHING_PROTOCOLS = 101
NGX_HTTP_PROCESSING = 102
NGX_HTTP_OK = 200
NGX_HTTP_CREATED = 201
NGX_HTTP_ACCEPTED = 202
NGX_HTTP_NO_CONTENT = 204
NGX_HTTP_PARTIAL_CONTENT = 206
NGX_HTTP_SPECIAL_RESPONSE = 300
NGX_HTTP_MOVED_PERMANENTLY = 301
NGX_HTTP_MOVED_TEMPORARILY = 302
NGX_HTTP_SEE_OTHER = 303
NGX_HTTP_NOT_MODIFIED = 304
NGX_HTTP_TEMPORARY_REDIRECT = 307
NGX_HTTP_PERMANENT_REDIRECT = 308
NGX_HTTP_BAD_REQUEST = 400
NGX_HTTP_UNAUTHORIZED = 401
NGX_HTTP_FORBIDDEN = 403
NGX_HTTP_NOT_FOUND = 404
NGX_HTTP_NOT_ALLOWED = 405
NGX_HTTP_REQUEST_TIME_OUT = 408
NGX_HTTP_CONFLICT = 409
NGX_HTTP_LENGTH_REQUIRED = 411
NGX_HTTP_PRECONDITION_FAILED = 412
NGX_HTTP_REQUEST_ENTITY_TOO_LARGE = 413
NGX_HTTP_REQUEST_URI_TOO_LARGE = 414
NGX_HTTP_UNSUPPORTED_MEDIA_TYPE = 415
NGX_HTTP_RANGE_NOT_SATISFIABLE = 416
NGX_HTTP_CLOSE = 444
NGX_HTTP_NGINX_CODES = 494
NGX_HTTP_REQUEST_HEADER_TOO_LARGE = 494
NGX_HTTPS_CERT_ERROR = 495
NGX_HTTPS_NO_CERT = 496
NGX_HTTP_TO_HTTPS = 497
NGX_HTTP_CLIENT_CLOSED_REQUEST = 499
NGX_HTTP_INTERNAL_SERVER_ERROR = 500
NGX_HTTP_NOT_IMPLEMENTED = 501
NGX_HTTP_BAD_GATEWAY = 502
NGX_HTTP_SERVICE_UNAVAILABLE = 503
NGX_HTTP_GATEWAY_TIME_OUT = 504
NGX_HTTP_INSUFFICIENT_STORAGE = 507
NGX_LOG_STDERR = 0
NGX_LOG_EMERG = 1
NGX_LOG_ALERT = 2
NGX_LOG_CRIT = 3
NGX_LOG_ERR = 4
NGX_LOG_WARN = 5
NGX_LOG_NOTICE = 6
NGX_LOG_INFO = 7
NGX_LOG_DEBUG = 8
