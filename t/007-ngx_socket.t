# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: ngx_socket
ngx_socket
--- config
resolver 8.8.8.8;
location = /ngx_socket {
	default_type 'application/json;charset=UTF-8';
    content_by_php '
        yield ngx_socket::connect("hq.sinajs.cn", 80);
        yield ngx_socket::send("GET /list=s_sh000001 HTTP/1.0\r\nHost: hq.sinajs.cn\r\nConnection: close\r\n\r\n");
        yield $ret = ngx_socket::recv(1024);
        yield ngx_socket::close();
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
