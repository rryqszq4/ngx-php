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
