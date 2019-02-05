# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 2: ngx_socket2
ngx_socket2
--- config
resolver 8.8.8.8;
location = /ngx_socket2 {
    default_type 'application/json;charset=UTF-8';
    content_by_php '
        $fd = ngx_socket_create();
        yield ngx_socket_connect($fd, "hq.sinajs.cn", 80);
        $send_buf = "GET /list=s_sh000001 HTTP/1.0\\r\\nHost: hq.sinajs.cn\\r\\nConnection: close\\r\\n\\r\\n";
        yield ngx_socket_send($fd, $send_buf, strlen($send_buf));
        $ret = "";
        yield ngx_socket_recv($fd, $ret, 1024);
        yield ngx_socket_close($fd);
        $ret = explode("\r\n",$ret);
        var_dump($ret[0]);
        var_dump($ret[1]);
        var_dump(explode(":",$ret[2])[0].": 7?");
        var_dump($ret[3]);
        var_dump($ret[4]);
        var_dump(explode("=",$ret[6])[0]);
    ';
}
--- request
GET /ngx_socket
--- response_body
string(15) "HTTP/1.1 200 OK"
string(23) "Cache-Control: no-cache"
string(18) "Content-Length: 7?"
string(22) "Connection: Keep-Alive"
string(49) "Content-Type: application/javascript; charset=GBK"
string(21) "var hq_str_s_sh000001"



=== TEST 3: ngx_socket http
--- http_config
    server {
        listen 8999;
        server_name localhost;
    
        location = /foo {
            content_by_php 'echo "foo";';
        }
    }
--- config
    location = /ngx_socket_http {
        content_by_php '
            $fd = ngx_socket_create();
            yield ngx_socket_connect($fd, "127.0.0.1", 8999);
            $send_buf = "GET /foo HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
            yield ngx_socket_send($fd, $send_buf, strlen($send_buf));
            $ret = "";
            yield ngx_socket_recv($fd, $ret, 1024);
            yield ngx_socket_close($fd);
            $ret = explode("\r\n",$ret);
            var_dump($ret[0]);
            var_dump($ret[6]);
        ';
    }
--- request
GET ngx_socket_http
--- response_body
string(15) "HTTP/1.1 200 OK"
string(3) "foo"

