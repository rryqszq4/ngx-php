# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: ngx_sleep
ngx_sleep
--- config
location = /ngx_sleep {
    content_by_php '
        echo "ngx_sleep start\n";
        yield ngx::sleep(1);
        echo "ngx_sleep end\n";
    ';
}
--- request
GET /ngx_sleep
--- response_body
ngx_sleep start
ngx_sleep end
