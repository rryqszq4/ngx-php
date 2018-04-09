<div align="left">
<a href="https://github.com/rryqszq4/ngx_php7"><img width="320" src="https://raw.githubusercontent.com/rryqszq4/ngx_php7/master/docs/ngx_php7_logo_v0.2.png"></a>
</div>

[![Build Status](https://travis-ci.org/rryqszq4/ngx_php7.svg?branch=master)](https://travis-ci.org/rryqszq4/ngx_php7)
[![license](https://img.shields.io/badge/license-BSD--2--Clause-blue.svg)](https://github.com/rryqszq4/ngx_php7/blob/master/LICENSE)
[![QQ group](https://img.shields.io/badge/QQ--group-558795330-26bcf5.svg)](https://github.com/rryqszq4/ngx_php7)

[ngx_php7](https://github.com/rryqszq4/ngx_php7) - Embedded php7 programming language for nginx-module.  
[ngx_php](https://github.com/rryqszq4/ngx_php) - Embedded php5 script language for nginx-module.  

Requirement
-----------
- PHP-7.0.* ~ PHP-7.2.*
- nginx-1.4.7 ~ nginx-1.10.3

Installation
-------
```sh
$ wget 'http://php.net/distributions/php-7.1.16.tar.gz'
$ tar xf php-7.1.16.tar.gz
$ cd php-7.1.16

$ ./configure --prefix=/path/to/php --enable-embed
$ make && make install

$ git clone https://github.com/rryqszq4/ngx_php7.git

$ wget 'http://nginx.org/download/nginx-1.10.3.tar.gz'
$ tar -zxvf nginx-1.10.3.tar.gz
$ cd nginx-1.10.3

$ export PHP_BIN=/path/to/php/bin
$ export PHP_INC=/path/to/php/include/php
$ export PHP_LIB=/path/to/php/lib

$ ./configure --user=www --group=www \
$             --prefix=/path/to/nginx \
$             --with-ld-opt="-Wl,-rpath,$PHP_LIB" \
$             --add-module=/path/to/ngx_php7/third_party/ngx_devel_kit \
$             --add-module=/path/to/ngx_php7
```

Synopsis
--------

```nginx
user www www;
worker_processes  4;

events {
    worker_connections  1024;
}

http {
    include       mime.types;
    default_type  application/octet-stream;

    keepalive_timeout  65;
    
    client_max_body_size 64k;   
    client_body_buffer_size 64k;

    php_ini_path /usr/local/php/etc/php.ini;

    server {
        listen       80;
        server_name  localhost;
        default_type 'application/json; charset=UTF-8';
    
        location /php {
            content_by_php '
                echo "hello ngx_php7";
            ';
        }

        location = /ngx_request {
            content_by_php '
                echo ngx_request::document_uri();
            ';
        }

        location = /ngx_get {
            content_by_php '
                echo "ngx::query_args()\n";
                var_dump(ngx::query_args());
            ';
        }

        location = /ngx_post {
            content_by_php '
                echo "ngx::post_args()\n";
                var_dump(ngx::post_args());
            ';
        }

        location = /ngx_sleep {
            content_by_php '
                echo "ngx_sleep start\n";
                yield ngx::sleep(1);
                echo "ngx_sleep end\n";
            ';
        }

        location = /ngx_socket {
            default_type 'application/json;charset=UTF-8';
            content_by_php '
                yield ngx_socket::connect("hq.sinajs.cn", 80);
                yield ngx_socket::send("GET /list=s_sh000001 HTTP/1.0\r\nHost: hq.sinajs.cn\r\nConnection: close\r\n\r\n");
                yield $ret = ngx_socket::recv(1024);
                yield ngx_socket::close();
                
                var_dump($ret);
            ';
        }

    }
}
```

Copyright and License
---------------------
```
Copyright (c) 2016-2018, rryqszq4 <rryqszq@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```