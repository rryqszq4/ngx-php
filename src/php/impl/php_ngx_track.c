/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_generators.h"
#include "zend_vm.h"

#include "php_ngx.h"
#include "php_ngx_track.h"
#include "../../ngx_http_php_module.h"

static int ngx_track_zval(zval zv);
static void ngx_track_znode(unsigned int node_type, znode_op node, zend_op_array *op_array TSRMLS_DC);
static void ngx_track_op_array(zend_op_array *op_array TSRMLS_DC);
static int ngx_track_fe_wrapper(zval *el TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key);
static int ngx_track_cle_wrapper (zval *el TSRMLS_DC);
//static int ngx_check_fe_wrapper (zval *el, zend_bool *have_fe TSRMLS_DC);
static int ngx_track_fe(zend_op_array *fe TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key);
static int ngx_track_cle(zend_class_entry *class_entry TSRMLS_DC);
//static int ngx_check_fe(zend_op_array *fe, zend_bool *have_fe TSRMLS_DC);
static inline zend_class_entry *ngx_get_called_scope(const zend_execute_data *e);
static inline const char *ngx_get_executed_filename(void);
static void ngx_function_name(zend_execute_data *execute_data);

static void ngx_track_print_sample_tabs(ngx_uint_t depth);
static void ngx_stack_print_tab(ngx_uint_t depth, int flag);

void ngx_track_zend_op_array(zend_op_array *op_array);

zend_op_array* ngx_compile_file(zend_file_handle* file_handle, int type TSRMLS_DC)
{
    zend_file_handle fake;
    zend_op_array *op_array;
    char *filename;
    char *bufptr;
    size_t len;

    if (zend_stream_fixup(file_handle, &bufptr, &len) == FAILURE) {
        return ori_compile_file(file_handle, type TSRMLS_CC);
    }

    //php_printf("%*s", len, bufptr);

    filename = (char *)(file_handle->opened_path ? ZSTR_VAL(file_handle->opened_path) : file_handle->filename);

    memset(&fake, 0, sizeof(fake));
    fake.type = ZEND_HANDLE_MAPPED;
    fake.handle.stream.mmap.buf = bufptr;
    fake.handle.stream.mmap.len = len;
    fake.free_filename = 0;
    fake.filename = filename;
    fake.opened_path = file_handle->opened_path;

    op_array = ori_compile_file(&fake, type);

    zend_file_handle_dtor(&fake);
    zend_file_handle_dtor(file_handle);

    //op_array = ori_compile_file(file_handle, type TSRMLS_CC);

    ngx_http_request_t *r = ngx_php_request;
    ngx_http_php_ctx_t *ctx;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    //debug
    //ngx_track_zend_op_array(op_array);

    if (op_array && (ctx->output_type & OUTPUT_OPCODE)) {
        ctx->output_type = OUTPUT_CONTENT;

        //ngx_http_set_ctx(r, ctx, ngx_http_php_module);

        if (!ctx->opcode_logo) {
php_printf("    ___                   __    \n");
php_printf("  /`__ \\___  ___  ___  __/ /__  \n");
php_printf(" / /_/ / _ \\/ _ `/ _ \\/ _ / __\\ \n");
php_printf(" \\___./ .__/\\___.\\___/\\___\\__..   /ngx_php7_tracker\n"); 
php_printf("     /_/                         /version: %s\n", NGX_HTTP_PHP_MODULE_VERSION);
php_printf("\n/* ~: IS_TMP_VAR, $: IS_VAR, !: IS_CV */\n\n");
            ctx->opcode_logo = 1;
        }

        ngx_track_op_array(op_array TSRMLS_CC);

        zend_hash_apply_with_arguments(CG(function_table) TSRMLS_CC, (apply_func_args_t) ngx_track_fe_wrapper, 0);
    
        zend_hash_apply(CG(class_table), (apply_func_t) ngx_track_cle_wrapper TSRMLS_CC);
        
        ctx->output_type = OUTPUT_OPCODE;

        ngx_http_set_ctx(r, ctx, ngx_http_php_module);
    }

    //ctx->enable_output = 0;

    //ngx_http_set_ctx(r, ctx, ngx_http_php_module);

    return op_array;
}

zend_op_array *ngx_compile_string(zval *source_string, char *filename TSRMLS_DC)
{
    zend_op_array *op_array;

    op_array = ori_compile_string(source_string, filename TSRMLS_CC);

    ngx_http_request_t *r = ngx_php_request;
    ngx_http_php_ctx_t *ctx;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    //debug
    //ngx_track_zend_op_array(op_array);

    if (op_array && (ctx->output_type & OUTPUT_OPCODE)) {
        ctx->output_type = OUTPUT_CONTENT;

        //ngx_http_set_ctx(r, ctx, ngx_http_php_module);

        if (!ctx->opcode_logo) {
php_printf("    ___                   __    \n");
php_printf("  /`__ \\___  ___  ___  __/ /__  \n");
php_printf(" / /_/ / _ \\/ _ `/ _ \\/ _ / __\\ \n");
php_printf(" \\___./ .__/\\___.\\___/\\___\\__..   /ngx_php7_tracker\n"); 
php_printf("     /_/                         /version: %s\n", NGX_HTTP_PHP_MODULE_VERSION);
php_printf("\n/* ~: IS_TMP_VAR, $: IS_VAR, !: IS_CV */\n\n");
            ctx->opcode_logo = 1;
        }

        ngx_track_op_array(op_array TSRMLS_CC);

        zend_hash_apply_with_arguments(CG(function_table) TSRMLS_CC, (apply_func_args_t) ngx_track_fe_wrapper, 0);
    
        zend_hash_apply(CG(class_table), (apply_func_t) ngx_track_cle_wrapper TSRMLS_CC);
        
        ctx->output_type = OUTPUT_OPCODE;

        ngx_http_set_ctx(r, ctx, ngx_http_php_module);
    }

    //ctx->enable_output = 0;

    //ngx_http_set_ctx(r, ctx, ngx_http_php_module);

    return op_array;
}

static int ngx_track_zval(zval zv)
{
    switch (zv.u1.v.type) {
        case IS_UNDEF: 
            php_printf("%-16s","<undef>");
            return IS_UNDEF;
        case IS_NULL : 
            php_printf("%-16s","<null>");
            return IS_NULL;
        case IS_FALSE : 
            php_printf("%-16s","<false>");
            return IS_FALSE;
        case IS_TRUE : 
            php_printf("%-16s","<true>");
            return IS_TRUE;
        case IS_LONG : 
            php_printf("%-16ld", zv.value.lval);
            return IS_LONG;
        case IS_DOUBLE : 
            php_printf("%-16g", zv.value.dval);
            return IS_DOUBLE;
        case IS_STRING: 
            php_printf("%-16s", ZSTR_VAL(zv.value.str));
            return IS_STRING;
        case IS_ARRAY: 
            php_printf("%-16s","<array>");
            return IS_ARRAY;
        case IS_OBJECT: 
            php_printf("%-16s","<object>");
            return IS_OBJECT;
        case IS_RESOURCE: 
            php_printf("%-16s","<resource>");
            return IS_RESOURCE;
        case IS_REFERENCE: 
            php_printf("%-16s","<reference>");
            return IS_REFERENCE;
        case IS_CONSTANT: 
            php_printf("%-16s","<constant>");
            return IS_CONSTANT;
        case IS_CALLABLE: 
            php_printf("%-16s","<callable>");
            return IS_CALLABLE;
        case IS_INDIRECT: 
            php_printf("%-16s","<indirect>");
            return IS_INDIRECT;
        case IS_PTR: 
            php_printf("%-16s","<prt>");
            return IS_PTR;
        default: 
            php_printf("%-16s","<unknown>");
            return -1;
    }
}

static void ngx_track_znode(unsigned int node_type, znode_op node, zend_op_array *op_array TSRMLS_DC)
{
    switch (node_type) {
        case IS_UNDEF:
            ngx_track_zval(*RT_CONSTANT_EX(op_array->literals, node));
            break;
        case IS_CONST:
            ngx_track_zval(*RT_CONSTANT_EX(op_array->literals, node));
            break;
        case IS_TMP_VAR:
            php_printf("~%-15d", EX_VAR_TO_NUM(node.var));
            break;
        case IS_VAR:
            php_printf("$%-15d", EX_VAR_TO_NUM(node.var));
            break;
        case IS_CV:
            php_printf("!%-15d", EX_VAR_TO_NUM(node.var));
            break;
        default:
            php_printf("%-16s", " ");
            break;
    }
}

static void ngx_track_op_array(zend_op_array *op_array TSRMLS_DC)
{
    unsigned int i;
    zend_op op;

    php_printf("filename: '%s'\n", op_array->filename?ZSTR_VAL(op_array->filename):NULL);

    if (op_array->scope) {
        php_printf("function_name: %s::%s\n", ZSTR_VAL(op_array->scope->name), op_array->function_name?ZSTR_VAL(op_array->function_name):NULL);
    }else {
        php_printf("function_name: %s\n", op_array->function_name?ZSTR_VAL(op_array->function_name):NULL);
    }

    php_printf("    %-6s%-6s%-38s%-16s%-16s%-16s\n","id","line","opcode","op1","op2","result");
    php_printf("    >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

    for (i = 0; i < op_array->last; i++) {
        op = op_array->opcodes[i];
        php_printf("    %-6d%-6d%-38s", 
            i, 
            op.lineno, 
            zend_get_opcode_name(op.opcode)
            //op.op1_type,
            //op.op2_type,
            //op.result_type
            );
        ngx_track_znode(op.op1_type, op.op1, op_array TSRMLS_CC);
        ngx_track_znode(op.op2_type, op.op2, op_array TSRMLS_CC);
        ngx_track_znode(op.result_type, op.result, op_array TSRMLS_CC);
        php_printf("\n");
    }

    php_printf("\n");
}

static int ngx_track_fe_wrapper(zval *el TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
    return ngx_track_fe((zend_op_array *) Z_PTR_P(el) TSRMLS_CC, num_args, args, hash_key);
}

static int ngx_track_cle_wrapper (zval *el TSRMLS_DC)
{
    return ngx_track_cle((zend_class_entry *) Z_PTR_P(el) TSRMLS_CC);
}

/*static int ngx_check_fe_wrapper (zval *el, zend_bool *have_fe TSRMLS_DC)
{
    return ngx_check_fe((zend_op_array *) Z_PTR_P(el), have_fe TSRMLS_CC);
}*/

static int ngx_track_fe(zend_op_array *fe TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
    if (fe->type == ZEND_USER_FUNCTION) {
        ngx_track_op_array(fe TSRMLS_CC);
    }

    return ZEND_HASH_APPLY_KEEP;
}

static int ngx_track_cle(zend_class_entry *class_entry TSRMLS_DC)
{
    zend_class_entry *ce;
    //zend_bool have_fe = 0;

    ce = class_entry;

    if (ce->type != ZEND_INTERNAL_CLASS) {
        //zend_hash_apply_with_arguments(&ce->function_table, (apply_func_args_t) ngx_check_fe_wrapper, (int )&have_fe TSRMLS_CC);

        //if (have_fe) {
            zend_hash_apply_with_arguments(&ce->function_table TSRMLS_CC, (apply_func_args_t) ngx_track_fe_wrapper, 0);
        //}
    }

    return ZEND_HASH_APPLY_KEEP; 
}

/*static int ngx_check_fe(zend_op_array *fe, zend_bool *have_fe TSRMLS_DC)
{
    if (fe->type == ZEND_USER_FUNCTION) {
        *have_fe = 1;
    }

    return 0;
}*/

static inline zend_class_entry *ngx_get_called_scope(const zend_execute_data *e)
{
#if PHP_VERSION_ID < 70100
    return e->called_scope;
#else
    return zend_get_called_scope((zend_execute_data *) e);
#endif
}

static inline const char *ngx_get_executed_filename(void)
{
    zend_execute_data *ex = EG(current_execute_data);

    while (ex && (!ex->func || !ZEND_USER_CODE(ex->func->type))) {
        ex = ex->prev_execute_data;
    }
    if (ex) {
        return ZSTR_VAL(ex->func->op_array.filename);
    } else {
        return zend_get_executed_filename();
    }
}

static void ngx_function_name(zend_execute_data *execute_data)
{
    zend_execute_data *data;
    zend_function *curr_func;
    const char * cls;
    const char * func;
    const zend_op *opline;

    data = EG(current_execute_data);

    if (data) {
        curr_func = data->func;
        if (curr_func->common.function_name) {
            func = curr_func->common.function_name->val;
            cls = curr_func->common.scope ?
                    curr_func->common.scope->name->val :
                    (ngx_get_called_scope(data) ?
                            ngx_get_called_scope(data)->name->val : NULL);
            if (cls) {
                php_printf("%s::%-20s", cls, func);
            }else {
                php_printf("%-30s", func);
            }
        }else {
            if (data->prev_execute_data) {
                opline = data->prev_execute_data->opline;
            }else {
                opline = data->opline;
            }

            if (opline) {
                switch (opline->extended_value) {
                    case ZEND_EVAL:
                        php_printf("%-30s", "eval");
                        break;
                    case ZEND_INCLUDE:
                        php_printf("%-30s", "include");
                        break;
                    case ZEND_REQUIRE:
                        php_printf("%-30s", "require");
                        break;
                    case ZEND_INCLUDE_ONCE:
                        php_printf("%-30s", "include_once");
                        break;
                    case ZEND_REQUIRE_ONCE:
                        php_printf("%-30s", "require_once");
                        break;
                    default:
                        php_printf("%-30s", "main");
                        break;
                }
            }
        }

        //php_printf("\n");
    }
}

static void ngx_track_print_sample_tabs(ngx_uint_t depth)
{
    int i;
    for (i = 0; i < (int)depth; i++) {
        php_printf("    ");
    }
}

static void ngx_stack_print_tab(ngx_uint_t depth, int flag)
{
    int i;
    for (i = 0; i <= (int)depth; i++) {
        /*if (i == 0) {
            if (flag == 0) {
                php_printf("   ");
            }else {
                php_printf("|---");
            }
        }else {
            if (flag == 0) {
                php_printf("    ");
            }else {
                php_printf("|---");
            }
        }*/
        if (i == (int)depth) {
            if (flag == 0) {
                php_printf("*   ");
            }else {
                php_printf("|---");
            }
        }else {
            php_printf("    ");
        }
    }
}

void 
ngx_track_zend_op(zend_op *opline, int tabs_len)
{
    if (opline) {
        ngx_track_print_sample_tabs(tabs_len);php_printf("------------------------------\n");
        ngx_track_print_sample_tabs(tabs_len);php_printf("|zend_op = %p {\n", opline);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.handler = %p\n", opline->handler);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.op1 = %p\n", opline->op1);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.op2 = %p\n", opline->op2);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.result = %p\n", opline->result);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.extended_value = %d\n", opline->extended_value);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.lineno = %d\n", opline->lineno);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.opcode = %d\n", opline->opcode);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.op1_type = %d\n", opline->op1_type);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.op2_type = %d\n", opline->op2_type);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.result_type = %d\n", opline->result_type);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|}\n");
        ngx_track_print_sample_tabs(tabs_len);php_printf("------------------------------\n");
    }
}

void
ngx_track_zend_op_array(zend_op_array *op_array)
{
    int i;
    zend_op op;

    if (op_array) {
        php_printf("------------------------------\n");
        php_printf("|zend_op_array = %p {\n", op_array);
        php_printf("|.type = %d\n", op_array->type);
        php_printf("|.arg_flags = %d\n", op_array->arg_flags);
        php_printf("|.fn_flags = %p\n", op_array->fn_flags);
            if (op_array->scope) {
        php_printf("|    .function_name = %s::%s\n", ZSTR_VAL(op_array->scope->name), op_array->function_name?ZSTR_VAL(op_array->function_name):NULL);
            }else {
        php_printf("|    .function_name = %s\n", op_array->function_name?ZSTR_VAL(op_array->function_name):"(null)");
            }
        php_printf("|    .opcodes = %p\n", op_array->opcodes);
            for (i = 0; i < (int)op_array->last; i++) {
                op = op_array->opcodes[i];
        php_printf("|    [%d].opcode = %p(%s)\n", i, &op_array->opcodes[i], zend_get_opcode_name(op.opcode));
        ngx_track_zend_op(&op, 2);
            } 
        php_printf("|}\n");
        php_printf("------------------------------\n");

    }
}

void 
ngx_track_zend_execute_data(zend_execute_data *execute_data)
{
    int i;
    zend_function *fbc;
    zend_op_array op_array;
    zend_op op;
    zend_generator *generator;

    if (execute_data) {
        php_printf("------------------------------\n");
        php_printf("|zend_execute_data = %p {\n", execute_data);
        php_printf("|.opline = %p\n", execute_data->opline);
        if (execute_data->opline) {
        php_printf("|    .opcode = %s\n", zend_get_opcode_name(execute_data->opline->opcode));
        }
        php_printf("|.call = %p\n", execute_data->call);
        php_printf("|.return_value = %p\n", execute_data->return_value);
        if (execute_data->return_value) {
        //if (execute_data->func->common.fn_flags == 0x8800101) {
            generator = (zend_generator *)execute_data->return_value;
            if (generator) {
        //php_printf("|.execute_data = %p\n", generator->execute_data);
                ngx_track_zend_generator(generator, 1);
            }
        }
        php_printf("|.func = %p\n", execute_data->func);
        if (execute_data->func) {
        php_printf("|    .type = %d\n", execute_data->func->type,execute_data->func->type);
            if (&execute_data->func->common) {
        php_printf("|    .common = %p\n", execute_data->func->common);
            fbc = execute_data->func;
        php_printf("|        .type = %d\n", fbc->common.type);
        php_printf("|        .arg_flags = %p\n", fbc->common.arg_flags);
        php_printf("|        .fn_flags = %p\n", fbc->common.fn_flags);
        php_printf("|        .function_name = %s\n", fbc->common.function_name?ZSTR_VAL(fbc->common.function_name):"(null)");
        php_printf("|        .prototype = %p\n", fbc->common.prototype);
        php_printf("|        .num_args = %p\n", fbc->common.num_args);
        php_printf("|        .required_num_args = %p\n", fbc->common.required_num_args);    
        php_printf("|        .arg_info = %p\n", fbc->common.arg_info);
            }
            if (&execute_data->func->op_array) {
        php_printf("|    .op_array = %p\n", execute_data->func->op_array);
                op_array = execute_data->func->op_array;
        php_printf("|        .type = %d\n", op_array.type);
        php_printf("|        .fn_flags = %p\n", op_array.fn_flags);
        php_printf("|        .last = %d\n", op_array.last);
                if (op_array.scope) {
        php_printf("|        .function_name = %s::%s\n", ZSTR_VAL(op_array.scope->name), op_array.function_name?ZSTR_VAL(op_array.function_name):NULL);
                }else {
        php_printf("|        .function_name = %s\n", op_array.function_name?ZSTR_VAL(op_array.function_name):"(null)");
                }
        php_printf("|        .opcodes = %p\n", op_array.opcodes);
                for (i = 0; i < (int)op_array.last; i++) {
        op = op_array.opcodes[i];
        php_printf("|        [%d].opcode = %p(%s)\n", i, &op_array.opcodes[i], zend_get_opcode_name(op.opcode));
                }    
            }
        }
        php_printf("|.This = %p\n", execute_data->This);
#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 1
        php_printf("|.called_scope = %p\n", execute_data->called_scope);
#endif
        php_printf("|.prev_execute_data = %p\n", execute_data->prev_execute_data);
        php_printf("|.symbol_table = %p\n", execute_data->symbol_table);
        php_printf("|}\n");
        php_printf("------------------------------\n");
    }
}

void 
ngx_track_zend_generator(zend_generator *generator, int tabs_len)
{

    if (generator) {

        ngx_track_print_sample_tabs(tabs_len);php_printf("------------------------------\n");
        ngx_track_print_sample_tabs(tabs_len);php_printf("|zend_generator = %p {\n", generator);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.std = %p\n", generator->std);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.iterator = %p\n", generator->iterator);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.execute_data = %p\n", generator->execute_data);
        //ngx_track_zend_execute_data(generator->execute_data);
#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 1
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.stack = %p\n", generator->stack);
#endif
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.value = %p\n", generator->value);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.key = %p\n", generator->key);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.retval = %p\n", generator->retval);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.send_target = %p\n", generator->send_target);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.largest_used_integer_key = %p\n", generator->largest_used_integer_key);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.values = %p\n", generator->values);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.node = %p\n", generator->node);
        ngx_track_zend_generator_node(&generator->node, tabs_len*2);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.execute_fake = %p\n", generator->execute_fake);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.flags = %p\n", generator->flags);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|}\n");
        ngx_track_print_sample_tabs(tabs_len);php_printf("------------------------------\n");

    }
}

void 
ngx_track_zend_generator_node(zend_generator_node *node, int tabs_len)
{

    if (node) {

        ngx_track_print_sample_tabs(tabs_len);php_printf("------------------------------\n");
        ngx_track_print_sample_tabs(tabs_len);php_printf("|zend_generator_node = %p {\n", node);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.parent = %p\n", node->parent);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.children = %p\n", node->children);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.child = %p\n", node->child);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|.ptr = %p\n", node->ptr);
        ngx_track_print_sample_tabs(tabs_len);php_printf("|}\n");
        ngx_track_print_sample_tabs(tabs_len);php_printf("------------------------------\n");

    }
}

void ngx_execute_ex(zend_execute_data *execute_data TSRMLS_DC)
{
    int lineno;
    const char *filename;

    ngx_http_request_t *r = ngx_php_request;
    ngx_http_php_ctx_t *ctx;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx->output_type & OUTPUT_STACK) {
        ctx->output_type = OUTPUT_CONTENT;

        if (!ctx->stack_logo) {
php_printf("    ____                __      \n");
php_printf("   / __/_.._ ___  ___  / /__    \n");
php_printf("  _\\ \\/_  __/ _ `/ _ `/  '_/    \n");
php_printf(" /___/ /__/ \\_,\\/\\___/_/\\_\\     /ngx_php7_tracker\n");
php_printf("                               /version: %s\n\n", NGX_HTTP_PHP_MODULE_VERSION);
            ctx->stack_logo = 1;
        }
        //ngx_http_set_ctx(r, ctx, ngx_http_php_module);
        
        ngx_stack_print_tab(ctx->stack_depth,1);
        //php_printf("%d", ctx->stack_depth);

        ngx_function_name(execute_data);

        lineno = zend_get_executed_lineno();

        filename = ngx_get_executed_filename();
        php_printf("%s line %-6d", filename, lineno);
        php_printf("\n");

        ctx->output_type = OUTPUT_STACK;
        ctx->stack_depth += 1;

        ngx_http_set_ctx(r, ctx, ngx_http_php_module);
    }

    struct timeval tv_start;
    struct timeval tv_end;
    gettimeofday(&tv_start, 0);

    //debug
    //ngx_track_zend_execute_data(execute_data);

    ori_execute_ex(execute_data TSRMLS_CC);

    gettimeofday(&tv_end, 0);

    if (ctx->output_type & OUTPUT_STACK) {
        ctx->output_type = OUTPUT_CONTENT;
        ctx->stack_depth -= 1;

        //ngx_http_set_ctx(r, ctx, ngx_http_php_module);
        ngx_stack_print_tab(ctx->stack_depth,0);
        php_printf("real time: %f sec", (float)(((tv_end.tv_sec - tv_start.tv_sec) * 1000000) + (tv_end.tv_usec - tv_start.tv_usec))/1000000);
        php_printf("\n");

        ctx->output_type = OUTPUT_STACK;

        ngx_http_set_ctx(r, ctx, ngx_http_php_module);
    }
}

void ngx_execute_internal(zend_execute_data *execute_data, zval *return_value TSRMLS_DC)
{
    int lineno;
    const char *filename;

    ngx_http_request_t *r = ngx_php_request;
    ngx_http_php_ctx_t *ctx;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx->output_type & OUTPUT_STACK) {
        ctx->output_type = OUTPUT_CONTENT;

        if (!ctx->stack_logo) {
php_printf("    ____                __      \n");
php_printf("   / __/_.._ ___  ___  / /__    \n");
php_printf("  _\\ \\/_  __/ _ `/ _ `/  '_/    \n");
php_printf(" /___/ /__/ \\_,\\/\\___/_/\\_\\     /ngx_php7_tracker\n");
php_printf("                               /version: %s\n\n", NGX_HTTP_PHP_MODULE_VERSION);
            ctx->stack_logo = 1;
        }
        //ngx_http_set_ctx(r, ctx, ngx_http_php_module);
        
        ngx_stack_print_tab(ctx->stack_depth,1);
        //php_printf("%d", ctx->stack_depth);

        ngx_function_name(execute_data);

        lineno = zend_get_executed_lineno();

        filename = ngx_get_executed_filename();
        php_printf("%s line %-6d", filename, lineno);
        php_printf("\n");

        ctx->output_type = OUTPUT_STACK;
        ctx->stack_depth += 1;

        ngx_http_set_ctx(r, ctx, ngx_http_php_module);
    }

    struct timeval tv_start;
    struct timeval tv_end;
    gettimeofday(&tv_start, 0);

    //debug
    //ngx_track_zend_execute_data(execute_data);

    execute_internal(execute_data, return_value);

    gettimeofday(&tv_end, 0);

    if (ctx->output_type & OUTPUT_STACK) {
        ctx->output_type = OUTPUT_CONTENT;
        ctx->stack_depth -= 1;

        //ngx_http_set_ctx(r, ctx, ngx_http_php_module);
        ngx_stack_print_tab(ctx->stack_depth,0);
        php_printf("real time: %f sec", (float)(((tv_end.tv_sec - tv_start.tv_sec) * 1000000) + (tv_end.tv_usec - tv_start.tv_usec))/1000000);
        php_printf("\n");
        
        ctx->output_type = OUTPUT_STACK;

        ngx_http_set_ctx(r, ctx, ngx_http_php_module);
        
    }
}



