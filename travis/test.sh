#!/bin/bash
# Copyright (c) 2016-2017, rryqszq4 <rryqszq@gmail.com>
echo "ngx_php7 test ..."
NGX_PATH=`pwd`'/build/nginx/sbin'
${NGX_PATH}/nginx -V
export PATH=${NGX_PATH}:$PATH
prove -r t