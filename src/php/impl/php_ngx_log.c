/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#include "php_ngx_log.h"
#include "../../ngx_http_php_module.h"

static zend_class_entry *php_ngx_log_class_entry;

ZEND_BEGIN_ARG_INFO_EX(ngx_log_error_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, level)
    ZEND_ARG_INFO(0, log)
ZEND_END_ARG_INFO()

PHP_METHOD(ngx_log, error)
{
    long level;
    zend_string *log_str;
    ngx_http_request_t *r;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lS", &level, &log_str) == FAILURE){
        RETURN_NULL();
    }

    r = ngx_php_request;

    ngx_log_error((ngx_uint_t)level, r->connection->log, 0, "%*s", ZSTR_LEN(log_str), ZSTR_VAL(log_str));
}

static const zend_function_entry php_ngx_log_class_functions[] = {
    PHP_ME(ngx_log, error, ngx_log_error_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    {NULL, NULL, NULL, 0, 0}
};

void 
php_impl_ngx_log_init(int module_number TSRMLS_DC)
{
    zend_class_entry ngx_log_class_entry;
    INIT_CLASS_ENTRY(ngx_log_class_entry, "ngx_log", php_ngx_log_class_functions);
    php_ngx_log_class_entry = zend_register_internal_class(&ngx_log_class_entry TSRMLS_CC);

    /*zend_declare_class_constant_long(php_ngx_log_class_entry, "STDERR", sizeof("STDERR")-1, NGX_LOG_STDERR TSRMLS_CC);
    zend_declare_class_constant_long(php_ngx_log_class_entry, "EMERG", sizeof("EMERG")-1, NGX_LOG_EMERG TSRMLS_CC);
    zend_declare_class_constant_long(php_ngx_log_class_entry, "ALERT", sizeof("ALERT")-1, NGX_LOG_ALERT TSRMLS_CC);
    zend_declare_class_constant_long(php_ngx_log_class_entry, "CRIT", sizeof("CRIT")-1, NGX_LOG_CRIT TSRMLS_CC);
    zend_declare_class_constant_long(php_ngx_log_class_entry, "ERR", sizeof("ERR")-1, NGX_LOG_ERR TSRMLS_CC);
    zend_declare_class_constant_long(php_ngx_log_class_entry, "WARN", sizeof("WARN")-1, NGX_LOG_WARN TSRMLS_CC);
    zend_declare_class_constant_long(php_ngx_log_class_entry, "NOTICE", sizeof("NOTICE")-1, NGX_LOG_NOTICE TSRMLS_CC);
    zend_declare_class_constant_long(php_ngx_log_class_entry, "INFO", sizeof("INFO")-1, NGX_LOG_INFO TSRMLS_CC);
    zend_declare_class_constant_long(php_ngx_log_class_entry, "DEBUG", sizeof("DEBUG")-1, NGX_LOG_DEBUG TSRMLS_CC);
*/
}