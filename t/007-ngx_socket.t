# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket "no_plan";

run_tests();

__DATA__
=== TEST 2: ngx_socket2
ngx_socket2
--- config
resolver 1.1.1.1;
location = /ngx_socket2 {
    default_type 'application/json;charset=UTF-8';
    content_by_php '
        $fd = ngx_socket_create();
        yield ngx_socket_connect($fd, "httpbin.org", 80);
        $send_buf = "GET /get HTTP/1.1\\r\\nHost: httpbin.org\\r\\nConnection: close\\r\\n\\r\\n";
        yield ngx_socket_send($fd, $send_buf, strlen($send_buf));
        $ret = "";
        yield ngx_socket_recv($fd, $ret, 1024);
        yield ngx_socket_close($fd);
        $ret = explode("\r\n",$ret);
        var_dump($ret[0]);
        var_dump(explode(":",$ret[1])[0].": GMT");
        var_dump($ret[2]);
        var_dump(explode(":",$ret[3])[0]);
    ';
}
--- request
GET /ngx_socket2
--- response_body
string(15) "HTTP/1.1 200 OK"
string(9) "Date: GMT"
string(30) "Content-Type: application/json"
string(14) "Content-Length"
