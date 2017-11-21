# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: ngx_request
ngx_request
--- config
location = /ngx_request {
    content_by_php '
        var_dump(ngx_request::document_uri());
    ';
}
--- request
GET /ngx_request
--- response_body
string(12) "/ngx_request"

