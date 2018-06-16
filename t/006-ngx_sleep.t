# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: ngx_sleep
ngx_sleep
--- config
location = /ngx_sleep {
    content_by_php '
        echo "ngx_sleep start\n";
        yield ngx::sleep(1);
        echo "ngx_sleep end\n";
    ';
}
--- request
GET /ngx_sleep
--- response_body
ngx_sleep start
ngx_sleep end



=== TEST 2: sleep function
sleep function
--- config
location =/nginx_sleep {
	content_by_php '
		require_once "/home/travis/build/rryqszq4/ngx_php7/t/lib/sleep.php";
		echo "ngx_sleep start\n";
        yield from nginx_sleep();
        echo "ngx_sleep end\n";
	';
}
--- request
GET /nginx_sleep
--- response_body
ngx_sleep start
ngx_sleep end
