ngx_php7
========
[![Build Status](https://travis-ci.org/rryqszq4/ngx_php7.svg?branch=master)](https://travis-ci.org/rryqszq4/ngx_php7)
[![GitHub release](https://img.shields.io/github/release/rryqszq4/ngx_php7.svg)](https://github.com/rryqszq4/ngx_php7/releases/latest)
[![license](https://img.shields.io/badge/license-BSD--2--Clause-blue.svg)](https://github.com/rryqszq4/ngx_php7/blob/master/LICENSE)
[![QQ group](https://img.shields.io/badge/QQ--group-558795330-26bcf5.svg)](https://github.com/rryqszq4/ngx_php7)

![](https://raw.githubusercontent.com/rryqszq4/ngx_php7/master/docs/hello_world_performance.png)

ngx_php7 is an extension module of high-performance web server nginx, which implements embedded php7 script to process nginx location and variables.  

ngx_php7 draws on the design of [ngx_lua](https://github.com/openresty/lua-nginx-module) and is committed to providing non-blocking web services with significant performance advantages over php-cgi, mod_php, php-fpm and hhvm.  

ngx_php7 doesn't want to replace anything, just want to provide a solution.  

There is a legacy version of [ngx_php5](https://github.com/rryqszq4/ngx_php/tree/ngx_php5), which records some of my past code practices and is also valuable.   

What's different with official php
----------------------------------
* Global variable is unsafe in per request
* Static variable of a class is unsafe in per request
* Do not design singleton mode
* The native IO function works fine, but it slows down nginx

Requirement
-----------
- PHP-7.0.* ~ PHP-7.3.*
- nginx-1.4.7 ~ nginx-1.14.2 and mainline 1.15.7

Installation
------------
```sh
$ wget 'http://php.net/distributions/php-7.2.14.tar.gz'
$ tar xf php-7.2.14.tar.gz
$ cd php-7.2.14

$ ./configure --prefix=/path/to/php --enable-embed
$ make && make install

$ git clone https://github.com/rryqszq4/ngx_php7.git

$ wget 'http://nginx.org/download/nginx-1.12.2.tar.gz'
$ tar -zxvf nginx-1.12.2.tar.gz
$ cd nginx-1.12.2

$ export PHP_CONFIG=/path/to/php/bin/php-config
$ export PHP_BIN=/path/to/php/bin
$ export PHP_INC=/path/to/php/include/php
$ export PHP_LIB=/path/to/php/lib

$ ./configure --user=www --group=www \
$             --prefix=/path/to/nginx \
$             --with-ld-opt="-Wl,-rpath,$PHP_LIB" \
$             --add-module=/path/to/ngx_php7/third_party/ngx_devel_kit \
$             --add-module=/path/to/ngx_php7
$ make && make install
```

Docker
------

```sh
$ docker build -t nginx-php7 .
$ : "app.conf: Create nginx config"
$ docker run -p 80:80 -v $PWD/app.conf:/etc/nginx/conf.d/default.conf nginx-php7
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

        # curl /ngx_get?a=1&b=2
        location = /ngx_get {
            content_by_php '
                echo "ngx::query_args()\n";
                var_dump(ngx::query_args());
            ';
        }

        # curl -d 'a=1&b=2' /ngx_post
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
                yield ngx_socket::send("GET /list=s_sh000001 HTTP/1.0\r\n
                                        Host: hq.sinajs.cn\r\nConnection: close\r\n\r\n");
                yield $ret = ngx_socket::recv(1024);
                yield ngx_socket::close();
                
                var_dump($ret);
            ';
        }

        location = /ngx_socket2 {
            default_type 'application/json;charset=UTF-8';
            content_by_php '
                $fd = ngx_socket_create();
                var_dump($fd);
                yield ngx_socket_connect($fd, "hq.sinajs.cn", 80);
                $send_buf = "GET /list=s_sh000001 HTTP/1.0\r\n
                                            Host: hq.sinajs.cn\r\nConnection: close\r\n\r\n";
                yield ngx_socket_send($fd, $send_buf, strlen($send_buf));
                $recv_buf = "";
                yield ngx_socket_recv($fd, $recv_buf);
                var_dump($recv_buf);
                yield ngx_socket_close($fd);
            ';
        }

        location = /ngx_var {
            set $a 1234567890;
            content_by_php '
                $a = ngx_var::get("a");
                var_dump($a);
            ';
        }
        
        # run a php file
        location = /php {
            content_by_php '
                include "name_of_php_file.php";
            ';
        }
        
        # run any php file in root
        location = / {
            content_by_php '
                include ngx_var::get("uri");
            ';
        }

    }
}
```

Test
----
Using the perl of [Test::Nginx](https://github.com/openresty/test-nginx) module to testing, searching and finding out problem in ngx_php7.
```sh
ngx_php7 test ...
nginx version: nginx/1.10.3
built by gcc 4.8.4 (Ubuntu 4.8.4-2ubuntu1~14.04.3) 
configure arguments: --prefix=/home/travis/build/rryqszq4/ngx_php7/build/nginx --with-ld-opt=-Wl,-rpath,/home/travis/build/rryqszq4/ngx_php7/build/php/lib --add-module=../../../ngx_php7/third_party/ngx_devel_kit --add-module=../../../ngx_php7
t/001-hello.t ........... ok
t/002-ini.t ............. ok
t/004-ngx_request.t ..... ok
t/005-ngx_log.t ......... ok
t/006-ngx_sleep.t ....... ok
t/007-ngx_socket.t ...... ok
t/008-ngx_exit.t ........ ok
t/009-ngx_query_args.t .. ok
t/010-ngx_post_args.t ... ok
t/011-ngx_constants.t ... ok
t/012-function.t ........ ok
t/013-class.t ........... ok
t/014-ngx_var.t ......... ok
All tests successful.
Files=13, Tests=28,  5 wallclock secs ( 0.05 usr  0.02 sys +  1.26 cusr  0.22 csys =  1.55 CPU)
Result: PASS
```

Directives
----------
* [php_ini_path](#php_ini_path)
* [init_worker_by_php](#init_worker_by_php)
* [rewrite_by_php](#rewrite_by_php)
* [access_by_php](#access_by_php)
* [content_by_php](#content_by_php)
* [log_by_php](#log_by_php)
* [header_filter_by_php](#header_filter_by_php)
* [body_filter_by_php](#body_filter_by_php)

php_ini_path
------------
* **syntax:** `php_ini_path`_`<php.ini file path>`_
* **context:** `http`
* **phase:** `loading-config`

init_worker_by_php
------------------
* **syntax:** `init_worker_by_php`_`<php script code>`_
* **context:** `http`
* **phase:** `starting-worker`

rewrite_by_php
--------------
* **syntax:** `rewrite_by_php`_`<php script code>`_
* **context:** `http, server, location, location if`
* **phase:** `rewrite`

access_by_php
-------------
* **syntax:** `access_by_php`_`<php script code>`_
* **context:** `http, server, location, location if`
* **phase:** `access`

content_by_php
--------------
* **syntax:** `content_by_php`_`<php script code>`_
* **context:** `http, server, location, location if`
* **phase:** `content`

log_by_php
----------
* **syntax:** `log_by_php`_`<php script code>`_
* **context:** `http, server, location, location if`
* **phase:** `log`

header_filter_by_php
--------------------
* **syntax:** `header_filter_by_php`_`<php script code>`_
* **context:** `http, server, location, location if`
* **phase:** `output-header-filter`

body_filter_by_php
------------------
* **syntax:** `body_filter_by_php`_`<php script code>`_
* **context:** `http, server, location, location if`
* **phase:** `output-body-filter`


Nginx API for php
-----------------
* [ngx_exit](#ngx_exit)
* [ngx_query_args](#ngx_query_args)
* [ngx_post_args](#ngx_post_args)
* [ngx_sleep](#ngx_sleep)
* [ngx_log_error](#ngx_log_error)
* [ngx_request_method](#ngx_request_method)
* [ngx_request_document_root](#ngx_request_document_root)
* [ngx_request_document_uri](#ngx_request_document_uri)
* [ngx_request_script_name](#ngx_request_script_name)
* [ngx_request_script_filename](#ngx_request_script_filename)
* [ngx_request_query_string](#ngx_request_query_string)
* [ngx_request_uri](#ngx_request_uri)
* [ngx_request_server_protocol](#ngx_request_server_protocol)
* [ngx_request_remote_addr](#ngx_request_remote_addr)
* [ngx_request_server_addr](#ngx_request_server_addr)
* [ngx_request_remote_port](#ngx_request_remote_port)
* [ngx_request_server_port](#ngx_request_server_port)
* [ngx_request_server_name](#ngx_request_server_name)
* [ngx_request_headers](#ngx_request_headers)
* [ngx_socket_create](#ngx_socket_create)
* [ngx_socket_connect](#ngx_socket_connect)
* [ngx_socket_close](#ngx_socket_close)
* [ngx_socket_send](#ngx_socket_send)
* [ngx_socket_recv](#ngx_socket_recv)
* [ngx_var_get](#ngx_var_get)
* [ngx_var_set](#ngx_var_set)


Copyright and License
---------------------
```
Copyright (c) 2016-2019, rryqszq4 <rryqszq@gmail.com>
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
