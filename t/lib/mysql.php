<?php
/**
 *  Copyright(c) 2017-2018 rryqszq4 rryqszq@gmail.com
 */

namespace php\ngx;

// mysql protocol
// https://dev.mysql.com/doc/internals/en/client-server-protocol.html
class mysql
{

    const VERSION = '0.1.0';

    private $socket = null;

    private $headerNum    = 0;
    private $headerCurr   = 0;
    private $resultState  = 0;
    private $resultFields = [];
    private $resultRows   = [];

    public function __construct()
    {
        $this->socket = \ngx_socket_create();
    }

    public function __destruct()
    {
        if ($this->socket) {
            \ngx_socket_clear($this->socket);
        }
    }

    private function reset()
    {
        $this->headerNum    = 0;
        $this->headerCurr   = 0;
        $this->resultState  = 0;
        $this->resultFields = [];
        $this->resultRows   = [];
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
            //var_dump(bin2hex(chr($bytes[$i])).'   '.chr($bytes[$i]));
        }
        //var_dump($bytes);
    }

    private function length_encoded_integer($data, &$start)
    {
        $first = \ord(\substr($data, $start, 1));
        #var_dump($first);
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

    private function read_packet()
    {
        #var_dump("read_packet");
        //$i = 0;
        //do {
        //   if ($i > 0) {usleep(1000);}
        //   if ($i > 1000) {break;}
        $data = '';
        yield \ngx_socket_recv($this->socket, $data, 4);
        //    $i++;
        //} while (empty($data));
        #$this->print_bin($data);
        $field_count = \unpack('v', \substr($data, 0, 3))[1];
        #var_dump("field_count: ".$field_count);
        //$data = '';
        yield \ngx_socket_recv($this->socket, $data, $field_count);
        #$this->print_bin($data);
        if ($field_count !== 1) {
            $field_count = \ord(\substr($data, 0, 1));
        }
        #var_dump("field_count: ".$field_count."   ".$this->resultState);
        if ($field_count === 0x00) {
            #var_dump("OK packet");
            $this->ok_packet($data);
        } else if ($field_count === 0xff) {
            #var_dump("Error packet");
            $this->error_packet($data);
        } else if ($field_count === 0xfe) {
            #var_dump("EOF packet ".$this->resultState);
            if ($this->resultState === 2) {
                yield from $this->read_packet();
            }
            if ($this->resultState === 201) {
                $this->resultState = 0;
            }
        } else {
            #var_dump("Data packet");
            if ($field_count === 1 && $this->resultState === 0) {
                #var_dump("header set");
                $this->headerNum = \ord($data);
                #var_dump($this->headerNum);
                $this->resultState = 1;
                yield from $this->read_packet();
            } else if ($this->resultState === 1) {
                #var_dump('fields '.$this->headerCurr."   ".$this->headerNum);
                if ($this->headerCurr < $this->headerNum - 1) {
                    $this->field_data_packet($data);
                    ++$this->headerCurr;
                    yield from $this->read_packet();
                } else {
                    $this->field_data_packet($data);
                    ++$this->headerCurr;
                    $this->resultState = 2;
                    yield from $this->read_packet();
                }
            } else if ($this->resultState === 2 || $this->resultState === 201) {
                $this->resultState = 201;
                #var_dump("rows");
                #var_dump($data);
                $this->row_data_packet($data);
                yield from $this->read_packet();
            } else {
                return $data;
            }
        }
    }

    private function handshake_packet()
    {
        $data = (yield from $this->read_packet());

        #var_dump("packet length: ".$len);

        $protocol_ver = \ord(\substr($data, 0, 1));

        #var_dump("protocol version: ".$protocol_ver);

        #var_dump(strpos($data, "\0", 2));

        #var_dump("server version: ".substr($data, 1, strpos($data, "\0", 2)-1));

        $pos = \strpos($data, "\0", 2) + 1;

        $thread_id = \unpack('V', \substr($data, $pos, 4));
        $thread_id = $thread_id[1];
        #var_dump("thread id: ".$thread_id);

        $pos += 4;
        #var_dump($pos);

        $scramble = \substr($data, $pos, 8);

        #var_dump($scramble);

        $capabilities = \unpack('v', \substr($data, $pos + 9, 2));
        $capabilities = $capabilities[1];
        #var_dump("server capabilities: ".$capabilities);

        $server_lang = \ord(\substr($data, $pos + 9 + 2, 1));
        #var_dump("server lang: ".$server_lang);

        $server_status = \unpack('v', \substr($data, $pos + 9 + 2 + 1, 2));
        $server_status = $server_status[1];
        #var_dump("server status: ".$server_status);

        $more_capabilites = \unpack('v', \substr($data, $pos + 9 + 2 + 1 + 2, 2));
        $more_capabilites = $more_capabilites[1];
        #var_dump("more capabilities: ".$more_capabilites);

        $capabilities = $capabilities | ($more_capabilites << 16);
        #var_dump("server capabilities: ".$capabilities);

        $scramble_2 = \substr($data, $pos + 9 + 2 + 1 + 2 + 2 + 1 + 10, 21 - 8 - 1);
        #var_dump($scramble_2);

        #var_dump("scramble: ".$scramble);
        #var_dump(strlen($scramble));

        return $scramble.$scramble_2;
    }

    private function auth_packet($scramble, $user, $password, $database)
    {
        // client
        $client_flags = 0x3f7cf;

        $stage1 = \sha1($password, 1);
        $stage2 = \sha1($stage1, 1);
        $stage3 = \sha1($scramble.$stage2, 1);
        $n      = \strlen($stage1);

        $token = $stage1 ^ $stage3;

        $req = \pack('VV', $client_flags, 1048576)
            .\chr(0)
            .\str_repeat("\0", 23)
            .$user."\0"
            .\chr(\strlen($token))
            .$token
            .$database."\0";
        #var_dump($req);

        $pack_len = 4 + 4 + 1 + 23+\strlen($user) + 1+\strlen($token) + 1+\strlen($database) + 1;
        #var_dump($pack_len);

        yield from $this->write_packet($req, $pack_len);

        yield from $this->read_packet();
    }

    private function ok_packet($data)
    {
        $start    = 1;
        $rows_len = $this->length_encoded_integer($data, $start);
        #var_dump($rows_len);
        $insert_id = $this->length_encoded_integer($data, $start);
        #var_dump($insert_id);
        $server_status = \unpack('v', \substr($data, $start, 2))[1];
        $start += 2;
        #var_dump($server_status);
        $warning_count = \unpack('v', \substr($data, $start, 2))[1];
        $start += 2;
        #var_dump($warning_count);
        $message = \substr($data, $start);
        #var_dump($message);
    }

    private function error_packet($data)
    {
        $start = 1;
        $errno = \unpack('v', \substr($data, $start, 2))[1];
        $start += 2;
        #var_dump($errno);
        $sql_state = \substr($data, $start, 6);
        $start += 6;
        #var_dump($sql_state);
        $message = \substr($data, $start);
        #var_dump($message);
    }

    private function field_data_packet($result)
    {
        $start            = 0;
        $field            = [];
        $field['catalog'] = $catalog = $this->parse_field_data($result, $start);
        #var_dump($catalog);
        $field['db'] = $db = $this->parse_field_data($result, $start);
        #var_dump($db);
        $field['table'] = $table = $this->parse_field_data($result, $start);
        #var_dump($table);
        $field['ori_table'] = $ori_table = $this->parse_field_data($result, $start);
        #var_dump($ori_table);
        $field['column'] = $column = $this->parse_field_data($result, $start);
        #var_dump($column);
        $field['ori_column'] = $ori_column = $this->parse_field_data($result, $start);
        #var_dump($ori_column);

        #$this->print_bin(substr($result, $start));

        #$this->print_bin(substr($result, $start, 1));
        // 0xC0
        $start += 1;
        #$this->print_bin(substr($result, $start, 2));
        $field['charset'] = $charset = \unpack('v', \substr($result, $start, 2))[1];
        #var_dump($charset);
        $start += 2;
        $field['length'] = $length = \unpack('V', \substr($result, $start, 4))[1];
        #var_dump($length);
        $start += 4;
        $field['type'] = $type = \ord(\substr($result, $start, 1));
        #var_dump($type);
        $start += 1;
        $field['flag'] = $flag = \unpack('v', \substr($result, $start, 2))[1];
        #var_dump($flag);
        $start += 2;
        $field['decimals'] = $decimals = \unpack('v', \substr($result, $start, 2))[1];
        #var_dump($decimals);

        $this->resultFields[] = $field;
    }

    private function parse_field_data($result, &$start)
    {
        $len = \ord(\substr($result, $start, 1));
        #var_dump($len);
        $field = \substr($result, $start + 1, $len);
        $start = $start + 1 + $len;

        return $field;
    }

    private function row_data_packet($data)
    {
        $start = 0;
        $row   = [];
        foreach ($this->resultFields as $field) {
            $len = $this->length_encoded_integer($data, $start);
            #var_dump($len);

            $value = \substr($data, $start, $len);
            #var_dump($value);
            $start += $len;
            $row[$field['column']] = $value;
        }
        #var_dump($row);
        $this->resultRows[] = $row;
    }

    public function connect($host = '', $port = '', $user = '', $password = '', $database = '')
    {
        yield \ngx_socket_connect($this->socket, $host, $port);

        if ( ! \ngx_socket_iskeepalive()) {
            $scramble = (yield from $this->handshake_packet());

            yield from $this->auth_packet($scramble, $user, $password, $database);
        }
    }

    public function query($sql)
    {
        $this->reset();

        $req      = \chr(0x03).$sql;
        $pack_len = \strlen($sql) + 1;

        yield from $this->write_packet($req, $pack_len, 0);

        // result set packet
        yield from $this->read_packet();

        return $this->resultRows;
    }

    public function close()
    {
        yield \ngx_socket_close($this->socket);

        unset($this->socket);
    }

    public function clear()
    {
        \ngx_socket_clear($this->socket);

        unset($this->socket);
    }
}
