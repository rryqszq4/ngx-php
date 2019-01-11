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

#ifndef __NGX_PHP_DEBUG_H__
#define __NGX_PHP_DEBUG_H__

#include <nginx.h>
#include <ngx_core.h>

#ifndef NGX_PHP_DEBUG
#define NGX_PHP_DEBUG 0
#endif

#if defined(NGX_PHP_DEBUG) && (NGX_PHP_DEBUG)

    #if (NGX_HAVE_VARIADIC_MACROS)

        #define ddebug(...) fprintf(stderr, "[ngx_php] [debug] %s: ", __func__); \
            fprintf(stderr, __VA_ARGS__); \
            fprintf(stderr, " at %s line %d.\n", __FILE__, __LINE__);

    #else

        #include <stdarg.h>
        #include <stdio.h>
        #include <stdarg.h>

        static void ddebug(const char *fmt, ...)
        {
        }

    #endif

#else
    
    #if (NGX_HAVE_VARIADIC_MACROS)

        #define ddebug(...)

    #else

        #include <stdarg.h>

        static void ddebug(const char *fmt, ...)
        {
        }

    #endif

#endif


#if defined(NGX_PHP_DEBUG) && (NGX_PHP_DEBUG)

    #if (NGX_HAVE_VARIADIC_MACROS)

        #define ngx_php_debug(...) fprintf(stderr, "[ngx_php] [debug] *%d %s: ", \
            (int) ngx_php_request->connection->log->connection, __func__); \
            fprintf(stderr, __VA_ARGS__); \
            fprintf(stderr, " at %s line %d.\n", __FILE__, __LINE__);

    #else

        #include <stdarg.h>
        #include <stdio.h>
        #include <stdarg.h>

        static void ngx_php_debug(const char *fmt, ...)
        {
        }

    #endif

#else
    
    #if (NGX_HAVE_VARIADIC_MACROS)

        #define ngx_php_debug(...)

    #else

        #include <stdarg.h>

        static void ngx_php_debug(const char *fmt, ...)
        {
        }

    #endif

#endif

#if defined(NGX_PHP_DEBUG) && (NGX_PHP_DEBUG)

#define ngx_php_debug_check_read_event_handler(r)                           \
            ngx_php_debug("r->read_event_handler = %s",                     \
                r->read_event_handler == ngx_http_block_reading ?           \
                    "ngx_http_block_reading" :                              \
                r->read_event_handler == ngx_http_test_reading ?            \
                    "ngx_http_test_reading" :                               \
                r->read_event_handler == ngx_http_request_empty_handler ?   \
                    "ngx_http_request_empty_handler" : "UNKNOWN")

#define ngx_php_debug_check_write_event_handler(r)                          \
            ngx_php_debug ("r->write_event_handler = %s",                   \
                r->write_event_handler == ngx_http_handler ?                \
                    "ngx_http_handler" :                                    \
                r->write_event_handler == ngx_http_core_run_phases ?        \
                    "ngx_http_core_run_phases" :                            \
                r->write_event_handler == ngx_http_request_empty_handler ?  \
                    "ngx_http_request_empty_handler" : "UNKNOWN")

#else

#define ngx_php_debug_check_read_event_handler(r)
#define ngx_php_debug_check_write_event_handler(r)

#endif


#endif


