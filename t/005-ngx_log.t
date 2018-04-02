# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: ngx_log
ngx_log
--- config
location = /ngx_log {
    content_by_php '
        echo "ngx_log::error start\n";
        ngx_log::error(NGX_LOG_ERR, "test");
        echo "ngx_log::error end\n";
    ';
}
--- request
GET /ngx_log
--- response_body
ngx_log::error start
ngx_log::error end
