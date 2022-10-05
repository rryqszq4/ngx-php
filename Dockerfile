# Nginx Docker Playground (https://github.com/nginx-with-docker/nginx-docker-playground)
FROM soulteary/prebuilt-nginx-modules:base-1.23.1-alpine AS Builder
RUN apk update && apk --no-cache add curl gcc g++ make musl-dev linux-headers gd-dev geoip-dev libxml2-dev libxslt-dev openssl-dev  pcre-dev perl-dev pkgconf zlib-dev libedit-dev ncurses-dev php8-dev php8-embed git unzip argon2-dev
ENV PHP_LIB=/usr/lib
WORKDIR /usr/src

# Nginx Development Kit (https://github.com/vision5/ngx_devel_kit)
ARG DEVEL_KIT_MODULE_CHECKSUM=e15316e13a7b19a3d2502becbb26043a464a135a
ARG DEVEL_KIT_VERSION=0.3.1
ARG DEVEL_KIT_NAME=ngx_devel_kit
RUN curl -L "https://github.com/vision5/ngx_devel_kit/archive/v${DEVEL_KIT_VERSION}.tar.gz" -o "v${DEVEL_KIT_VERSION}.tar.gz" && \
    echo "${DEVEL_KIT_MODULE_CHECKSUM}  v${DEVEL_KIT_VERSION}.tar.gz" | shasum -c && \
    tar -zxC /usr/src -f v${DEVEL_KIT_VERSION}.tar.gz && \
    mv ${DEVEL_KIT_NAME}-${DEVEL_KIT_VERSION}/ ${DEVEL_KIT_NAME}

# Nginx PHP Module Stable Version
ARG MODULE_CHECKSUM=9fe0ee2ca753cf82b8d982fef1e9888b1a59d8b1
ARG MODULE_VERSION=0.0.26
ARG MODULE_NAME=ngx-php
ARG MODULE_SOURCE=https://github.com/rryqszq4/ngx-php
RUN curl -L "${MODULE_SOURCE}/archive/refs/tags/v${MODULE_VERSION}.zip" -o "v${MODULE_VERSION}.zip" && \
    echo "${MODULE_CHECKSUM}  v${MODULE_VERSION}.zip" | shasum -c && \
    unzip "v${MODULE_VERSION}.zip" && \
    mv "$MODULE_NAME-$MODULE_VERSION" "$MODULE_NAME"

# Build with Nginx
RUN cd /usr/src/nginx && \
    CONFARGS=$(nginx -V 2>&1 | sed -n -e 's/^.*arguments: //p') \
    CONFARGS=${CONFARGS/-Os -fomit-frame-pointer -g/-Os} && \
    echo $CONFARGS && \
    ./configure --with-compat $CONFARGS --with-ld-opt="-Wl,-rpath,${PHP_LIB}" --add-dynamic-module=../${DEVEL_KIT_NAME} --add-dynamic-module=../${MODULE_NAME} && \
    make modules

# Generate Ngx-PHP configuration demo
RUN apk add bash
SHELL ["/bin/bash", "-c"]
RUN echo $' \n\
load_module modules/ndk_http_module.so; \n\
load_module modules/ngx_http_php_module.so; \n\
events { worker_connections 1024; } \n\
http { \n\
    include /etc/nginx/mime.types; \n\
    php_ini_path /etc/php8/php.ini; \n\
    gzip on; \n\
    server { \n\
        listen 80; \n\
        server_name localhost; \n\
        location = / { \n\
            content_by_php_block { \n\
                echo "hello world via ngx_php"; \n\
            } \n\
        } \n\
    } \n\
} \n\
'> /nginx.conf


# Build Final Nginx Docker Image
FROM nginx:1.23.1-alpine
LABEL MAINTAINER=soulteary@gmail.com
COPY --from=Builder /usr/lib/libphp.so          /usr/lib/
COPY --from=Builder /usr/lib/libargon2.so.1     /usr/lib/
COPY --from=Builder /lib/libz.so.1              /lib/
COPY --from=Builder /etc/php8/php.ini           /etc/php8/
COPY --from=Builder /nginx.conf                 /etc/nginx/nginx.conf
COPY --from=Builder /usr/src/nginx/objs/ndk_http_module.so      /etc/nginx/modules/
COPY --from=Builder /usr/src/nginx/objs/ngx_http_php_module.so  /etc/nginx/modules/
ENV PHP_LIB=/usr/lib
