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
/* This file contain structure declarations to build parse  		  */
/* tree on actions configuration file 					  */
/*                                                                        */
/**************************************************************************/

#ifndef _PARSE_TREE_H
#define _PARSE_TREE_H

#include "y.tab.h"

#define CONFIG_TYPE_ACTIONS	1
#define CONFIG_TYPE_AIM		2
#define MAX_UIN_NODES		50
#define UIN_NONE		0
#define MAX_SECTIONS		20
#define MAX_INCLUDE_DEPTH 	5

/* defines for action codes used for substitution */
#define SUBS_NUM		28
#define SUBS_DIRECT		15
#define SUBS_USERS		9
#define SUBS_REG		4

#define SUBS_UIN		0
#define SUBS_IP			1
#define SUBS_ERRCODE		2
#define SUBS_ATYPE		3
#define SUBS_CSTATUS		4
#define SUBS_NSTATUS		5
#define SUBS_SRCHCODE		6
#define SUBS_ADATE		7
#define SUBS_OPTSTRING		8
#define SUBS_ISDVER		9
#define SUBS_COMPILER		10
#define SUBS_OS			11
#define SUBS_ONLNUSERS		12
#define SUBS_SYSNAME		13
#define SUBS_STARTTIME		14

#define SUBS_DFNAME		15
#define SUBS_DLNAME		16
#define SUBS_DNNAME		17
#define SUBS_DEMAIL1		18
#define SUBS_DEMAIL2		19
#define SUBS_DEMAIL3		20
#define SUBS_BDATE		21
#define SUBS_CDATE		22
#define SUBS_LLDATE		23

#define SUBS_RFNAME		24
#define SUBS_RLNAME		25
#define SUBS_RNNAME		26
#define SUBS_REMAIL		27

/* some defines for section codes and other stuff */
#define ACT_ONLINE		0
#define ACT_OFFLINE		1
#define ACT_SAVEBASIC		2
#define ACT_SEARCH		3
#define ACT_STATUS		4
#define ACT_REGISTR		5
#define ACT_RDBMSFAIL  		6
#define ACT_RDBMSERR		7
#define ACT_INTERR		8
#define ACT_PPHUNG		9

/* actions codes */
#define A_NONE	  		0
#define A_MSG     		1
#define A_LOG	  		2
#define A_STOP	  		3
#define A_MAIL	  		4
#define A_RUN	  		5

/* variable codes */
#define VAR_STR	  		0
#define VAR_NUM   		1
#define VAR_LST   		2


/* Single record for uin or uin range */
#ifndef _UIN_RECORD
#define _UIN_RECORD
typedef struct uin_record
{
   unsigned long uin_1;
   unsigned long uin_2;

} uin_record;
#endif /* _UIN_RECORD */


/* struct that contain all uins       */
/* and uin ranges from single rule    */
#ifndef _UIN_NODE
#define _UIN_NODE
typedef struct uin_node
{
   unsigned short count;
   struct uin_record node[MAX_UIN_NODES];
   
} uin_node;
#endif /* _UIN_NODE */


/* structure for one rule node. Rule nodes for  */
/* every section are placed in dual linked list */
#ifndef _RULE_NODE
#define _RULE_NODE
typedef struct rule_node
{
  struct uin_node *for_uins;		/* clause uin list      */
  struct uin_node *tgt_uins;		/* target uin list      */
  
  unsigned short action;		/* action code          */
  unsigned short astop;			/* stop processing flag */
  unsigned short enabled;		/* rule enabled flag    */
  unsigned long  status;		/* clause status value  */
  
  char   *string_1;			/* string parameter #1  */
  char   *string_2;			/* string parameter #2  */
  char   *string_3;			/* string parameter #3  */
  
  struct rule_node *prev;		/* pointer for previous rule */
  struct rule_node *next;		/* pointer for next rule     */

} rule_node;
#endif /* _RULE_NODE */

/* single record for defined variable   */
#ifndef _VAR_RECORD
#define _VAR_RECORD
typedef struct variable_record
{
  int type;
  char name[128];
  char str_val[255];
  unsigned long num_val;
  struct uin_node *uins;
  
  struct variable_record *next;
  struct variable_record *prev;

} variable_record;
#endif /* _VAR_RECORD */

/* template cache record */
#ifndef _TEMPLATE_RECORD
#define _TEMPLATE_RECORD
typedef struct template_record
{
  fstring filename;
  char *value;
  int  size;
  BOOL db_lookup_needed;
  BOOL rg_lookup_needed;
  char smatrix[SUBS_NUM];
  
  struct template_record *next;
  struct template_record *prev;

} template_record;
#endif /* _TEMPLATE_RECORD */


/* This is a root structure for actions */
/* config parser. It contain links to   */
/* all action lists      	        */
#ifndef _RULES_ROOT
#define _RULES_ROOT
typedef struct _rules_root
{
   struct rule_node *sections_root[MAX_SECTIONS];
      
} _rules_root;
#endif /* _RULES_ROOT */


/**************************************************************/
/* aim config parser definitions starts from here...          */
/**************************************************************/

#ifndef _PARAM_LIST
#define _PARAM_LIST
typedef struct param_list
{
  char *name;
  unsigned short value;
  struct param_list *next;
  struct param_list *prev;

} param_list;

#endif /* _PARAM_LIST */

#ifndef _RATE_CLASS
#define _RATE_CLASS
typedef struct rate_class
{
  int rate_index;
  
  struct param_list *plist;
  struct rate_class *next;
  struct rate_class *prev;

} rate_class;

#endif /* _RATE_CLASS */

#ifndef _SUBTYPE
#define _SUBTYPE
typedef struct subtype
{
  unsigned short num;
  unsigned short rate_ind;
  struct subtype *next;
  struct subtype *prev;

} subtype;
#endif /* _SUBTYPE */

#ifndef _AIM_FAMILY
#define _AIM_FAMILY
typedef struct aim_family
{
  unsigned short number;
  unsigned short version;
  
  struct subtype    *subtypes;
  struct aim_family *next;
  struct aim_family *prev;

} aim_family;
#endif /* _AIM_FAMILY */

#ifndef _AIM_ROOT
#define _AIM_ROOT
typedef struct _aim_root
{
   struct aim_family *aim_families;
   struct rate_class *rate_classes;

} _aim_root;
#endif /* _AIM_ROOT */

#endif /* _PARSE_TREE_H */
