#!/bin/bash
# Copyright (c) 2016-2017, rryqszq4 <rryqszq@gmail.com>
echo "Ngx-php test ..."
NGX_PATH=`pwd`'/nginx/sbin'
${NGX_PATH}/nginx -V
export PATH=${NGX_PATH}:$PATH
NGX_MODULE_PATH=`pwd`'/nginx/modules'
if [ -d "${NGX_MODULE_PATH}" ]; then
  for file in `\find ${NGX_MODULE_PATH} -name '*.so'`; do
    TEST_NGINX_LOAD_MODULES="${TEST_NGINX_LOAD_MODULES}${file} "
  done
  export TEST_NGINX_LOAD_MODULES
fi

export TRAVIS_BUILD_DIR=`pwd`
echo $TRAVIS_BUILD_DIR
ls
prove -r t