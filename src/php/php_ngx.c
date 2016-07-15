/**
 *    Copyright(c) 2016 rryqszq4
 *
 *
 */

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_ngx.h"

/* If you declare any globals in php_php_ngx.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(php_ngx)
*/

/* True global resources - no need for thread safety here */
static int le_php_ngx;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("php_ngx.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_php_ngx_globals, php_ngx_globals)
    STD_PHP_INI_ENTRY("php_ngx.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_php_ngx_globals, php_ngx_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_php_ngx_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_php_ngx_compiled)
{
    char *arg = NULL;
    size_t arg_len, len;
    zend_string *strg;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
        return;
    }

    strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "php_ngx", arg);

    RETURN_STR(strg);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_php_ngx_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_php_ngx_init_globals(zend_php_ngx_globals *php_ngx_globals)
{
    php_ngx_globals->global_value = 0;
    php_ngx_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(php_ngx)
{
    /* If you have INI entries, uncomment these lines
    REGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(php_ngx)
{
    /* uncomment this line if you have INI entries
    UNREGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(php_ngx)
{
#if defined(COMPILE_DL_PHP_NGX) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(php_ngx)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(php_ngx)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "php_ngx support", "enabled");
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
    DISPLAY_INI_ENTRIES();
    */
}
/* }}} */

/* {{{ php_ngx_functions[]
 *
 * Every user visible function must have an entry in php_ngx_functions[].
 */
const zend_function_entry php_ngx_functions[] = {
    PHP_FE(confirm_php_ngx_compiled,    NULL)       /* For testing, remove later. */
    PHP_FE_END  /* Must be the last line in php_ngx_functions[] */
};
/* }}} */

/* {{{ php_ngx_module_entry
 */
zend_module_entry php_ngx_module_entry = {
    STANDARD_MODULE_HEADER,
    "php_ngx",
    php_ngx_functions,
    PHP_MINIT(php_ngx),
    PHP_MSHUTDOWN(php_ngx),
    PHP_RINIT(php_ngx),     /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(php_ngx), /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(php_ngx),
    PHP_PHP_NGX_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PHP_NGX
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(php_ngx)
#endif