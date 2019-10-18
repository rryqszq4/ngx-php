<?php
/**
 *  Copyright(c) 2017-2019 rryqszq4 rryqszq@gmail.com
 */

namespace php\ngx;

class Redis {

    /**
     * @var string
     */
    const VERSION = '0.1.0';
    
    /**
     * Show debug info
     *
     * @var bool
     */
    public $isDebug = false;

    /**
     * @var resource
     */
    protected $socket = null;

    /**
     * Avaiable redis commands.
     *
     * @var string[]
     */
    protected static $commands = [
        'get',
        'set',
        'mget',
        'mset',
        'del',
        'incr',
        'decr',
        'llen',
        'lindex',
        'lpop',
        'lpush',
        'lrange',
        'linsert',
        'hexists',
        'hget',
        'hset',
        'hmget',
        'hdel',
        'smembers',
        'sismember',
        'sadd',
        'srem',
        'sdiff',
        'sinter',
        'sunion',
        'zrange',
        'zrangebyscore',
        'zrank',
        'zadd',
        'zrem',
        'zincrby',
        'auth',
        'eval',
        'expire',
        'script',
        'sort'
    ];

    public function __construct() {
        $this->socket = \ngx_socket_create();
    }

    public function __destruct() {
        if ($this->socket) {
            \ngx_socket_clear($this->socket);
        }
    }

    public function connect($host="", $port="") {
        yield \ngx_socket_connect($this->socket, $host, $port);
    }

    public function close() {
        yield \ngx_socket_close($this->socket);

        unset($this->socket);
    }

    public function clear() {
        \ngx_socket_clear($this->socket);

        unset($this->socket);
    }

    private function write_data(...$args) {
        $payload = '';

        $this->debug($args);

        foreach ($args as $arg) {
            $payload .= '$'.\strlen($arg)."\r\n{$arg}\r\n";
        }
        $payload = '*'.\count($args)."\r\n{$payload}";

        $this->debug($payload);

        yield \ngx_socket_send($this->socket, $payload, \strlen($payload));
    }

    private function read_data() {
        do {
            $buf = '';
            yield \ngx_socket_recv($this->socket, $buf);
            $data .= $buf;

            $this->debug($buf);

        } while (\strlen($buf) >= 1024);

        $this->debug($data);

        return $this->data_parse($data);
    }

    private function data_parse($data) {

        switch(\ord($data[0])) {
            case 36: // '$'
                $size = \intval(\substr($data, 1));
                if ($size < 0) {
                    return null;
                }
                
                $token = \strtok($data, "\r\n");
                return \strtok("\r\n");

            case 43: // '+'
                return \trim(\substr($data, 1));
            
            case 42: // '*'
                $size = \intval(\substr($data, 1));
                if ($size < 0) {
                    return null;
                }

                $output = [];
                $data = \substr($data, \strpos($data, "\r\n")+2);
                for($i = 0; $i < $size; $i++) {
                    $res = $this->data_parse($data);
                    
                    $this->debug($data);

                    if (!empty($res)) {
                        $output[$i] = $res;
                    }else {
                        $output[$i] = null;
                    }
                    $data = \substr($data, \strpos($data, "\r\n")+2);
                    $data = \substr($data, \strpos($data, "\r\n")+2);
                }
                return $output;
            
            case 58: // ':'
                return \intval(\substr($data, 1));
            
            case 45: // '-'
                return false;
            
            default:
                return null;
        }
    }

    public function __call($name, $params) {
        if (!\in_array($name, self::$commands)) {
            return false;
        }

        \array_unshift($params, $name);

        $this->debug($params);

        yield from $this->write_data(...$params);

        return ( yield from $this->read_data() );
    }

    protected function debug($data) {
        if ($this->isDebug) {
            \var_dump($data);
        }
    }
}
