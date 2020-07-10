# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__

=== TEST 1: test ngx_cookie_get_all
--- config
	location = /ngx_cookie_get_all {
		php_content '
			echo ngx_cookie_get_all()."\n";
		';
	}
--- request
GET /ngx_cookie_get_all
--- more_headers
Cookie: foo=ngx_php; bar=ngx_cookie
--- response_body
foo=ngx_php; bar=ngx_cookie



=== TEST 2: test ngx_cookie_get
--- config
	location = /ngx_cookie_get {
		php_content '
			echo ngx_cookie_get("foo")."\n";
			echo ngx_cookie_get("bar")."\n";
		';
	}
--- request
GET /ngx_cookie_get
--- more_headers
Cookie: foo=ngx_php; bar=ngx_cookie
--- response_body
ngx_php
ngx_cookie



=== TEST 3: test ngx_cookie_set
--- config
	location = /ngx_cookie_set {
		php_content '
			ngx_cookie_set("foo=ngx_php; bar=ngx_cookie");
			echo "ok\n";
		';
	}
--- request
GET /ngx_cookie_set
--- response_headers
Set-Cookie: foo=ngx_php; bar=ngx_cookie
--- response_body
ok



=== TEST 4: test ngx_cookie_get bug
Value of cookie is null, and pass value get null.
--- config
	location = /ngx_cookie_get {
		php_content '
			echo "(".ngx_cookie_get("abc").")\n";
			echo "(".ngx_cookie_get("def").")\n";
			echo "(".ngx_cookie_get("foo").")\n";
			echo "(".ngx_cookie_get("bar").")\n";
		';
	}
--- request
GET /ngx_cookie_get
--- more_headers
Cookie: foo=ngx_php; abc=; def=; bar=ngx_cookie
--- response_body
()
()
(ngx_php)
(ngx_cookie)
