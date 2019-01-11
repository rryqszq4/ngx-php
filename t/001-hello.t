# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

$ENV{'TEST_NGINX_BUILD_DIR'} = $ENV{'TRAVIS_BUILD_DIR'};

run_tests();

__DATA__
=== TEST 1: hello, ngx_php
This is just a simple demonstration of the
echo directive provided by ngx_php.
--- config
location = /t {
    content_by_php '
        echo "hello ngx_php!\n";
    ';
}
--- request
GET /t
--- response_body
hello ngx_php!



=== TEST 2: include hello
include hello
--- config
location =/include {
	content_by_php '
		include "$TEST_NGINX_BUILD_DIR/t/lib/hello.php";
	';
}
--- request
GET /include
--- response_body
hello, world!
