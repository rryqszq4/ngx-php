# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

$ENV{'TEST_NGINX_BUILD_DIR'} = $ENV{'TRAVIS_BUILD_DIR'};

run_tests();

__DATA__
=== TEST 1: test mysql
test mysql
--- config
location =/ngx_mysql {
	content_by_php '
		require_once("$TEST_NGINX_BUILD_DIR/t/lib/mysql.php");
        $m = new ngx\php\mysql();
        yield from $m->connect("127.0.0.1","3306","ngx_php","ngx_php","world");
        $sql = "select * from world.city order by ID asc limit 1 ;";
        $ret = yield from $m->query($sql);
        echo implode(",",array_values($ret[0]));
        yield from $m->close();
	';
}
--- request
GET /ngx_mysql
--- response_body
1,Kabul,AFG,Kabol,1780000



=== TEST 2: test mysql keepalive
test mysql keepalive
--- config
server {
	listen 80;
	server_name localhost;
	php_keepalive 32;
	location =/ngx_mysql2 {
		content_by_php '
			require_once("$TEST_NGINX_BUILD_DIR/t/lib/mysql.php");
	        $m = new ngx\php\mysql();
	        yield from $m->connect("127.0.0.1","3306","ngx_php","ngx_php","world");
	        $sql = "select * from world.city order by ID asc limit 1 ;";
	        $ret = yield from $m->query($sql);
	        echo implode(",",array_values($ret[0]));
	        yield from $m->close();
		';
	}
}
--- request
GET /ngx_mysql2
--- response_body
1,Kabul,AFG,Kabol,1780000