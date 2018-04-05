# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: ngx_post_args
ngx_post_args
--- config
location = /ngx_post_args {
    content_by_php '
        echo "ngx::post_args()\n";
        var_dump(ngx::post_args());
    ';
}
--- more_headers
Content-type: application/x-www-form-urlencoded
--- request
POST /ngx_post_args
a=1&b=2
--- response_body
ngx::post_args()
array(2) {
  ["a"]=>
  string(1) "1"
  ["b"]=>
  string(1) "2"
}
