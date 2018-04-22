/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#ifndef _PHP_NGX_H_
#define _PHP_NGX_H_

#include <php.h>
#include <SAPI.h>
#include <php_main.h>
#include <php_variables.h>
#include <php_ini.h>
#include <zend_ini.h>
#include <zend_exceptions.h>
#include <ext/standard/php_standard.h>
#include <ext/standard/info.h>

#include <ngx_http.h>

#include "php_ngx_track.h"

extern ngx_http_request_t * ngx_php_request;

int php_ngx_module_init();
void php_ngx_module_shutdown(TSRMLS_D);

int php_ngx_request_init(TSRMLS_D);
void php_ngx_request_shutdown(TSRMLS_D);

extern sapi_module_struct php_ngx_module;

extern zend_module_entry php_ngx_module_entry;
#define phpext_php_ngx_ptr &php_ngx_module_entry

#define PHP_PHP_NGX_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#   define PHP_PHP_NGX_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_PHP_NGX_API __attribute__ ((visibility("default")))
#else
#   define PHP_PHP_NGX_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/*
    Declare any global variables you may need between the BEGIN
    and END macros here:
*/
ZEND_BEGIN_MODULE_GLOBALS(php_ngx)
    zend_long  global_value;
    char *global_string;
    ngx_http_request_t *global_r;
ZEND_END_MODULE_GLOBALS(php_ngx)


/* Always refer to the globals in your function as PHP_NGX_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
#define PHP_NGX_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(php_ngx, v)

#if defined(ZTS) && defined(COMPILE_DL_PHP_NGX)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif  

/* PHP_PHP_NGX_H */


