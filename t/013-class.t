# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: declared classes
declared classes
--- config
location = /declared_classes {
    content_by_php '
        $class = get_declared_classes();
        foreach ($class as $v){
        	if ($v == "ngx" || !strncmp($v, "ngx_", 4)){
        		echo "{$v}\n";
        	}
        }
    ';
}
--- request
GET /declared_classes
--- response_body
ngx
ngx_log
ngx_request
ngx_socket
ngx_var
