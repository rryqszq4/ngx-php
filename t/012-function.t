# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: internal functions
internal functions
--- config
location = /internal-functions {
    content_by_php '
        $func = get_defined_functions();
        foreach ($func["internal"] as $v) {
        	if (!strncmp($v, "ngx_", 4)) 
            {
            	echo "{$v}\n";
            }
        }
    ';
}
--- request
GET /internal-functions
--- response_body
ngx_exit
ngx_query_args
ngx_post_args
ngx_sleep
ngx_log_error
ngx_request_method
ngx_request_document_root
ngx_request_document_uri
ngx_request_script_name
ngx_request_script_filename
ngx_request_query_string
ngx_request_uri
ngx_request_server_protocol
ngx_request_remote_addr
ngx_request_server_addr
ngx_request_remote_port
ngx_request_server_port
ngx_request_server_name
ngx_request_headers
ngx_socket_create
ngx_socket_connect
ngx_socket_close
ngx_socket_send
ngx_socket_recv
ngx_var_get
ngx_var_set
ngx_header_set
ngx_header_get
ngx_header_gets
