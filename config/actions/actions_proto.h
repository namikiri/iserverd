/**************************************************************************/
/*									  */
/* Copyright (c) 2000-2005 by Alexandr V. Shutko, Khabarovsk, Russia	  */
/* All rights reserved.							  */
/*                                                                        */
/* Redistribution and use in source and binary forms, with or without	  */
/* modification, are permitted provided that the following conditions	  */
/* are met:								  */
/* 1. Redistributions of source code must retain the above copyright	  */
/*    notice, this list of conditions and the following disclaimer.	  */
/* 2. Redistributions in binary form must reproduce the above copyright	  */
/*    notice, this list of conditions and the following disclaimer in 	  */
/*    the documentation and/or other materials provided with the 	  */
/*    distribution.							  */
/*                                                                        */
/* THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND */
/* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE  */
/* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 	  */
/* PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS  */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 	  */
/* OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT   */
/* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 	  */
/* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,  */
/* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE   */
/* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 	  */
/* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.			  */
/*									  */
/* This file contain actions config parser function prototypes		  */
/*                                                                        */
/**************************************************************************/

#ifndef _ACTIONS_PROTO_H
#define _ACTIONS_PROTO_H

#ifdef __cplusplus
extern "C" {
#endif

struct uin_node *create_uin_node();
void delete_uin_node(struct uin_node *node);
int  add_uin2node(struct uin_node *rnode, unsigned long uin_1, unsigned long uin_2);
int  is_uin_match(unsigned long uin, struct uin_node *rnode);
void raw_add_rule_to_list(int section, struct rule_node *r_new);
void add_rule_to_list(int section, unsigned short action, char *par1, char *par2, char *par3, struct uin_node *for_node, struct uin_node *tgt_node, unsigned short rstatus, int astop);
void delete_node_list(struct rule_node *r_begin);
void delete_parse_tree();
void print_uin_node(struct uin_node *uin_lst);
void init_parse_tree();
void add_var2list(struct variable_record *variable);
struct variable_record *create_variableA(char *name, char *str_val, unsigned long num_val, int type);
struct variable_record *create_variableB(char *name, struct uin_node *uins);
struct variable_record *variable_lookup(char *name);
void parse_config_file(char *filename2, int file_type);
struct template_record *template_read(char *name);
#ifdef __cplusplus
int calculate_dp_size(struct template_record *ptt, struct full_user_info user, struct full_user_info ruser, BOOL lok, BOOL rlok, unsigned short atype, unsigned long pl1, unsigned long pl2, unsigned long pl3, unsigned long pl4, unsigned long date, char *par11);
#endif
void print_parse_tree();

int calc_rule_number(struct rule_node *r_begin);
int calc_vars_number();
void print_parse_stats();
void log_parse_stats();
void print_matched_rules(unsigned long uin);
void print_subtypes(struct aim_family *fam);
void print_rparam_list(struct param_list *plist);
void print_aim_tree();

void init_aim_tree();
void add_rate_class(unsigned short index);
void add_rate_variable(unsigned short index, char *pname, unsigned short pvalue);
void plist_free(struct param_list *plist);
void add_snac_family(unsigned short fnumber);
void subtypes_list_free(struct subtype *stlist);
void family_set_version(unsigned short fnumber, unsigned short version);
void add_family_subtype(unsigned short fnumber, unsigned short rclass, unsigned short stype_num);
unsigned long rate_check(unsigned short index);
int get_family_version(unsigned short family);
int get_rate_classes_num();
void template_scan(struct template_record *ptt);
unsigned long get_rate_param(unsigned short index, char *pname);
unsigned short get_subtypes_num(unsigned short rate_ind);
struct template_record *template_lookup(char *name);
struct variable_record *aim_create_variable(char *name, char *str_val);
void aim_var2list(struct variable_record *variable);
struct variable_record *aim_get_variable(char *name);

void yyerror(char *s, ... );
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _ACTIONS_PROTO_H */

