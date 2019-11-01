# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: content_by_php_block
Test directive content_by_php_block
--- config
location = /t1 {
    content_by_php_block {
        # php code start
        $a = 'abc';
        $b = "def";
        {
            var_dump($a);
            var_dump($b);
        }
        # php code end
    }
}
--- request
GET /t1
--- response_body
string(3) "abc"
string(3) "def"
