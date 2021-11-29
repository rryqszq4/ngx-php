/*
==============================================================================
Copyright (c) 2016-2020, rryqszq4 <rryqszq@gmail.com>
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

#include "ngx_http_php_module.h"
#include "ngx_http_php_core.h"
#include "ngx_php_conf_file.h"
#include "ngx_http_php_directive.h"
#include "ngx_http_php_variable.h"
#include "ngx_http_php_handler.h"

static char *ngx_http_php_init_worker_block_phase_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static char *ngx_http_php_rewrite_block_phase_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static char *ngx_http_php_access_block_phase_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static char *ngx_http_php_content_block_phase_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static char *ngx_http_php_log_block_phase_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static char *ngx_http_php_header_filter_block_phase_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static char *ngx_http_php_body_filter_block_phase_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static char *
ngx_http_php_init_worker_block_phase_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    if (pmcf->init_worker_inline_code != NGX_CONF_UNSET_PTR){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_string(cf->pool, &value[0]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    pmcf->init_worker_inline_code = code;
    pmcf->enabled_init_worker_handler = 1;

    return NGX_CONF_OK;
}

static char *
ngx_http_php_rewrite_block_phase_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    /*if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }*/

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->rewrite_handler != NULL){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_string(cf->pool, &value[0]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    plcf->rewrite_inline_code = code;
    plcf->rewrite_handler = ngx_http_php_rewrite_inline_handler;
    pmcf->enabled_rewrite_handler = 1;

    return NGX_CONF_OK;
}

static char *
ngx_http_php_access_block_phase_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    /*if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }*/

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->access_handler != NULL){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_string(cf->pool, &value[0]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    plcf->access_inline_code = code;
    plcf->access_handler = ngx_http_php_access_inline_handler;
    pmcf->enabled_access_handler = 1;

    return NGX_CONF_OK;
}

static char *
ngx_http_php_content_block_phase_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    /*if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }*/

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->content_handler != NULL){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_string(cf->pool, &value[0]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    plcf->content_inline_code = code;
    plcf->content_handler = ngx_http_php_content_inline_handler;
    pmcf->enabled_content_handler = 1;

    return NGX_CONF_OK;
}

static char *
ngx_http_php_log_block_phase_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    /*if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }*/

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->log_handler != NULL){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_file(cf->pool, &value[0]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    plcf->log_code = code;
    plcf->log_handler = ngx_http_php_log_inline_handler;
    pmcf->enabled_log_handler = 1;

    return NGX_CONF_OK;
}

static char *
ngx_http_php_header_filter_block_phase_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    /*if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }*/

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->header_filter_handler != NULL) {
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_string(cf->pool, &value[0]);
    if (code == NGX_CONF_UNSET_PTR) {
        return NGX_CONF_ERROR;
    }

    plcf->header_filter_inline_code = code;
    plcf->header_filter_handler = ngx_http_php_header_filter_inline_handler;
    pmcf->enabled_header_filter = 1;

    return NGX_CONF_OK;
}

static char *
ngx_http_php_body_filter_block_phase_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    /*if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }*/

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->body_filter_handler != NULL) {
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_string(cf->pool, &value[0]);
    if (code == NGX_CONF_UNSET_PTR) {
        return NGX_CONF_ERROR;
    }

    plcf->body_filter_inline_code = code;
    plcf->body_filter_handler = ngx_http_php_body_filter_inline_handler;
    pmcf->enabled_body_filter = 1;

    return NGX_CONF_OK;
}

char *
ngx_http_php_ini_path(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf = conf;
    ngx_str_t *value;

    if (pmcf->ini_path.len != 0){
        return "is duplicated";
    }

    value = cf->args->elts;

    pmcf->ini_path.len = value[1].len;
    pmcf->ini_path.data = value[1].data;

    return NGX_CONF_OK;
}

char *
ngx_http_php_init_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    if (pmcf->init_inline_code != NGX_CONF_UNSET_PTR){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_string(cf->pool, &value[1]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }
    pmcf->init_inline_code = code;

    return NGX_CONF_OK;
}

char *
ngx_http_php_init_file_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    if (pmcf->init_code != NGX_CONF_UNSET_PTR){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_file(cf->pool, &value[1]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }
    pmcf->init_code = code;

    return NGX_CONF_OK;
}

char *
ngx_http_php_init_worker_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    if (pmcf->init_worker_inline_code != NGX_CONF_UNSET_PTR){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_string(cf->pool, &value[1]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }
    pmcf->init_worker_inline_code = code;
    pmcf->enabled_init_worker_handler = 1;

    return NGX_CONF_OK;
}

char *
ngx_http_php_rewrite_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->rewrite_handler != NULL){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_file(cf->pool, &value[1]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    plcf->rewrite_code = code;
    plcf->rewrite_handler = cmd->post;
    pmcf->enabled_rewrite_handler = 1;

    return NGX_CONF_OK;
}

char *
ngx_http_php_rewrite_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->rewrite_handler != NULL){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_string(cf->pool, &value[1]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    plcf->rewrite_inline_code = code;
    plcf->rewrite_handler = cmd->post;
    pmcf->enabled_rewrite_handler = 1;

    return NGX_CONF_OK;
}

char *
ngx_http_php_access_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->access_handler != NULL){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_file(cf->pool, &value[1]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    plcf->access_code = code;
    plcf->access_handler = cmd->post;
    pmcf->enabled_access_handler = 1;

    return NGX_CONF_OK;
}

char *
ngx_http_php_access_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->access_handler != NULL){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_string(cf->pool, &value[1]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    plcf->access_inline_code = code;
    plcf->access_handler = cmd->post;
    pmcf->enabled_access_handler = 1;

    return NGX_CONF_OK;
}

char *
ngx_http_php_content_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->content_handler != NULL){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_file(cf->pool, &value[1]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    plcf->content_code = code;
    plcf->content_handler = cmd->post;
    pmcf->enabled_content_handler = 1;

    return NGX_CONF_OK;
}

char *
ngx_http_php_content_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->content_handler != NULL){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_string(cf->pool, &value[1]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    plcf->content_inline_code = code;
    plcf->content_handler = cmd->post;
    pmcf->enabled_content_handler = 1;

    return NGX_CONF_OK;
}

char *
ngx_http_php_init_worker_block_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    char        *rv;
    ngx_conf_t   save;

    save = *cf;
    cf->handler = ngx_http_php_init_worker_block_phase_handler;
    cf->handler_conf = conf;

    rv = ngx_php_conf_parse(cf, NULL);
    *cf = save;

    return rv;
}

char *
ngx_http_php_rewrite_block_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    char        *rv;
    ngx_conf_t   save;

    save = *cf;
    cf->handler = ngx_http_php_rewrite_block_phase_handler;
    cf->handler_conf = conf;

    rv = ngx_php_conf_parse(cf, NULL);
    *cf = save;

    return rv;
}

char *
ngx_http_php_access_block_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    char        *rv;
    ngx_conf_t   save;

    save = *cf;
    cf->handler = ngx_http_php_access_block_phase_handler;
    cf->handler_conf = conf;

    rv = ngx_php_conf_parse(cf, NULL);
    *cf = save;

    return rv;
}

char *
ngx_http_php_content_block_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    char        *rv;
    ngx_conf_t   save;

    save = *cf;
    cf->handler = ngx_http_php_content_block_phase_handler;
    cf->handler_conf = conf;

    rv = ngx_php_conf_parse(cf, NULL);
    *cf = save;

    return rv;
}

char *
ngx_http_php_log_block_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    char        *rv;
    ngx_conf_t   save;

    save = *cf;
    cf->handler = ngx_http_php_log_block_phase_handler;
    cf->handler_conf = conf;

    rv = ngx_php_conf_parse(cf, NULL);
    *cf = save;

    return rv;
}

char *
ngx_http_php_header_filter_block_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    char        *rv;
    ngx_conf_t   save;

    save = *cf;
    cf->handler = ngx_http_php_header_filter_block_phase_handler;
    cf->handler_conf = conf;

    rv = ngx_php_conf_parse(cf, NULL);
    *cf = save;

    return rv;
}

char *
ngx_http_php_body_filter_block_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    char        *rv;
    ngx_conf_t   save;

    save = *cf;
    cf->handler = ngx_http_php_body_filter_block_phase_handler;
    cf->handler_conf = conf;

    rv = ngx_php_conf_parse(cf, NULL);
    *cf = save;

    return rv;
}

char *
ngx_http_php_opcode_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->opcode_handler != NULL) {
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_string(cf->pool, &value[1]);
    if (code == NGX_CONF_UNSET_PTR) {
        return NGX_CONF_ERROR;
    }

    plcf->opcode_inline_code = code;
    plcf->opcode_handler = cmd->post;
    pmcf->enabled_opcode_handler = 1;

    return NGX_CONF_OK;
}

char *
ngx_http_php_stack_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->stack_handler != NULL) {
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_string(cf->pool, &value[1]);
    if (code == NGX_CONF_UNSET_PTR) {
        return NGX_CONF_ERROR;
    }

    plcf->stack_inline_code = code;
    plcf->stack_handler = cmd->post;
    pmcf->enabled_stack_handler = 1;

    return NGX_CONF_OK;
}

char *
ngx_http_php_log_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->log_handler != NULL){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_file(cf->pool, &value[1]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    plcf->log_code = code;
    plcf->log_handler = cmd->post;
    pmcf->enabled_log_handler = 1;

    return NGX_CONF_OK;
}

char *
ngx_http_php_log_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->log_handler != NULL){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_string(cf->pool, &value[1]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    plcf->log_inline_code = code;
    plcf->log_handler = cmd->post;
    pmcf->enabled_log_handler = 1;

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_php_variable_handler(ngx_http_request_t *r, ngx_http_variable_value_t *v,
    uintptr_t data)
{
    ngx_http_php_variable_t *pv = (ngx_http_php_variable_t *) data;

    ngx_http_php_var_set((char *)pv->key.data, pv->key.len, (char *)pv->handler.data, pv->handler.len);
    
    return NGX_OK;

}

char *
ngx_http_php_set_inline2(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_int_t                   index;
    ngx_str_t                   *value;
    ngx_str_t                   handler;
    ngx_http_variable_t         *v;
    ngx_http_php_variable_t     *pv;
    //ngx_http_php_main_conf_t    *pmcf;

    value = cf->args->elts;

    if (value[1].data[0] != '$') {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid variable name \"%V\"", &value[1]);
        return NGX_CONF_ERROR;
    }

    value[1].len--;
    value[1].data++;

    v = ngx_http_add_variable(cf, &value[1], NGX_HTTP_VAR_CHANGEABLE);
    if (v == NULL) {
        return NGX_CONF_ERROR;
    }

    pv = ngx_palloc(cf->pool, sizeof(ngx_http_php_variable_t));
    if (pv == NULL) {
        return NGX_CONF_ERROR;
    }

    index = ngx_http_get_variable_index(cf, &value[1]);
    if (index == NGX_ERROR) {
        return NGX_CONF_ERROR;
    }

    //pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);

    pv->code = ngx_http_php_code_from_string(cf->pool, &value[2]);
    if (pv->code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    php_ngx_module_init();

#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 2
    //zend_startup_module(&php_ngx_module_entry);
#else
    EG(current_module) = &php_ngx_module_entry;
    EG(current_module)->type = MODULE_PERSISTENT;
#endif

    php_ngx_request_init();

        zval retval;
        zend_eval_string_ex(pv->code->code.string, &retval, "ngx_php run code return", 1 );

        if (Z_TYPE(retval) == IS_TRUE || 
            Z_TYPE(retval) == IS_FALSE ||  
            Z_TYPE(retval) == IS_LONG ||
            Z_TYPE(retval) == IS_DOUBLE ||
            Z_TYPE(retval) == IS_STRING ){

            convert_to_string(&retval);

            handler.data = ngx_pnalloc(cf->pool, Z_STRLEN(retval));
            ngx_memcpy(handler.data, Z_STRVAL(retval), Z_STRLEN(retval));
            handler.len = Z_STRLEN(retval);
        } else {
            handler.data = NULL;
            handler.len = 0;
        }

        zval_dtor(&retval);

    php_ngx_request_shutdown();
    php_ngx_module_shutdown();

    pv->key = value[1];
    pv->handler = handler;

    v->get_handler = ngx_http_php_variable_handler;
    v->data = (uintptr_t) pv;

    return NGX_CONF_OK;
}

#if defined(NDK) && NDK

char *
ngx_http_php_set_inline(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t *value;
    ngx_str_t target;
    ndk_set_var_t filter;

    ngx_http_php_set_var_data_t *filter_data;

    /*
        value[0] = "php_set_code"
        value[1] = target variable name
        value[2] = php code
        value[3..] = real params
    */

    value = cf->args->elts;
    target = value[1];

    filter.type = NDK_SET_VAR_MULTI_VALUE_DATA;
    filter.func = cmd->post;
    filter.size = cf->args->nelts - 3;

    filter_data = ngx_palloc(cf->pool, sizeof(ngx_http_php_set_var_data_t));
    if (filter_data == NULL){
        return NGX_CONF_ERROR;
    }

    filter_data->size = filter.size;
    filter_data->var_name = value[1];
    filter_data->script = value[2];

    filter_data->code = ngx_http_php_code_from_string(cf->pool, &filter_data->script);
    if (filter_data->code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    filter.data = filter_data;
    
    return ndk_set_var_multi_value_core(cf, &target, &value[3], &filter);
}

char *
ngx_http_php_set_run_inline(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t *value;
    ngx_str_t target;
    ndk_set_var_t filter;

    ngx_http_php_set_var_data_t *filter_data;

    /*
        value[0] = "php_set_run_code"
        value[1] = target variable name
        value[2] = php code
        value[3..] = real params
    */

    value = cf->args->elts;
    target = value[1];

    filter.type = NDK_SET_VAR_MULTI_VALUE_DATA;
    filter.func = cmd->post;
    filter.size = cf->args->nelts - 3;

    filter_data = ngx_palloc(cf->pool, sizeof(ngx_http_php_set_var_data_t));
    if (filter_data == NULL){
        return NGX_CONF_ERROR;
    }

    filter_data->size = filter.size;
    filter_data->var_name = value[1];
    filter_data->script = value[2];

    filter_data->code = ngx_http_php_code_from_string(cf->pool, &filter_data->script);
    if (filter_data->code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    PHP_EMBED_START_BLOCK(0, NULL);
        zval retval;
        zend_eval_string_ex(filter_data->code->code.string, &retval, "ngx_php run code return", 1 );

        if (Z_TYPE(retval) == IS_TRUE || 
            Z_TYPE(retval) == IS_FALSE ||  
            Z_TYPE(retval) == IS_LONG ||
            Z_TYPE(retval) == IS_DOUBLE ||
            Z_TYPE(retval) == IS_STRING ){

            convert_to_string(&retval);

            filter_data->result.data = ngx_palloc(cf->pool, Z_STRLEN(retval));
            ngx_memcpy(filter_data->result.data, Z_STRVAL(retval), Z_STRLEN(retval));
            filter_data->result.len = Z_STRLEN(retval);
        } else {
            filter_data->result.data = NULL;
            filter_data->result.len = 0;
        }

        zval_dtor(&retval);
    PHP_EMBED_END_BLOCK();
    
    if (filter_data->result.data != NULL){
        filter_data->code = ngx_http_php_code_from_string(cf->pool, &filter_data->result);
        if (filter_data->code == NGX_CONF_UNSET_PTR){
            return NGX_CONF_ERROR;
        }
    }

    filter.data = filter_data;
    return ndk_set_var_multi_value_core(cf, &target, &value[3], &filter);
}

char *
ngx_http_php_set_file(ngx_conf_t *cf, ngx_command_t *cmd, void *conf){
    ngx_str_t *value;
    ngx_str_t target;
    ndk_set_var_t filter;

    ngx_http_php_set_var_data_t *filter_data;

    /*
        value[0] = "php_set_file"
        value[1] = target variable name
        value[2] = php file
        value[3..] = real params
    */

    value = cf->args->elts;
    target = value[1];

    filter.type = NDK_SET_VAR_MULTI_VALUE_DATA;
    filter.func = cmd->post;
    filter.size = cf->args->nelts - 2;

    filter_data = ngx_palloc(cf->pool, sizeof(ngx_http_php_set_var_data_t));
    if (filter_data == NULL){
        return NGX_CONF_ERROR;
    }

    filter_data->size = filter.size;
    filter_data->var_name = value[1];
    filter_data->script = value[2];

    filter_data->code = ngx_http_php_code_from_file(cf->pool, &filter_data->script);
    if (filter_data->code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    filter.data = filter_data;

    return ndk_set_var_multi_value_core(cf, &target, &value[2], &filter);
}

char *
ngx_http_php_set_run_file(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t *value;
    ngx_str_t target;
    ndk_set_var_t filter;

    ngx_http_php_set_var_data_t *filter_data;

    /*
        value[0] = "php_set_file"
        value[1] = target variable name
        value[2] = php file
        value[3..] = real params
    */

    value = cf->args->elts;
    target = value[1];

    filter.type = NDK_SET_VAR_MULTI_VALUE_DATA;
    filter.func = cmd->post;
    filter.size = cf->args->nelts - 2;

    filter_data = ngx_palloc(cf->pool, sizeof(ngx_http_php_set_var_data_t));
    if (filter_data == NULL){
        return NGX_CONF_ERROR;
    }

    filter_data->size = filter.size;
    filter_data->var_name = value[1];
    filter_data->script = value[2];

    filter_data->code = ngx_http_php_code_from_file(cf->pool, &filter_data->script);
    if (filter_data->code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    PHP_EMBED_START_BLOCK(0, NULL);
        zval *retval;
        zend_file_handle file_handle;

        retval = NULL;
#if PHP_MAJOR_VERSION < 8 || (PHP_MAJOR_VERSION == 8 && PHP_MINOR_VERSION < 1) 
        file_handle.type = ZEND_HANDLE_FP;
        file_handle.opened_path = NULL;    
        file_handle.free_filename = 0;
        file_handle.filename = filter_data->code->code.file;
        if (!(file_handle.handle.fp = VCWD_FOPEN(file_handle.filename, "rb"))) {
            php_printf("Could not open input file: %s\n", file_handle.filename);
            return NGX_CONF_ERROR;
        }
        php_execute_simple_script(&file_handle, retval );
#else
        zend_stream_init_filename(&file_handle, code->code.file);
        if (php_execute_simple_script(&file_handle, retval ) == FAILURE) {
            php_printf("Failed to execute PHP script.\n");
            return NGX_CONF_ERROR;
        }
#endif

        if (Z_TYPE_P(retval) == IS_TRUE || 
            Z_TYPE_P(retval) == IS_FALSE || 
            Z_TYPE_P(retval) == IS_LONG ||
            Z_TYPE_P(retval) == IS_DOUBLE ||
            Z_TYPE_P(retval) == IS_STRING ){

            convert_to_string(retval);

            filter_data->result.data = ngx_palloc(cf->pool, Z_STRLEN_P(retval));
            ngx_memcpy(filter_data->result.data, Z_STRVAL_P(retval), Z_STRLEN_P(retval));
            filter_data->result.len = Z_STRLEN_P(retval);
        } else {
            filter_data->result.data = NULL;
            filter_data->result.len = 0;
        }

        zval_dtor(retval);
    PHP_EMBED_END_BLOCK();
    
    if (filter_data->result.data != NULL){
        filter_data->code = ngx_http_php_code_from_string(cf->pool, &filter_data->result);
        if (filter_data->code == NGX_CONF_UNSET_PTR){
            return NGX_CONF_ERROR;
        }
    }

    filter.data = filter_data;

    return ndk_set_var_multi_value_core(cf, &target, &value[2], &filter);
}

#endif

char *
ngx_http_php_header_filter_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->header_filter_handler != NULL) {
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_string(cf->pool, &value[1]);
    if (code == NGX_CONF_UNSET_PTR) {
        return NGX_CONF_ERROR;
    }

    plcf->header_filter_inline_code = code;
    plcf->header_filter_handler = cmd->post;
    pmcf->enabled_header_filter = 1;

    return NGX_CONF_OK;
}

char *
ngx_http_php_body_filter_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_php_main_conf_t *pmcf;
    ngx_http_php_loc_conf_t *plcf;
    ngx_str_t *value;
    ngx_http_php_code_t *code;

    if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }

    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);
    plcf = conf;

    if (plcf->body_filter_handler != NULL) {
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_php_code_from_string(cf->pool, &value[1]);
    if (code == NGX_CONF_UNSET_PTR) {
        return NGX_CONF_ERROR;
    }

    plcf->body_filter_inline_code = code;
    plcf->body_filter_handler = cmd->post;
    pmcf->enabled_body_filter = 1;

    return NGX_CONF_OK;
}

char *
ngx_http_php_conf_keepalive(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t *value = cf->args->elts;
    ngx_http_php_srv_conf_t *pscf = conf;
    ngx_int_t keepalive_size;

    keepalive_size = ngx_atoi(value[1].data, value[1].len);
    if (keepalive_size == NGX_ERROR) {
        return "is duplicated";
    }
    
    if (pscf->keepalive_conf) {
        pscf->keepalive_conf->max_cached = (ngx_uint_t)keepalive_size;
    }

    ngx_http_php_keepalive_init(cf->pool, pscf->keepalive_conf);

    return NGX_CONF_OK;
}

char *
ngx_http_php_conf_socket_keepalive(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t *value = cf->args->elts;
    ngx_http_php_srv_conf_t *pscf = conf;
    ngx_int_t keepalive_size;

    keepalive_size = ngx_atoi(value[1].data, value[1].len);
    if (keepalive_size == NGX_ERROR) {
        return "is duplicated";
    }
    
    if (pscf->keepalive_conf) {
        pscf->keepalive_conf->max_cached = (ngx_uint_t)keepalive_size;
    }

    ngx_http_php_keepalive_init(cf->pool, pscf->keepalive_conf);

    return NGX_CONF_OK;
}

