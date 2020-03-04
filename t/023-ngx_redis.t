# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

$ENV{'TEST_NGINX_BUILD_DIR'} = $ENV{'TRAVIS_BUILD_DIR'};

run_tests();

__DATA__

=== TEST 1: test_redis
test redis
--- config
location =/t1 {
	content_by_php_block {
		require_once("$TEST_NGINX_BUILD_DIR/t/lib/redis.php");
		$r = new php\ngx\Redis();
        yield from $r->connect("127.0.0.1","6379");
        $ret = yield from $r->set("foo", "123456789");
        $res = yield from $fd->get("foo");
        echo $res."\n";
        unset($r);
	}
}
--- request
GET /t1
--- response_body
123456789
