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



=== TEST 2: ngx_query_args bug param is null
ngx_query_args bug param is null
--- config
location = /ngx_query_args {
    content_by_php '
        echo "ngx::query_args()\n";
        var_dump(ngx::query_args());
    ';
}
--- request
GET /ngx_query_args?a=&b=2
--- response_body
ngx::query_args()
array(2) {
  ["a"]=>
  string(0) ""
  ["b"]=>
  string(1) "2"
}



=== TEST 3: ngx_query_args is null
When ngx_query_args is null that return value is array()
--- config
location =/t3 {
    content_by_php_block {
        var_dump(ngx_query_args());
    }
}
--- request
GET /t3
--- response_body
array(0) {
}
