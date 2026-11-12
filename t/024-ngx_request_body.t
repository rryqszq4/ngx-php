# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: ngx_request_body
Test func ngx_request_body
--- config
location = /t1 {
    content_by_php_block {
        $body = ngx_request_body();
        echo $body."\n";
    }
}
--- request
POST /t1
Hello world
--- response_body
Hello world


=== TEST 2: ngx_request_body fragmented across multiple chunks
Request body should include all fragments in order
--- config
location = /t2 {
    content_by_php_block {
        $body = ngx_request_body();
        echo $body."\n";
    }
}
--- raw_request eval
"POST /t2 HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n6\r\n world\r\n0\r\n\r\n"
--- response_body
Hello world
