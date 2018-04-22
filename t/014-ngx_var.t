# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: ngx_var::get
ngx_var_get
--- config
location = /ngx_var_get {
	set $a 1234567890;
    content_by_php '
        $a = ngx_var::get("a");
        var_dump($a);
    ';
}
--- request
GET /ngx_var_get
--- response_body
string(10) "1234567890"



=== TEST 2: ngx_var::set
ngx_var_set
--- config
location = /ngx_var_set{
	set $a 1234567890;
    content_by_php '
    	ngx_var::set("a", "abc");
        $a = ngx_var::get("a");
        var_dump($a);
    ';
}
--- request
GET /ngx_var_set
--- response_body
string(3) "abc"
