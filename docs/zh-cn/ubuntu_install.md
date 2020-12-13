# ubuntu 系统安装指南

### 1. 使用package安装php
```sh
$ apt-get update -yqq && apt-get install -yqq software-properties-common

$ LC_ALL=C.UTF-8 add-apt-repository ppa:ondrej/php

$ apt-get update -yqq

$ apt-get install -yqq wget git unzip libxml2-dev cmake make systemtap-sdt-dev \
$                    zlibc zlib1g zlib1g-dev libpcre3 libpcre3-dev libargon2-0-dev libsodium-dev \
$                    php7.4 php7.4-common php7.4-dev libphp7.4-embed php7.4-mysql
```

### 2. 编译安装
```sh
$ git clone https://github.com/rryqszq4/ngx_php7.git

$ wget 'http://nginx.org/download/nginx-1.18.0.tar.gz'
$ tar -zxvf nginx-1.18.0.tar.gz
$ cd nginx-1.18.0

$ export PHP_LIB=/usr/lib

$ ./configure --user=www --group=www \
$             --prefix=/path/to/nginx \
$             --with-ld-opt="-Wl,-rpath,$PHP_LIB" \
$             --add-module=/path/to/ngx_php7/third_party/ngx_devel_kit \
$             --add-module=/path/to/ngx_php7
$ make && make install
```