<?php

function set_byte3($n) {
    return chr($n&0xff)
            .chr(($n>>8)&0xff)
            .chr(($n>>16)&0xff);
}

function set_byte4($n){
    return chr($n&0xff)
            .chr(($n>>8)&0xff)
            .chr(($n>>16)&0xff)
            .chr(($n>>24)&0xff);
}

function test_mysql(){

$fd = ngx_socket_create();
var_dump($fd);
yield ngx_socket_connect($fd, "127.0.0.1", 3306);

yield ngx_sleep(1);

$result = "";
yield ngx_socket_recv($fd, $result);
echo "<br />";
var_dump("receive: ".$result);
echo "<br />";echo "<br />";echo "<br />";
$bytes = array();

$len = unpack('v',substr($result,0,4));
$len = $len[1];
$num = substr($result,3,1);
$num = ord($num);

var_dump("packet length: ".$len);

$data = substr($result, 4, $len);
echo "<br />";
//var_dump(strlen($data));
echo "<br />";
$protocol_ver = ord(substr($data, 0, 1));

var_dump("protocol version: ".$protocol_ver);

var_dump("server version: ".substr($data, 1, $protocol_ver));

$thread_id = unpack("V", substr($data, 1+$protocol_ver+1, 4));
$thread_id = $thread_id[1];
var_dump("thread id: ".$thread_id);

$scramble = substr($data, 1+$protocol_ver+1+4, 8);

var_dump($scramble);

#$pos = 1+$protocol_ver+4 + 9;

$capabilities = unpack('v', substr($data, 1+$protocol_ver+1+4+9, 2));
$capabilities = $capabilities[1];
var_dump("server capabilities: ".$capabilities);

$server_lang = ord(substr($data, 1+$protocol_ver+1+4+9+2,1));
var_dump("server lang: ".$server_lang);   

$server_status = unpack('v', substr($data, 1+$protocol_ver+1+4+9+2+1, 2));
$server_status = $server_status[1];
var_dump("server status: ".$server_status);

$more_capabilites = unpack('v', substr($data, 1+$protocol_ver+1+4+9+2+1+2,2));
$more_capabilites = $more_capabilites[1];
var_dump("more capabilities: ".$more_capabilites);

$capabilities = $capabilities | ($more_capabilites << 16);
var_dump("server capabilities: ".$capabilities);

$scramble_2 = substr($data, 1+$protocol_ver+1+4+9+2+1+2+2+1+10, 21-8-1);
var_dump($scramble_2);

$scramble = $scramble.$scramble_2;
var_dump($scramble);

#$data = substr($data, $pos);

#$data = substr($data, 1, $protocol_ver+1);

for ($i = 0; $i < strlen($result);$i++) {
        $bytes[] = ord($result[$i]);
                #var_dump(bin2hex(chr($bytes[$i])).'   '.chr($bytes[$i]));
}
//var_dump($bytes);

#client 
$client_flags = 0x3f7cf;
$database = "";
$user = "root";
$password = "";
$stage1 = sha1($password,1);
$stage2 = sha1($stage1,1);
$stage3 = sha1($scramble.$stage2,1);
/*$n = strlen($stage1);
$bytes = array();
for ($i = 0; $i < $n; $i++) {
    $bytes[] = ord($stage3[$i]) ^ ord($stage1[$i]);
}


$token = '';
foreach ($bytes as $ch) {
    $token .= chr($ch);
}*/
$token = $stage1 ^ $stage3;


$req = set_byte4($client_flags)
        .set_byte4(1024*1024)
        ."\0"
        .str_repeat("\0", 23)
        .$user."\0"
        .chr(strlen($token)).$token
        .$database."\0";
var_dump($req);

$pack_len = 4 + 4 + 1 + 23 + strlen($user) + 1 + strlen($token) + 1 + strlen($database) + 1;
var_dump($pack_len);

#var_dump(set_byte3($pack_len).chr(1));

$pack = substr_replace(pack("V", $pack_len), chr(1), 3, 1).$req;
var_dump($pack);

yield ngx_socket_send($fd, $pack, strlen($pack));

yield ngx_socket_recv($fd, $result);
var_dump("receive: ".$result);

$len = unpack('v',substr($result, 0,4));
$len = $len[1];
$data = substr($result, 4, $len);
//var_dump(ord($data));

$query = "SELECT * FROM mysql.user limit 0, 200;";
#$query = "select sleep(1);";
$req = chr(0x03).$query;
$pack_len = strlen($query) + 1;
$pack = set_byte3($pack_len).chr(0).$req;

yield ngx_socket_send($fd, $pack, strlen($pack));

yield ngx_socket_recv($fd, $result);


var_dump("receive: ".$result);



$len = unpack('v',substr($result, 0,4));
$len = $len[1];
$data = substr($result, 4, $len);
//var_dump(ord($data));





var_dump(123);

#$result1 = $tcpsock->receive();
#var_dump($result1);


//ngx_log::error(ngx_log::ERR, "test");
//var_dump(ngx_log::ERR);

yield ngx_socket_close($fd);
}

?>