<?php
/**
 *  Copyright(c) 2017-2019 rryqszq4 rryqszq@gmail.com
 */

namespace php\ngx;

// mysql protocol
// https://dev.mysql.com/doc/internals/en/client-server-protocol.html
class mysql
{

    const VERSION = '0.4.0';

    private $socket = null;

    private $headerNum    = 0;
    private $headerCurr   = 0;
    private $resultState  = 0;
    private $resultFields = null;
    private $resultRows   = null;

    public static $charsets = array(
            '_default'  => 0,
            'big5'      => 1,
            'dec8'      => 3,
            'cp850'     => 4,
            'hp8'       => 6,
            'koi8r'     => 7,
            'latin1'    => 8,
            'latin2'    => 9,
            'swe7'      => 10,
            'ascii'     => 11,
            'ujis'      => 12,
            'sjis'      => 13,
            'hebrew'    => 16,
            'tis620'    => 18,
            'euckr'     => 19,
            'koi8u'     => 22,
            'gb2312'    => 24,
            'greek'     => 25,
            'cp1250'    => 26,
            'gbk'       => 28,
            'latin5'    => 30,
            'armscii8'  => 32,
            'utf8'      => 33,
            'ucs2'      => 35,
            'cp866'     => 36,
            'keybcs2'   => 37,
            'macce'     => 38,
            'macroman'  => 39,
            'cp852'     => 40,
            'latin7'    => 41,
            'utf8mb4'   => 45,
            'cp1251'    => 51,
            'utf16'     => 54,
            'utf16le'   => 56,
            'cp1256'    => 57,
            'cp1257'    => 59,
            'utf32'     => 60,
            'binary'    => 63,
            'geostd8'   => 92,
            'cp932'     => 95,
            'eucjpms'   => 97,
            'gb18030'   => 248
    );

    public function __construct()
    {
        $this->socket = \ngx_socket_create();
    }

    public function __destruct()
    {
        if ($this->socket) {
            \ngx_socket_clear($this->socket);
            \ngx_socket_destroy($this->socket);
        }
    }

    private function reset()
    {
        $this->headerNum    = 0;
        $this->headerCurr   = 0;
        $this->resultState  = 0;
        $this->resultFields = new \ArrayObject();
        $this->resultRows   = new \ArrayObject();
    }

    private function print_bin($result)
    {
        $hex_arr = '';
        $bin_arr = '';
        $len     = strlen($result);
        for ($i = 0, $j = 1; $i < $len; $i++, $j++) {
            $bytes[] = \ord($result[$i]);
            if ($j % 8 === 0) {
                $hex_arr .= \bin2hex(\chr($bytes[$i])).'   ';
            } else {
                $hex_arr .= \bin2hex(\chr($bytes[$i])).' ';
            }
            if (\in_array($bytes[$i], [0, 10])) {
                $bin_arr .= '.';
            } else {
                $bin_arr .= \chr($bytes[$i]);
            }

            if ($j === $len) {
                echo "{$hex_arr}   {$bin_arr}\n";
            }

            if ($j % 16 === 0) {
                echo "{$hex_arr}   {$bin_arr}\n";
                $hex_arr = '';
                $bin_arr = '';
            }
        }
    }

    private function length_encoded_integer($data, &$start)
    {
        $first = \ord(\substr($data, $start, 1));

        if ($first <= 250) {
            $start += 1;

            return $first;
        }
        if ($first === 251) {
            $start += 1;

            return;
        }
        if ($first === 252) {
            $ret = \unpack('v', \substr($data, $start, 2));
            $start += 2;

            return $ret[1];
        }
        if ($first === 253) {
            $ret = \unpack('V', \substr($data, $start, 3)."\0");
            $start += 3;

            return $ret[1];
        }

        $ret = \unpack('P', \substr($data, $start, 8));
        $start += 8;

        return $ret[1];
    }

    private function write_packet($data, $len, $chr = 1)
    {
        $pack = \substr(\pack('V', $len), 0, 3).\chr($chr).$data;

        yield \ngx_socket_send($this->socket, $pack, \strlen($pack));
    }

    private function read_packet($sync=0)
    {
        $data = '';
        if ($sync == 0) {
            do {
                yield \ngx_socket_recv($this->socket, $data, 4);
            }while(empty($data));
        }else {
            \ngx_socket_recvsync($this->socket, $data, 4);
        }
        
        $field_count = \unpack('v', \substr($data, 0, 3))[1];

        \ngx_socket_recvsync($this->socket, $data, $field_count);

        if ($field_count !== 1) {
            $field_count = \ord(\substr($data, 0, 1));
        }

        if ($field_count === 0x00) {
            // Ok packet
            $this->ok_packet($data);
        } else if ($field_count === 0xff) {
            // Error packet
            $this->error_packet($data);
        } else if ($field_count === 0xfe) {
            // EOF packet
            if ($this->resultState === 2) {
                yield from $this->read_packet(1);
            }
            if ($this->resultState === 201) {
                $this->resultState = 0;
            }
        } else {
            // Data packet
            if ($field_count === 1 && $this->resultState === 0) {
                // Header set
                $this->headerNum = \ord($data);
                // headerNum
                $this->resultState = 1;
                yield from $this->read_packet(1);
            } else if ($this->resultState === 1) {
                // Field data packet
                if ($this->headerCurr < $this->headerNum - 1) {
                    $this->field_data_packet($data);
                    ++$this->headerCurr;
                    yield from $this->read_packet(1);
                } else {
                    $this->field_data_packet($data);
                    ++$this->headerCurr;
                    $this->resultState = 2;
                    yield from $this->read_packet(1);
                }
            } else if ($this->resultState === 2 || $this->resultState === 201) {
                // Row data packet
                $this->resultState = 201;
                $this->row_data_packet($data);
                yield from $this->read_packet(1);
            } else {
                return $data;
            }
        }
    }

    private function handshake_packet()
    {
        // Packet length
        $data = (yield from $this->read_packet());

        // Protocol version
        // Server version
        $protocol_ver = \ord(\substr($data, 0, 1));

        $pos = \strpos($data, "\0", 2) + 1;

        // Thread id
        $thread_id = \unpack('V', \substr($data, $pos, 4));
        $thread_id = $thread_id[1];

        $pos += 4;

        // Scramble
        $scramble = \substr($data, $pos, 8);

        // Server capabilities
        $capabilities = \unpack('v', \substr($data, $pos + 9, 2));
        $capabilities = $capabilities[1];

        // Server lang
        $server_lang = \ord(\substr($data, $pos + 9 + 2, 1));

        // Server status
        $server_status = \unpack('v', \substr($data, $pos + 9 + 2 + 1, 2));
        $server_status = $server_status[1];

        // More capabilities
        $more_capabilites = \unpack('v', \substr($data, $pos + 9 + 2 + 1 + 2, 2));
        $more_capabilites = $more_capabilites[1];

        // Server capabilities
        $capabilities = $capabilities | ($more_capabilites << 16);

        $scramble_2 = \substr($data, $pos + 9 + 2 + 1 + 2 + 2 + 1 + 10, 21 - 8 - 1);

        return $scramble.$scramble_2;
    }

    private function auth_packet($scramble, $user, $password, $database, $charset)
    {
        // Client
        $client_flags = 0x3f7cf;

        $stage1 = \sha1($password, 1);
        $stage2 = \sha1($stage1, 1);
        $stage3 = \sha1($scramble.$stage2, 1);
        $n      = \strlen($stage1);

        $token = $stage1 ^ $stage3;

        $req = \pack('VV', $client_flags, 1048576)
            .\chr($charset)
            .\str_repeat("\0", 23)
            .$user."\0"
            .\chr(\strlen($token))
            .$token
            .$database."\0";

        $pack_len = 4 + 4 + 1 + 23+\strlen($user) + 1+\strlen($token) + 1+\strlen($database) + 1;

        yield from $this->write_packet($req, $pack_len);

        yield from $this->read_packet();
    }

    private function ok_packet($data)
    {
        $start    = 1;

        // Rows length
        $rows_len = $this->length_encoded_integer($data, $start);

        // Insert id
        $insert_id = $this->length_encoded_integer($data, $start);

        // Server status
        $server_status = \unpack('v', \substr($data, $start, 2))[1];
        $start += 2;

        // Warning count
        $warning_count = \unpack('v', \substr($data, $start, 2))[1];
        $start += 2;

        // Message
        $message = \substr($data, $start);
        
    }

    private function error_packet($data)
    {
        $start = 1;
        
        // Errno
        $errno = \unpack('v', \substr($data, $start, 2))[1];
        $start += 2;
        
        // SQL state
        $sql_state = \substr($data, $start, 6);
        $start += 6;

        // Message
        $message = \substr($data, $start);
        
    }

    private function field_data_packet($result)
    {
        $start            = 0;
        $field            = new \ArrayObject();

        // Catalog
        $field['catalog'] = $this->parse_field_data($result, $start);
        
        // DB
        $field['db'] = $this->parse_field_data($result, $start);
        
        // Table
        $field['table'] = $this->parse_field_data($result, $start);
        
        // Original table
        $field['ori_table'] = $this->parse_field_data($result, $start);
        
        // Column
        $field['column'] = $this->parse_field_data($result, $start);
        
        // Original column
        $field['ori_column'] = $this->parse_field_data($result, $start);
        
        // 0xC0
        $start += 1;
        
        // Charset
        $field['charset'] = \unpack('v', \substr($result, $start, 2))[1];
        $start += 2;

        // Length
        $field['length'] = \unpack('V', \substr($result, $start, 4))[1];
        $start += 4;

        // Type
        $field['type'] = \ord(\substr($result, $start, 1));
        $start += 1;

        // Flag
        $field['flag'] = \unpack('v', \substr($result, $start, 2))[1];
        $start += 2;

        // Decimals
        $field['decimals'] = \unpack('v', \substr($result, $start, 2))[1];

        $this->resultFields->append($field);
    }

    private function parse_field_data($result, &$start)
    {
        $len = \ord(\substr($result, $start, 1));

        $field = \substr($result, $start + 1, $len);
        $start = $start + 1 + $len;

        return $field;
    }

    private function row_data_packet($data)
    {
        $start = 0;
        $row   = new \ArrayObject();
        foreach ($this->resultFields as $field) {
            $len = $this->length_encoded_integer($data, $start);

            $value = \substr($data, $start, $len);

            $start += $len;
            $row[$field['column']] = $value;
        }
        
        // Result rows
        $this->resultRows->append($row);
    }

    public function connect($host = '', $port = '', $user = '', $password = '', $database = '', $charset=0)
    {

        yield \ngx_socket_connect($this->socket, $host, $port);

        if ( ! \ngx_socket_iskeepalive()) {
            $scramble = (yield from $this->handshake_packet());

            yield from $this->auth_packet($scramble, $user, $password, $database, $charset);
        }
    }

    public function query($sql)
    {
        $this->reset();

        $req      = \chr(0x03).$sql;
        $pack_len = \strlen($sql) + 1;

        yield from $this->write_packet($req, $pack_len, 0);

        // Result set packet
        yield from $this->read_packet();

        return $this->resultRows;
    }

    public function close()
    {
        yield \ngx_socket_close($this->socket);

        $this->reset();
    }

    public function clear()
    {
        \ngx_socket_clear($this->socket);
        
        $this->reset();
    }
}

class ConnectionObjectPool {

    const NULL_POOL = 0;
    const MYSQL_POOL = 1;

    private $freeQueue;

    private $maxQueue;

    public static $instance = null;

    public function __construct() 
    {
        $this->CreateQueue();
        $this->maxQueue = 4096;
    }

    public function CreateQueue() {
        $this->freeQueue = new \SplQueue();
    }

    public static function factory($type=0) 
    {
        if ($type == self::MYSQL_POOL) {
            if (!self::$instance) {
                self::$instance = new self();
            }
            return self::$instance;
        }else {
            return null;
        }
    }

    public function get() 
    {
        $connectionObject = null;

        if ( $this->freeQueue->isEmpty() ) {
            $connectionObject = new mysql();
        }else {
            $connectionObject = $this->freeQueue->dequeue();
        }

        return $connectionObject;
    }

    public function put($connectionObject) 
    {
        $count = $this->freeQueue->count();

        if ( $count < $this->maxQueue ) {
            $connectionObject->clear();
            $this->freeQueue->enqueue($connectionObject);
        }else {
            ngx_log_error(NGX_LOG_ERR, "ConnectionObjectPool is full");
        }
    }
}


