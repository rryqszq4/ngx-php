# mac osx 系统安装指南

## 编译安装php

1. 下载编译，make的时候会报错
```sh
$ wget 'http://php.net/distributions/php-7.4.4.tar.gz'
$ tar xf php-7.4.4.tar.gz
$ cd php-7.4.4

$ ./configure --prefix=/path/to/php --enable-embed
$ make
```

2. 下载ngx_php，执行修复脚本, 如果报错再执行一次修复脚本
```sh
$ git clone https://github.com/rryqszq4/ngx_php7.git

$ sh ngx_php7/utils/php-bug-66673.sh /path/to/php

$ cd php-7.4.4
$ make

$ sh ngx_php7/utils/php-bug-66673.sh /path/to/php

$ cd php-7.4.4
$ make
```

3. 修改Makefile文件
	注释掉libs/libphp$(PHP_MAJOR_VERSION).bundle...行
	添加libphp7.dylib行
```makefile
#libs/libphp$(PHP_MAJOR_VERSION).bundle: $(PHP_GLOBAL_OBJS) $(PHP_SAPI_OBJS)
#	$(CC) $(MH_BUNDLE_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(PHP_GLOBAL_OBJS:.lo=.o) $(PHP_SAPI_OBJS:.lo=.o) $(PHP_FRAMEWORKS) $(EXTRA_LIBS) $(ZEND_EXTRA_LIBS) -o $@ && cp $@ libs/libphp$(PHP_MAJOR_VERSION).so

libphp7.dylib: $(PHP_GLOBAL_OBJS) $(PHP_SAPI_OBJS)
	$(CC) $(MH_BUNDLE_FLAGS) $(CFLAGS_CLEAN) -fPIC -dynamiclib $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(PHP_GLOBAL_OBJS:.lo=.o) $(PHP_SAPI_OBJS:.lo=.o) $(PHP_FRAMEWORKS) $(EXTRA_LIBS) $(ZEND_EXTRA_LIBS) -o $@

```

	如果报错clang: error: unknown argument: '-module', 修改EXTRA_LDFLAGS...行如下：
```
```makefile
#EXTRA_LDFLAGS = -avoid-version -module -L/usr/local/opt/libiconv/lib

EXTRA_LDFLAGS = -avoid-version -L/usr/local/opt/libiconv/lib
```

	编译libphp7.dylib, 然后拷贝到php安装目录lib中
```sh
$ make libphp7.dylib

$ cp libphp7.dylib /path/to/php/lib
```

4. 编译nginx

```sh
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
