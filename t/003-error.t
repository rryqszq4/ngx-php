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
--- error_code: 500

