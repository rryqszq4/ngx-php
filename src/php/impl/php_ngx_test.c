/**
 *    Copyright(c) 2016 rryqszq4
 *
 *
 */

#include "php_ngx_test.h"
#include "../../ngx_http_php_module.h"

static zend_class_entry *php_ngx_test_generator_class_entry;

ZEND_BEGIN_ARG_INFO_EX(ngx_test_generator_construct_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_test_generator_yield_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_test_generator_send_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_test_generator_receive_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()


PHP_METHOD(ngx_test_generator, __construct)
{

}

PHP_METHOD(ngx_test_generator, yield)
{

}

PHP_METHOD(ngx_test_generator, send)
{

}

PHP_METHOD(ngx_test_generator, receive)
{

}


static const zend_function_entry php_ngx_test_generator_class_functions[] = {
    PHP_ME(ngx_test_generator, __construct, ngx_test_generator_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(ngx_test_generator, yield, ngx_test_generator_yield_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(ngx_test_generator, send, ngx_test_generator_send_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(ngx_test_generator, receive, ngx_test_generator_receive_arginfo, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL, 0, 0}
};

void 
php_impl_ngx_test_init(int module_number TSRMLS_DC)
{
    zend_class_entry ngx_test_generator_class_entry;
    INIT_CLASS_ENTRY(ngx_test_generator_class_entry, "ngx_test_generator", php_ngx_test_generator_class_functions);
    php_ngx_test_generator_class_entry = zend_register_internal_class(&ngx_test_generator_class_entry TSRMLS_CC);
}
