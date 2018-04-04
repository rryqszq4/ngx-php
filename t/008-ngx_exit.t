# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: ngx_exit
ngx_exit
--- config
location = /ngx_exit {
    content_by_php '
        echo "ngx::_exit start\n";
        ngx::_exit(NGX_OK);
        echo "ngx::_exit end\n";
    ';
}
--- request
GET /ngx_exit
--- response_body
ngx::_exit start
