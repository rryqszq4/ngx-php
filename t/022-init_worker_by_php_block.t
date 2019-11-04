# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: init_worker_by_php_block
Test directive init_worker_by_php_block
--- http_config
    init_worker_by_php_block {
        define('AAA', 'aaa');
        define("BBB", 'bbb');
    }
--- config
location = /t1 {
    content_by_php_block {
        var_dump(AAA);
        var_dump(BBB);
    }
}
--- request
GET /t1
--- response_body
string(3) "aaa"
string(3) "bbb"
