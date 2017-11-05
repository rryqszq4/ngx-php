ngx_php7
=================

[![Build Status](https://travis-ci.org/rryqszq4/ngx_php7.svg?branch=master)](https://travis-ci.org/rryqszq4/ngx_php7)
[![license](https://img.shields.io/badge/license-BSD--2--Clause-blue.svg)](https://github.com/rryqszq4/ngx_php/blob/master/LICENSE)

[ngx_php7](https://github.com/rryqszq4/ngx_php7) - Embedded php7 script language for nginx-module.  
[ngx_php](https://github.com/rryqszq4/ngx_php) for php5 nginx-module.  
QQ group：558795330

Requirement
-----------
- PHP 7.0.*  
PHP 7.1.*  
- nginx-1.4.7  
nginx-1.6.3  
nginx-1.8.1  
nginx-1.9.15

Installation
-------
```sh
$ wget 'http://php.net/distributions/php-7.0.23.tar.gz'
$ tar xf php-7.0.23.tar.gz
$ cd php-7.0.23

$ ./configure --prefix=/path/to/php --enable-embed
$ make && make install

$ git clone https://github.com/rryqszq4/ngx_php7.git

$ wget 'http://nginx.org/download/nginx-1.6.3.tar.gz'
$ tar -zxvf nginx-1.6.3.tar.gz
$ cd nginx-1.6.3

$ export PHP_BIN=/path/to/php/bin
$ export PHP_INC=/path/to/php/include/php
$ export PHP_LIB=/path/to/php/lib

$ ./configure --user=www --group=www \
$             --prefix=/path/to/nginx \
$             --with-ld-opt="-Wl,-rpath,$PHP_LIB" \
$             --add-module=/path/to/ngx_php7/dev/ngx_devel_kit \
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
    
    client_max_body_size 10m;   
    client_body_buffer_size 4096k;

    php_ini_path /usr/local/php/etc/php.ini;

    server {
        listen       80;
        server_name  localhost;
    
        location /php {
            content_by_php '
                echo "hello ngx_php7";
            ';
        }
    }
}
```

Copyright and License
---------------------
Copyright (c) 2016, rryqszq4 <rryqszq@gmail.com>
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