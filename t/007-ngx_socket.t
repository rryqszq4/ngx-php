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
        yield ngx_socket_connect($fd, "httpbingo.org", 80);
        $send_buf = "GET /get HTTP/1.1\\r\\nHost: httpbingo.org\\r\\nConnection: close\\r\\n\\r\\n";
        yield ngx_socket_send($fd, $send_buf, strlen($send_buf));
        $ret = "";
        yield ngx_socket_recv($fd, $ret, 1024);
        yield ngx_socket_close($fd);
        $ret = explode("\r\n",$ret);
        var_dump($ret[0]);
        var_dump(explode(":",$ret[4])[0].": GMT");
        var_dump($ret[3]);
        var_dump(explode(":",$ret[5])[0]);
    ';
}
--- request
GET /ngx_socket2
--- response_body
string(15) "HTTP/1.1 200 OK"
string(9) "date: GMT"
string(46) "content-type: application/json; encoding=utf-8"
string(14) "content-length"
