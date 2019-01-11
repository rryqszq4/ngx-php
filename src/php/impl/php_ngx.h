/*
==============================================================================
Copyright (c) 2016-2019, rryqszq4 <rryqszq@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
==============================================================================
*/

#ifndef __PHP_NGX_H__
#define __PHP_NGX_H__

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


