# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: error
error
--- config
location = /error {
    content_by_php '
        echo "hello ngx_php";
        $a = new abc();
    ';
}
--- request
GET /error
--- response_body
hello ngx_php
Fatal error: Uncaught Error: Class 'abc' not found in ngx_php eval code:3
Stack trace:
#0 {main}
  thrown in ngx_php eval code on line 3

