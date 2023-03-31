#!/bin/bash
# Copyright (c) 2016-2017, rryqszq4 <rryqszq@gmail.com>

# echo "where is mysqld.sock ..."
# netstat -ln | awk '/mysql(.*)?\.sock/ { print $9 }' 

# mkdir build
# cd build
# mkdir php
# mkdir nginx

# if [ ${PHP_SRC_VERSION:0:1} -ge "8" ]; then
#   PHP_MAJOR_VERSION=""
# else
#   PHP_MAJOR_VERSION=${PHP_SRC_VERSION:0:1}
# fi

# echo "nginx download ..."
# wget http://nginx.org/download/nginx-${NGINX_SRC_VERSION}.tar.gz
# echo "nginx download ... done"
# tar -zxf nginx-${NGINX_SRC_VERSION}.tar.gz

NGINX_SRC=`pwd`'/nginx-'${NGINX_SRC_VERSION}
NGINX_SRC_ROOT=`pwd`'/nginx'
cd ${NGINX_SRC}

# Show pwd
pwd
export PHP_CONFIG='/usr/bin/php-config'
export PHP_LIB='/usr/lib'
export NGX_PHP_LIBS="`$PHP_CONFIG --ldflags` `$PHP_CONFIG --libs` -L$PHP_LIB -lphp$PHP_MAJOR_VERSION "

#ls ${PHP_SRC_ROOT}

echo "nginx install ..."
if [ ! "${NGINX_MODULE}" = "DYNAMIC" ]; then
  ./configure --prefix=${NGINX_SRC_ROOT} \
              --with-ld-opt="-Wl,-rpath,$PHP_LIB" \
              --add-module=../third_party/ngx_devel_kit \
              --add-module=..
else
  ./configure --prefix=${NGINX_SRC_ROOT} \
              --with-ld-opt="-Wl,-rpath,$PHP_LIB" \
              --add-dynamic-module=../third_party/ngx_devel_kit \
              --add-dynamic-module=..
fi
make
make install
if [ $? -eq 0 ];then
    echo "nginx install ... done"
    echo "ngx_php $NGINX_SRC_VERSION with $PHP_SRC_VERSION compile success."
else 
    echo "ngx_php Nginx:$NGINX_SRC_VERSION with PHP-$PHP_SRC_VERSION compile failed."
    exit 1
fi

if [ "${NGINX_MODULE}" = "DYNAMIC" ]; then
  echo "nginx dynamic module install ..."
  mkdir -p ${NGINX_SRC_ROOT}/modules
  cp ./objs/*.so ${NGINX_SRC_ROOT}/modules/
fi
