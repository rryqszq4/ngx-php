# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__

=== TEST 1: rewrite_by_php test
--- config
	set $sel 0;
	location = /rewrite1 {
		rewrite_by_php '
			ngx_var_set("sel", 1);
		';
		return 200 $sel;
	}
--- request
GET /rewrite1
--- response_body eval
"1"



=== TEST 2: conflicts with the native if directive.
--- config
	set $sel 0;

	rewrite_by_php '
		ngx_var_set("sel", "1");
	';

	location = /rewrite2 {
		if ($sel = 1) {
			return 200 $sel;
		}

		return 404 $sel;
	}
--- request
GET /rewrite2
--- error_code: 200
--- response_body eval
"1"



=== TEST 3: directive be overwrite for test one.
--- config
	set $position null;

	rewrite_by_php '
		ngx_var_set("position", "in the server directive");
	';

	location = /rewrite3 {
		return 200 $position;
	}
--- request
GET /rewrite3
--- response_body eval
"in the server directive"



=== TEST 4: directive be overwrite for test two.
--- config
	set $position null;

	rewrite_by_php '
		ngx_var_set("position", "in the server directive");
	';

	location = /rewrite4 {
		rewrite_by_php '
			ngx_var_set("position", "in the location directive");
		';

		return 200 $position;
	}
--- request
GET /rewrite4
--- response_body eval
"in the location directive"
