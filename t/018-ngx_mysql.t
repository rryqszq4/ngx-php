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
        $m = new php\\ngx\mysql();
        yield from $m->connect("127.0.0.1","3306","ngx_php","ngx_php","world");
        $sql = "select * from world.city order by ID asc limit 1 ;";
        $ret = yield from $m->query($sql);
        echo implode(",",array_values($ret[0]))."\n";
        yield from $m->close();
    ';
}
--- request
GET /ngx_mysql
--- response_body
1,Kabul,AFG,Kabol,1780000



=== TEST 2: test function ngx_socket_clear
test clear
--- config
location =/ngx_mysql_clear {
    content_by_php '
        require_once("$TEST_NGINX_BUILD_DIR/t/lib/mysql.php");
        $m = new php\\ngx\mysql();
        yield from $m->connect("127.0.0.1","3306","ngx_php","ngx_php","world");
        $sql = "select * from world.city order by ID asc limit 1 ;";
        $ret = yield from $m->query($sql);
        echo implode(",",array_values($ret[0]))."\n";
        $m->clear();
    ';
}
--- request
GET /ngx_mysql_clear
--- response_body
1,Kabul,AFG,Kabol,1780000



=== TEST 3: test function __destruct
test unset
--- config
location =/ngx_mysql_destruct {
    content_by_php '
        require_once("$TEST_NGINX_BUILD_DIR/t/lib/mysql.php");
        $m = new php\\ngx\mysql();
        yield from $m->connect("127.0.0.1","3306","ngx_php","ngx_php","world");
        $sql = "select * from world.city order by ID asc limit 1 ;";
        $ret = yield from $m->query($sql);
        echo implode(",",array_values($ret[0]))."\n";
        unset($m);
    ';
}
--- request
GET /ngx_mysql_destruct
--- response_body
1,Kabul,AFG,Kabol,1780000



=== TEST 4: test double query
double query
--- config
location =/ngx_mysql_destruct {
    content_by_php '
        require_once("$TEST_NGINX_BUILD_DIR/t/lib/mysql.php");
        $m = new php\\ngx\mysql();
        yield from $m->connect("127.0.0.1","3306","ngx_php","ngx_php","world");
        $sql = "select * from world.city order by ID asc limit 1 ;";
        $ret = yield from $m->query($sql);
        echo implode(",",array_values($ret[0]))."\n";
        
        $ret = yield from $m->query($sql);
        echo implode(",",array_values($ret[0]))."\n";

        $ret = yield from $m->query($sql);
        echo implode(",",array_values($ret[0]))."\n";
    ';
}
--- request
GET /ngx_mysql_destruct
--- response_body
1,Kabul,AFG,Kabol,1780000
1,Kabul,AFG,Kabol,1780000
1,Kabul,AFG,Kabol,1780000