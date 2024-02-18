#!/bin/bash
# Copyright (c) 2016-2017, rryqszq4 <rryqszq@gmail.com>

# Show PHP cli
echo "PHP version ..."

PHP_COMPILE_PATH=`pwd`'/build/php'
if [ -d "$PHP_COMPILE_PATH" ]; then
    ${PHP_COMPILE_PATH}'/bin/php-config'
else 
    /usr/bin/php${PHP_SRC_VERSION} -v
fi

ls

echo "Ngx-php test ..."

NGINX_COMPILE_PATH=`pwd`'/build/nginx/sbin'
if [ -d "$NGINX_COMPILE_PATH" ]; then
    NGX_PATH=${NGINX_COMPILE_PATH}
else 
    NGX_PATH=`pwd`'/nginx/sbin'
fi

${NGX_PATH}/nginx -V
export PATH=${NGX_PATH}:$PATH
NGX_MODULE_PATH=`pwd`'/nginx/modules'
if [ -d "${NGX_MODULE_PATH}" ]; then
  for file in `\find ${NGX_MODULE_PATH} -name '*.so' | sort -h | xargs find`; do
    TEST_NGINX_LOAD_MODULES="${TEST_NGINX_LOAD_MODULES}${file} "
  done
  export TEST_NGINX_LOAD_MODULES
fi

export TRAVIS_BUILD_DIR=`pwd`
echo $TRAVIS_BUILD_DIR

prove -r t