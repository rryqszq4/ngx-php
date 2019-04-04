# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__

=== TEST 1: redirect uri
--- config
	location /test {
		content_by_php '
			echo "test";
		';
	}

	location /ngx_redirect {
		content_by_php '
			ngx_redirect("/test");
		';
	}
--- request
GET /ngx_redirect
--- raw_response_headers_like: Location: /test\r\n
--- response_body_like: 302 Found
--- error_code: 302


