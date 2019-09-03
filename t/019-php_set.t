# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__

=== TEST 1: directive php_set test
--- config
	php_set $foo '$a = "hello";';
	location = /php_set {
		php_content '
			echo ngx_var_get("foo")."\n";
		';
	}
--- request
GET /php_set
--- response_body
hello
