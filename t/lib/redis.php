<?php
/**
 *  Copyright(c) 2017-2019 rryqszq4 rryqszq@gmail.com
 */

namespace php\ngx;

class Redis {

    const VERSION = "0.1.0";

    protected $socket = null;

    protected $commands = array(
        "get",
        "set",
        "mget",
        "mset",
        "del",
        "incr",
        "decr",
        "llen",
        "lindex",
        "lpop",
        "lpush",
        "lrange",
        "linsert",
        "hexists",
        "hget",
        "hset",
        "hmget",
        "hdel",
        "smembers",
        "sismember",
        "sadd",
        "srem",
        "sdiff",
        "sinter",
        "sunion",
        "zrange",
        "zrangebyscore",
        "zrank",
        "zadd",
        "zrem",
        "zincrby",
        "auth",
        "eval",
        "expire",
        "script",
        "sort"
    );

    public function __construct() {
        $this->socket = ngx_socket_create();
    }

    public function __destruct() {
        if ($this->socket) {
            ngx_socket_clear($this->socket);
        }
    }

    public function connect($host="", $port="") {
        yield ngx_socket_connect($this->socket, $host, $port);
    }

    public function close() {
        yield ngx_socket_close($this->socket);

        unset($this->socket);
    }

    public function clear() {
        ngx_socket_clear($this->socket);

        unset($this->socket);
    }

    private function write_data(...$args) {
        $payload = '';
        #var_dump($args);
        foreach ($args as $arg) {
            $payload .= '$'.strlen($arg)."\r\n{$arg}\r\n";
        }
        $payload = '*'.count($args)."\r\n{$payload}";

        #var_dump($payload);

        yield ngx_socket_send($this->socket, $payload, strlen($payload));
    }

    private function read_data() {
        do {
            $buf = '';
            yield ngx_socket_recv($this->socket, $buf);
            $data .= $buf;
            #var_dump($buf);
            
        } while (strlen($buf) >= 1024);
        #var_dump($data);

        return $this->data_parse($data);
    }

    private function data_parse($data) {
        $prefix = ord($data[0]);

        if ($prefix == 36) { // '$' 
            $size = intval(substr($data, 1));
            if ($size < 0) {
                return null;
            }
            
            $token = strtok($data, "\r\n");
            $token = strtok("\r\n");
            return $token;

        }else if ($prefix == 43) { // '+'
            return trim(substr($data, 1));
        }else if ($prefix == 42) { // '*'
            $size = intval(substr($data, 1));
            if ($size < 0) {
                return null;
            }

            $output = array();
            $data = substr($data, strpos($data, "\r\n")+2);
            for($i = 0; $i < $size; $i++) {
                $res = $this->data_parse($data);
                #var_dump($data);
                if (!empty($res)) {
                    $output[$i] = $res;
                }else {
                    $output[$i] = null;
                }
                $data = substr($data, strpos($data, "\r\n")+2);
                $data = substr($data, strpos($data, "\r\n")+2);
            }
            return $output;

        }else if ($prefix == 58) { // ':'
            return intval(substr($data, 1));
        }else if ($prefix == 45) { // '-'
            return false;
        }else {
            return null;
        }
    }

    public function __call($name, $params) {
        if (!in_array($name, $this->commands)) {
            return false;
        }

        array_unshift($params, $name);
        #var_dump($params);
        yield from $this->write_data(...$params);

        $result = ( yield from $this->read_data() );

        return $result;
    }

}

?>