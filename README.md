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

[Benchmarks about ngx_php7 and php](https://www.techempower.com/benchmarks/#section=test&runid=76a34044-54d6-4349-adfe-863c2d5ae756&hw=ph&test=plaintext&l=zik073-7)  

Table of contents
-----------------
* [What's different with official php](#Whats-different-with-official-php)
* [Requirement](#Requirement)
* [Installation](#Installation)
* [Synopsis](#Synopsis)
* [Test](#Test)
* [Directives](#Directives)
* [Nginx API for php](#Nginx-API-for-php)
* [Nginx non-blocking API for php](#Nginx-non-blocking-API-for-php)
* [Nginx constants](#Nginx-constants)
* [Copyright and License](#Copyright-and-License)

What's different with official php
----------------------------------
* Global variable is unsafe in per request
* Static variable of a class is unsafe in per request
* Do not design singleton mode
* The native IO function works fine, but it slows down nginx

Requirement
-----------
- Linux only
- PHP-7.0.* ~ PHP-7.3.*
- nginx-1.4.7 ~ nginx-1.14.2 and mainline 1.15.7

Installation
------------

### Compile install

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

### CentOS / RedHat 7

```sh
yum -y install https://extras.getpagespeed.com/release-el7-latest.rpm
yum -y install http://rpms.remirepo.net/enterprise/remi-release-7.rpm yum-utils
yum-config-manager --enable remi-php73
yum install nginx-module-php7
```

Edit `nginx.conf` and load the required modules at the top:

    load_module modules/ndk_http_module.so;
    load_module modules/ngx_http_php_module.so;

### Docker

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
                echo ngx_request_document_uri();
            ';
        }

        # curl /ngx_get?a=1&b=2
        location = /ngx_get {
            content_by_php '
                echo "ngx_query_args()\n";
                var_dump(ngx_query_args());
            ';
        }

        # curl -d 'a=1&b=2' /ngx_post
        location = /ngx_post {
            content_by_php '
                echo "ngx_post_args()\n";
                var_dump(ngx_post_args());
            ';
        }

        location = /ngx_sleep {
            content_by_php '
                echo "ngx_sleep start\n";
                yield ngx_sleep(1);
                echo "ngx_sleep end\n";
            ';
        }

        location = /ngx_socket2 {
            default_type 'application/json;charset=UTF-8';
            content_by_php '
                $fd = ngx_socket_create();

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
                $a = ngx_var_get("a");
                var_dump($a);
            ';
        }
        
        # set content-type of response headers
        location = /ngx_header {
            content_by_php '
                ngx_header_set("Content-Type", "text/html; charset=UTF-8");
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
                include ngx_var_get("uri");
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
* [init_worker_by_php_block](#init_worker_by_php_block)
* [rewrite_by_php](#rewrite_by_php)
* [rewrite_by_php_block](#rewrite_by_php_block)
* [access_by_php](#access_by_php)
* [access_by_php_block](#access_by_php_block)
* [content_by_php](#content_by_php)
* [content_by_php_block](#content_by_php_block)
* [log_by_php](#log_by_php)
* [log_by_php_block](#log_by_php_block)
* [header_filter_by_php](#header_filter_by_php)
* [header_filter_by_php_block](#header_filter_by_php_block)
* [body_filter_by_php](#body_filter_by_php)
* [body_filter_by_php_block](#body_filter_by_php_block)
* [php_keepalive](#php_keepalive)
* [php_set](#php_set)

php_ini_path
------------
**syntax:** `php_ini_path`_`<php.ini file path>`_

**context:** `http`

**phase:** `loading-config`

This directive allows loading the official php configuration file php.ini, which will be used by subsequent PHP code.

init_worker_by_php
------------------
**syntax:** `init_worker_by_php`_`<php script code>`_

**context:** `http`

**phase:** `starting-worker`

init_worker_by_php_block
------------------------
**syntax:** `init_worker_by_php_block`_`{php script code}`_

**context:** `http`

**phase:** `starting-worker`

rewrite_by_php
--------------
**syntax:** `rewrite_by_php`_`<php script code>`_

**context:** `http, server, location, location if`

**phase:** `rewrite`

In the rewrite phase of nginx, you can execute inline php code.

rewrite_by_php_block
--------------------
**syntax:** `rewrite_by_php_block`_`{php script code}`_

**context:** `location, location if`

**phase:** `rewrite`

In the rewrite phase of nginx, you can execute inline php code.

access_by_php
-------------
**syntax:** `access_by_php`_`<php script code>`_

**context:** `http, server, location, location if`

**phase:** `access`

In the access phase of nginx, you can execute inline php code.

access_by_php_block
-------------------
**syntax:** `access_by_php_block`_`{php script code}`_

**context:** `location, location if`

**phase:** `access`

In the access phase of nginx, you can execute inline php code.

content_by_php
--------------
**syntax:** `content_by_php`_`<php script code>`_

**context:** `http, server, location, location if`

**phase:** `content`

In the content phase of nginx, you can execute inline php code.

content_by_php_block
--------------------
**syntax:** `content_by_php_block`_`{php script code}`_

**context:** `location, location if`

**phase:** `content`

In the content phase of nginx, you can execute inline php code.

log_by_php
----------
**syntax:** `log_by_php`_`<php script code>`_

**context:** `http, server, location, location if`

**phase:** `log`

log_by_php_block
----------------
**syntax:** `log_by_php_block`_`{php script code}`_

**context:** `location, location if`

**phase:** `log`

header_filter_by_php
--------------------
**syntax:** `header_filter_by_php`_`<php script code>`_

**context:** `http, server, location, location if`

**phase:** `output-header-filter`

header_filter_by_php_block
--------------------------
**syntax:** `header_filter_by_php_block`_`{php script code}`_

**context:** `location, location if`

**phase:** `output-header-filter`

body_filter_by_php
------------------
**syntax:** `body_filter_by_php`_`<php script code>`_

**context:** `http, server, location, location if`

**phase:** `output-body-filter`

body_filter_by_php_block
------------------------
**syntax:** `body_filter_by_php_block`_`{php script code}`_

**context:** `location, location if`

**phase:** `output-body-filter`

php_keepalive
-------------
**syntax:** `php_keepalive`_`<size>`_

**default:** `0`

**context:** `http, server`

In php, set upstream connection pool size.

php_set
-------
**syntax:** `php_set`_`$variable`_ _`<php script code>`_

**context:** `http, server, location, location if`

**phase:** `loading-config`

Installs a php handler for the specified variable.

Nginx API for php
-----------------
* [ngx_exit](#ngx_exit)
* [ngx_query_args](#ngx_query_args)
* [ngx_post_args](#ngx_post_args)
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
* [ngx_var_get](#ngx_var_get)
* [ngx_var_set](#ngx_var_set)
* [ngx_header_set](#ngx_header_set)
* [ngx_header_get](#ngx_header_get)
* [ngx_header_get_all](#ngx_header_get_all)
* [ngx_redirect](#ngx_redirect)
* [ngx_cookie_get_all](#ngx_cookie_get_all)
* [ngx_cookie_get](#ngx_cookie_get)
* [ngx_cookie_set](#ngx_cookie_set)

ngx_exit
--------
**syntax:** `ngx_exit(int $status) : void`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

End of current request and return http status code.

ngx_query_args
--------------
**syntax:** `ngx_query_args(void) : array` or `ngx::query_args(void) : array`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

An associative array of variables passed to the current script via the URL parameters (aka. query string).  
Instead of php official constant $_GET.

ngx_post_args
-------------
**syntax:** `ngx_post_args(void) : array` or `ngx::post_args(void) : array`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

An associative array of variables passed to the current script via the HTTP POST method  
when using application/x-www-form-urlencoded or multipart/form-data as the HTTP Content-Type in the request.  
Instead of php official constant $_POST.

ngx_log_error
-------------
**syntax:** `ngx_log_error(int $level, string $log_str) : void` or `ngx_log::error(int $level, string $log_str) : void`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

ngx_request_method
------------------
**syntax:** `ngx_request_method(void) : string` or `ngx_request::method(void) : string`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

Which request method was used to access the page, such as 'GET','POST','PUT','DELETE' and so on.

ngx_request_document_root
-------------------------
**syntax:** `ngx_request_document_root(void) : string` or `ngx_request::document_root(void) : string`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

The document root directory under which the current script is executing, as defined in the server's configuration file.

ngx_request_document_uri
------------------------
**syntax:** `ngx_request_document_uri(void) : string` or `ngx_request::document_uri(void) : string`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

ngx_request_script_name
-----------------------
**syntax:** `ngx_request_script_name(void) : string` or `ngx_request::script_name(void) : string`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

Contains the current script'path. This is useful for pages which need to point to the themselves.  
The __FILE__ constant contains the full path and filename of the current (included) file.

ngx_request_script_filename
---------------------------
**syntax:** `ngx_request_script_filename(void) : string` or `ngx_request::script_filename(void) : string`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

The absolute pathname of the currently executing script file name.

ngx_request_query_string
------------------------
**syntax:** `ngx_request_query_string(void) : string` or `ngx_request::query_string(void) : string`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

The query string, if any, via which the page was accessed.

ngx_request_uri
---------------
**syntax:** `ngx_request_uri(void) : string` or `ngx_request::uri(void) : string`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

The URI which was given in order to access this page, for instance, '/index.html'.

ngx_request_server_protocol
---------------------------
**syntax:** `ngx_request_server_protocol(void) : string` or `ngx_request::server_protocol(void) : string`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

Name and revision of th information protocol via which the page was requested, such as 'HTTP/1.0'.

ngx_request_remote_addr
-----------------------
**syntax:** `ngx_request_remote_addr(void) : string` or `ngx_request::remote_addr(void) : string`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

The IP address from which the user is viewing the current page.

ngx_request_server_addr
-----------------------
**syntax:** `ngx_request_server_addr(void) : string` or `ngx_request::server_addr(void) : string`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

The IP address of the server under which the current script is executing.

ngx_request_remote_port
-----------------------
**syntax:** `ngx_request_remote_port(void) : int` or `ngx_request::remote_port(void) : int`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

The port being used on the user's machine to communicate with the web server.

ngx_request_server_port
-----------------------
**syntax:** `ngx_request_server_port(void) : int` or `ngx_request::server_port(void) : int`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

The port on the server machine being used by the web server for communication. For default setups,  
this will be '80'; using SSL, for instance, will change this to whatever your defined secure HTTP port is.

ngx_request_server_name
-----------------------
**syntax:** `ngx_request_server_name(void) : string` or `ngx_request::server_name(void) : string`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

The name of the server host under which the current script is executing.   
If the script is running on a virtual host, this will be the value defined for that virtual host.

ngx_request_headers
-------------------
**syntax:** `ngx_request_headers(void): array` or `ngx_request::headers(void) : array`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

Get the header full information of the http request.

ngx_var_get
-----------
**syntax:** `ngx_var_get(string $key) : string` or `ngx_var::get(string $key) : string`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

Get the variables in the nginx configuration.

ngx_var_set
-----------
**syntax:** `ngx_var_set(string $key, string $value) : void` or `ngx_var::set(string $key, string $value) : void`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

Set the variables in the nginx configuration.

ngx_header_set
--------------
**syntax:** `ngx_header_set(string $key, string $value) : bool`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

Set the header information of the http response.

ngx_header_get
--------------
**syntax:** `ngx_header_get(string $key) : string`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

Get the header information of the http response.

ngx_header_gets
---------------
**syntax:** `ngx_header_gets(void) : array`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

Get the header full information of the http response.

ngx_redirect
------------
**syntax:** `ngx_redirect(string $uri, int $status) : bool`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

Set response header redirection.

ngx_cookie_get_all
------------------
**syntax:** `ngx_cookie_get_all(void) : string`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

ngx_cookie_get
--------------
**syntax:** `ngx_cookie_get(string key) : string`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

ngx_cookie_set
--------------
**syntax:** `ngx_cookie_set(string data): bool`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`


Nginx non-blocking API for php
------------------------------
* [yield ngx_sleep](#ngx_sleep)
* [yield ngx_msleep](#ngx_msleep)
* [ngx_socket_create](#ngx_socket_create)
* [ngx_socket_iskeepalive](#ngx_socket_iskeepalive)
* [yield ngx_socket_connect](#ngx_socket_connect)
* [yield ngx_socket_close](#ngx_socket_close)
* [yield ngx_socket_send](#ngx_socket_send)
* [yield ngx_socket_recv](#ngx_socket_recv)
* [ngx_socket_clear](#ngx_socket_clear)

ngx_sleep
---------
**syntax:** `yield ngx_sleep(int seconds)`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

Delays the program execution for the given number of seconds.

ngx_msleep
---------
**syntax:** `yield ngx_msleep(int milliseconds)`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

Delays the program execution for the given number of milliseconds.

ngx_socket_create
-----------------
**syntax:** `ngx_socket_create(int $domain, int $type, int $protocol) : resource`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

Creates and returns a socket resource, also referred to as an endpoint of communication.  
A typical network connection is made up of 2 sockets, one performing the role of the client,  
and another performing the role of the server.

ngx_socket_iskeepalive
----------------------
**syntax:** `ngx_socket_iskeepalive(void) : bool`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

ngx_socket_connect
------------------
**syntax:** `( yield ngx_socket_connect(resource $socket, string $address, int $port) ) : bool`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

Initiate a connection to address using the socket resource socket, which must be a valid  
socket resource created with ngx_socket_create().

ngx_socket_close
----------------
**syntax:** `( yield ngx_socket_close(resource $socket) ) : bool`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

ngx_socket_close() closes the socket resource given by socket. This function is specific to  
sockets and cannot be used on any other type of resources.

ngx_socket_send
---------------
**syntax:** `( yield ngx_socket_send(resource $socket, string $buf, int $len) ) : int`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

The function ngx_socket_send() sends len bytes to the socket socket from buf.

ngx_socket_recv
---------------
**syntax:** `( yield ngx_socket_recv(resource $socket, string &$buf, int $len) ) : int`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

The ngx_socket_recv() function receives len bytes of data in buf from socket. ngx_socket_recv() can be  
used to gather data from connected sockets. 

buf is passed by reference, so it must be specified as a variable in the argument list.  
Data read from socket by ngx_socket_recv() will be returned in buf.

ngx_socket_clear
----------------
**syntax:** `ngx_socket_recv(resource $socket) : bool`

**context:** `rewrite_by_php*, access_by_php*, content_by_php*`

Close the socket resource and is blocking but hight performance.

Nginx constants
---------------
* [version constants](version-constants)
* [log constants for php](#log-constants-for-php)
* [status constants for php](#status-constants-for-php)
* [http status constants for php](#http-status-constants-for-php)

version constants
-----------------
name | value
-|-
NGINX_VAR                       | NGINX
NGINX_VERSION                   | 1.12.2
NGX_HTTP_PHP_MODULE_VERSION     | 0.0.21
NGX_HTTP_PHP_MODULE_NAME        | ngx_php

log constants for php
---------------------
name | value
-|-
NGX_OK          | 0
NGX_ERROR       | -1
NGX_AGAIN       | -2
NGX_BUSY        | -3
NGX_DONE        | -4
NGX_DECLINED    | -5
NGX_ABORT       | -6

status constants for php
------------------------
name | value
-|-
NGX_LOG_STDERR  | 0
NGX_LOG_EMERG   | 1
NGX_LOG_ALERT   | 2
NGX_LOG_CRIT    | 3
NGX_LOG_ERR     | 4
NGX_LOG_WARN    | 5
NGX_LOG_NOTICE  | 6
NGX_LOG_INFO    | 7
NGX_LOG_DEBUG   | 8

http status constants for php
-----------------------------
name | value
-|-
NGX_HTTP_CONTINUE                   | 100
NGX_HTTP_SWITCHING_PROTOCOLS        | 101
NGX_HTTP_PROCESSING                 | 102
NGX_HTTP_OK                         | 200
NGX_HTTP_CREATED                    | 201
NGX_HTTP_ACCEPTED                   | 202
NGX_HTTP_NO_CONTENT                 | 204
NGX_HTTP_PARTIAL_CONTENT            | 206
NGX_HTTP_SPECIAL_RESPONSE           | 300
NGX_HTTP_MOVED_PERMANENTLY          | 301
NGX_HTTP_MOVED_TEMPORARILY          | 302
NGX_HTTP_SEE_OTHER                  | 303
NGX_HTTP_NOT_MODIFIED               | 304
NGX_HTTP_TEMPORARY_REDIRECT         | 307
NGX_HTTP_PERMANENT_REDIRECT         | 308
NGX_HTTP_BAD_REQUEST                | 400
NGX_HTTP_UNAUTHORIZED               | 401
NGX_HTTP_FORBIDDEN                  | 403
NGX_HTTP_NOT_FOUND                  | 404
NGX_HTTP_NOT_ALLOWED                | 405
NGX_HTTP_REQUEST_TIME_OUT           | 408
NGX_HTTP_CONFLICT                   | 409
NGX_HTTP_LENGTH_REQUIRED            | 411
NGX_HTTP_PRECONDITION_FAILED        | 412
NGX_HTTP_REQUEST_ENTITY_TOO_LARGE   | 413
NGX_HTTP_REQUEST_URI_TOO_LARGE      | 414
NGX_HTTP_UNSUPPORTED_MEDIA_TYPE     | 415
NGX_HTTP_RANGE_NOT_SATISFIABLE      | 416
NGX_HTTP_CLOSE                      | 444
NGX_HTTP_NGINX_CODES                | 494
NGX_HTTP_REQUEST_HEADER_TOO_LARGE   | 494
NGX_HTTPS_CERT_ERROR                | 495
NGX_HTTPS_NO_CERT                   | 496
NGX_HTTP_TO_HTTPS                   | 497
NGX_HTTP_CLIENT_CLOSED_REQUEST      | 499
NGX_HTTP_INTERNAL_SERVER_ERROR      | 500
NGX_HTTP_NOT_IMPLEMENTED            | 501
NGX_HTTP_BAD_GATEWAY                | 502
NGX_HTTP_SERVICE_UNAVAILABLE        | 503
NGX_HTTP_GATEWAY_TIME_OUT           | 504
NGX_HTTP_INSUFFICIENT_STORAGE       | 507


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
