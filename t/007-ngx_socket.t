# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket "no_plan";

run_tests();

__DATA__
=== TEST 1: ngx_socket to a local server
ngx_socket_local
--- config
location = /ngx_socket_backend {
    default_type 'text/plain';
    content_by_php_block {
        echo "hello ngx_socket";
    }
}
location = /ngx_socket_local {
    default_type 'text/plain';
    content_by_php_block {
        $fd = ngx_socket_create();
        yield ngx_socket_connect($fd, "127.0.0.1", $TEST_NGINX_SERVER_PORT);
        $send_buf = "GET /ngx_socket_backend HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
        yield ngx_socket_send($fd, $send_buf, strlen($send_buf));
        $ret = "";
        yield ngx_socket_recv($fd, $ret, 1024);
        yield ngx_socket_close($fd);
        $ret = explode("\r\n", $ret);
        var_dump($ret[0]);
        var_dump(end($ret));
    }
}
--- request
GET /ngx_socket_local
--- response_body
string(15) "HTTP/1.1 200 OK"
string(16) "hello ngx_socket"



=== TEST 2: ngx_socket to a slow local server
ngx_socket_local_slow
--- config
location = /ngx_socket_slow_backend {
    default_type 'text/plain';
    content_by_php_block {
        yield ngx_msleep(200);
        echo "hello ngx_socket";
        yield ngx_msleep(200);
        echo " world ngx_socket";
        echo str_repeat("!", 100000);
    }
}
location = /ngx_socket_slow {
    default_type 'text/plain';
    content_by_php_block {
        $fd = ngx_socket_create();
        yield ngx_socket_connect($fd, "127.0.0.1", $TEST_NGINX_SERVER_PORT);
        $send_buf = "GET /ngx_socket_slow_backend HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
        yield ngx_socket_send($fd, $send_buf, strlen($send_buf));
        $resp = "";
        while (true) {
            $ret = "";
            yield ngx_socket_recv($fd, $ret, 1024);
            if (!is_string($ret) || $ret === "") {
                break;
            }
            $resp .= $ret;
        }
        yield ngx_socket_close($fd);
        $resp = explode("\r\n", $resp);
        $body = end($resp);
        var_dump($resp[0]);
        var_dump(substr($body, 0, 33));
        var_dump(strlen($body));
    }
}
--- request
GET /ngx_socket_slow
--- response_body
string(15) "HTTP/1.1 200 OK"
string(33) "hello ngx_socket world ngx_socket"
int(100033)


