
# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

$ENV{'TEST_NGINX_DIR'} = $ENV{'TRAVIS_BUILD_DIR'};

run_tests();

__DATA__
=== TEST 1: ini file
ini file
--- http_config
php_ini_path $TEST_NGINX_DIR/.github/ngx-php/php.ini;
--- config
location = /opcache {
    content_by_php '
        echo opcache_get_status() === false ? 'enabled' : 'disabled';
    ';
}
--- request
GET /opcache
--- response_body eval
enabled