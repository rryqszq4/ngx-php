<?php
/**
 *  Copyright(c) 2017-2018 rryqszq4 rryqszq@gmail.com
 */

namespace ngx\php;

class mysql {

    const VERSION = "0.1.0";

    private $socket = null;

    public function __construct() {
        $this->socket = ngx_socket_create();
    }

    private function set_byte3($n) {
        return chr($n&0xff)
                .chr(($n>>8)&0xff)
                .chr(($n>>16)&0xff);
    }

    private function set_byte4($n){
        return chr($n&0xff)
                .chr(($n>>8)&0xff)
                .chr(($n>>16)&0xff)
                .chr(($n>>24)&0xff);
    }

    public function connect() {
        yield ngx_socket_connect($this->socket, "127.0.0.1", 3306);

        //yield ngx_sleep(1);

        $result = "";
        yield ngx_socket_recv($this->socket, $result, 4);
        
        var_dump("receive: ".$result);
        
        $bytes = array();

        $len = unpack('v',substr($result,0,4));
        $len = $len[1];
        
        yield ngx_socket_recv($this->socket, $data, $len);

        var_dump("packet length: ".$len);

        $protocol_ver = ord(substr($data, 0, 1));

        var_dump("protocol version: ".$protocol_ver);

        var_dump(strpos($data, "\0", 2));

        var_dump("server version: ".substr($data, 1, strpos($data, "\0", 2)-1));

        $pos = strpos($data, "\0", 2)+1;

        $thread_id = unpack("V", substr($data, $pos, 4));
        $thread_id = $thread_id[1];
        var_dump("thread id: ".$thread_id);

        $pos += 4;
        var_dump($pos);

        $scramble = substr($data, $pos, 8);

        var_dump($scramble);

        #$pos = 1+$protocol_ver+4 + 9;

        $capabilities = unpack('v', substr($data, $pos+9, 2));
        $capabilities = $capabilities[1];
        var_dump("server capabilities: ".$capabilities);

        $server_lang = ord(substr($data, $pos+9+2,1));
        var_dump("server lang: ".$server_lang);

        $server_status = unpack('v', substr($data, $pos+9+2+1, 2));
        $server_status = $server_status[1];
        var_dump("server status: ".$server_status);

        $more_capabilites = unpack('v', substr($data, $pos+9+2+1+2,2));
        $more_capabilites = $more_capabilites[1];
        var_dump("more capabilities: ".$more_capabilites);

        $capabilities = $capabilities | ($more_capabilites << 16);
        var_dump("server capabilities: ".$capabilities);

        $scramble_2 = substr($data, $pos+9+2+1+2+2+1+10, 21-8-1);
        var_dump($scramble_2);

        $scramble = $scramble.$scramble_2;
        var_dump("scramble: ".$scramble);
        var_dump(strlen($scramble));

        #$data = substr($data, $pos);

        #$data = substr($data, 1, $protocol_ver+1);

        $hex_arr="";
        $bin_arr="";
        for ($i = 0,$j=1; $i < strlen($result);$i++,$j++) {
                $bytes[] = ord($result[$i]);
                $hex_arr .= bin2hex(chr($bytes[$i]))." ";
                if ( in_array($bytes[$i], array(0,10)) ) {
                    $bin_arr .= '.';
                }else {
                    $bin_arr .= chr($bytes[$i]);
                }

                if ($j == strlen($result)) {
                    echo "{$hex_arr}   {$bin_arr}\n";
                }

                if ($j % 8 == 0) {
                    echo "{$hex_arr}   {$bin_arr}\n";
                    $hex_arr="";
                    $bin_arr="";
                }
                //var_dump(bin2hex(chr($bytes[$i])).'   '.chr($bytes[$i]));
        }
        //var_dump($bytes);

        #client 
        $client_flags = 0x3f7cf;
        $database = "sakila";
        $user = "root";
        $password = "\0";
        $stage1 = sha1($password,1);
        $stage2 = sha1($stage1,1);
        $stage3 = sha1($scramble.$stage2,1);
        $n = strlen($stage1);
        /*$bytes = array();
        for ($i = 0; $i < $n; $i++) {
            $bytes[] = ord($stage3[$i]) ^ ord($stage1[$i]);
        }


        $token = '';
        foreach ($bytes as $ch) {
            $token .= chr($ch);
        }*/
        $token = $stage1 ^ $stage3;


        $req = $this->set_byte4($client_flags)
                .$this->set_byte4(1024*1024)
                .chr(0)
                .str_repeat("\0", 23)
                .$user."\0"
                .chr(strlen($token)).$token
                .$database."\0";
        var_dump($req);

        $pack_len = 4 + 4 + 1 + 23 + strlen($user) + 1 + strlen($token) + 1 + strlen($database) + 1;
        var_dump($pack_len);

        var_dump($this->set_byte3($pack_len).chr(1));

        $pack = substr_replace(pack("V", $pack_len), chr(1), 3, 1).$req;
        var_dump($pack);

        yield ngx_socket_send($this->socket, $pack, strlen($pack));

        yield ngx_socket_recv($this->socket, $result, 4);
        var_dump("receive: ".$result);

        $len = unpack('v',substr($result, 0,4));
        var_dump($len);
        $len = $len[1];
        yield ngx_socket_recv($this->socket, $data, $len);

        var_dump($data);

        $query = "select * from city limit 10;";
        #$query = "select sleep(1);";
        $req = chr(0x03).$query;
        $pack_len = strlen($query) + 1;
        $pack = $this->set_byte3($pack_len).chr(0).$req;

        yield ngx_socket_send($this->socket, $pack, strlen($pack));

        yield ngx_socket_recv($this->socket, $result);


        var_dump("receive: ".$result);
    }

    public function close() {
        yield ngx_socket_close($this->socket);
    }

}

?>