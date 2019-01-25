# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__

=== TEST 1: set content-type of response headers
--- config
location = /ngx_header {
	content_by_php '
		ngx_header_set("Content-Type", "text/html; charset=UTF-8");
		echo "Testing ngx_header!\n";
	';
}
--- request
GET /ngx_header
--- response_headers
Content-Type: text/html; charset=UTF-8
--- response_body
Testing ngx_header!



=== TEST 2: set content-length of response headers
--- config
location = /ngx_header {
	content_by_php '
		$str = "Testing ngx_header!\n";
		echo $str;
		echo $str;
		ngx_header_set("Content-Length", strlen($str));
	';
}
--- request
GET /ngx_header
--- response_headers
Content-Length: 20
--- response_body
Testing ngx_header!



=== TEST 3: set other part of reponse headers
--- config
location = /ngx_header {
	content_by_php '
		ngx_header_set("X-Foo", "abc");
		echo "Testing ngx_header!\n";
	';
}
--- request
GET /ngx_header
--- response_headers
X-Foo: abc
--- response_body
Testing ngx_header!
