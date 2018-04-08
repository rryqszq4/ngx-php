#!/bin/bash
# Copyright (c) 2016-2017, rryqszq4 <rryqszq@gmail.com>
mkdir build
cd build
mkdir php
mkdir nginx
echo "php download ..."
wget http://php.net/distributions/php-${PHP_SRC_VERSION}.tar.gz
echo "php download ... done"
tar xf php-${PHP_SRC_VERSION}.tar.gz

PHP_SRC=`pwd`'/php-'${PHP_SRC_VERSION}
PHP_SRC_ROOT=`pwd`'/php'
cd ${PHP_SRC}

echo "php install ..."
./configure --prefix=${PHP_SRC_ROOT} \
--with-config-file-path=${PHP_SRC_ROOT} \
--with-mysqli=mysqlnd \
--with-pdo-mysql=mysqlnd \
--enable-shared \
--with-zlib \
--enable-xml \
--enable-bcmath \
--enable-shmop \
--enable-sysvsem \
--enable-sysvmsg \
--enable-sysvshm \
--with-curl \
--enable-mbregex \
--enable-mbstring \
--with-mcrypt \
--with-openssl \
--with-mhash \
--enable-pcntl \
--enable-sockets \
--with-xmlrpc \
--enable-zip \
--with-bz2 \
--with-gettext \
--with-readline \
--enable-bcmath \
--enable-soap \
--without-pear  \
--disable-cli \
--disable-cgi \
--enable-embed=shared
make
make install
#sudo ln -s ${PHP_SRC_ROOT}/lib/libphp7.so /usr/lib/libphp7.so
#sudo ln -s ${PHP_SRC_ROOT}/lib/libphp7.so /usr/local/lib/libphp7.so
echo "php install ... done"
cp php.ini-production ${PHP_SRC_ROOT}'/php.ini'

cd ..
echo "nginx download ..."
wget http://nginx.org/download/nginx-${NGINX_SRC_VERSION}.tar.gz
echo "nginx download ... done"
tar xf nginx-${NGINX_SRC_VERSION}.tar.gz

NGINX_SRC=`pwd`'/nginx-'${NGINX_SRC_VERSION}
NGINX_SRC_ROOT=`pwd`'/nginx'
cd ${NGINX_SRC}

export PHP_BIN=${PHP_SRC_ROOT}'/bin'
export PHP_INC=${PHP_SRC_ROOT}'/include/php'
export PHP_LIB=${PHP_SRC_ROOT}'/lib'
#export PHP_INI=${PHP_SRC_ROOT}'/php.ini'

#ls ${PHP_LIB}
ls ${PHP_SRC_ROOT}

echo "nginx install ..."
./configure --prefix=${NGINX_SRC_ROOT} \
--with-ld-opt="-Wl,-rpath,$PHP_LIB" \
--add-module=../../../ngx_php7/third_party/ngx_devel_kit \
--add-module=../../../ngx_php7
make
make install
if [ $? -eq 0 ];then
    echo "nginx install ... done"
    echo "ngx_php7 compile success."
else 
    echo "ngx_php7 compile failed."
    exit 1
fi
