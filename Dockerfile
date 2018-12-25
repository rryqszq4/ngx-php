FROM nginx:stable-alpine as ngx-php

COPY config /build/config
COPY src /build/src
COPY third_party /build/third_party

ENV PHP_LIB=/usr/lib

WORKDIR /build

RUN apk --no-cache add curl gcc g++ make musl-dev linux-headers gd-dev geoip-dev libxml2-dev libxslt-dev openssl-dev paxmark pcre-dev perl-dev pkgconf zlib-dev libedit-dev ncurses-dev php7-dev php7-embed \
 && NGINX_VERSION=$(nginx -v 2>&1 | sed 's/^[^0-9]*//') \
 && curl -sL -o nginx-${NGINX_VERSION}.tar.gz http://nginx.org/download/nginx-${NGINX_VERSION}.tar.gz \
 && tar -xf nginx-${NGINX_VERSION}.tar.gz \
 && cd nginx-${NGINX_VERSION} \
 && ./configure $(nginx -V 2>&1 | tail -1 | sed -e 's/configure arguments://' -e 's| --add-dynamic-module=[^ ]*||g') --with-ld-opt="-Wl,-rpath,${PHP_LIB}" --add-dynamic-module=../third_party/ngx_devel_kit --add-dynamic-module=.. \
 && make \
 && mkdir -p /usr/lib/nginx/modules \
 && cp objs/ndk_http_module.so /usr/lib/nginx/modules \
 && cp objs/ngx_http_php_module.so /usr/lib/nginx/modules


FROM nginx:stable-alpine

RUN apk --no-cache add php7-embed \
 && sed -i "s|events {|include /etc/nginx/modules/\*.conf;\n\nevents {|" /etc/nginx/nginx.conf \
 && echo -e "load_module \"/usr/lib/nginx/modules/ndk_http_module.so\";\nload_module \"/usr/lib/nginx/modules/ngx_http_php_module.so\";" > /etc/nginx/modules/php.conf

COPY --from=ngx-php /usr/lib/nginx/modules/ /usr/lib/nginx/modules/
