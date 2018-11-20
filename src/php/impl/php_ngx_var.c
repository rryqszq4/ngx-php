/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#include "php_ngx_var.h"
#include "../../ngx_http_php_module.h"
#include "../../ngx_http_php_variable.h"

static zend_class_entry *php_ngx_var_class_entry;

PHP_FUNCTION(ngx_var_get)
{
    zend_string                 *key_str;
    ngx_http_variable_value_t   *var;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S", &key_str) == FAILURE){
        RETURN_NULL();
    }

    var = ngx_http_php_var_get(ZSTR_VAL(key_str),ZSTR_LEN(key_str));

    ZVAL_STRINGL(return_value, (char *) var->data, var->len);
}

PHP_FUNCTION(ngx_var_set)
{
    zend_string *key_str;
    zend_string *value_str;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "SS", &key_str, &value_str) == FAILURE) {
        RETURN_NULL();
    }

    ngx_http_php_var_set(ZSTR_VAL(key_str), ZSTR_LEN(key_str), ZSTR_VAL(value_str), ZSTR_LEN(value_str));
}

PHP_METHOD(ngx_var, get)
{
    zend_string                 *key_str;
    ngx_http_variable_value_t   *var;
  	
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S", &key_str) == FAILURE){
        RETURN_NULL();
    }

    var = ngx_http_php_var_get(ZSTR_VAL(key_str),ZSTR_LEN(key_str));

    ZVAL_STRINGL(return_value, (char *) var->data, var->len);

}

PHP_METHOD(ngx_var, set)
{
    zend_string *key_str;
    zend_string *value_str;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "SS", &key_str, &value_str) == FAILURE) {
        RETURN_NULL();
    }

    ngx_http_php_var_set(ZSTR_VAL(key_str), ZSTR_LEN(key_str), ZSTR_VAL(value_str), ZSTR_LEN(value_str));
}

static const zend_function_entry php_ngx_var_class_functions[] = {
    PHP_ME(ngx_var, get, ngx_var_get_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_var, set, ngx_var_set_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    {NULL, NULL, NULL, 0, 0}
};

void 
php_impl_ngx_var_init(int module_number TSRMLS_DC)
{
    zend_class_entry ngx_var_class_entry;
    INIT_CLASS_ENTRY(ngx_var_class_entry, "ngx_var", php_ngx_var_class_functions);
    php_ngx_var_class_entry = zend_register_internal_class(&ngx_var_class_entry TSRMLS_CC);

}