# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: ngx_query_args
ngx_query_args
--- config
location = /ngx_query_args {
    content_by_php '
        echo "ngx::query_args()\n";
        var_dump(ngx::query_args());
    ';
}
--- request
GET /ngx_query_args?a=1&b=2
--- response_body
ngx::query_args()
array(2) {
  ["a"]=>
  string(1) "1"
  ["b"]=>
  string(1) "2"
}
