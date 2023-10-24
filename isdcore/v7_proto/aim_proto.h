/**************************************************************************/
/*									  */
/* Copyright (c) 2000-2005 by Alexandr V. Shutko, Khabarovsk, Russia	  */
/* All rights reserved.							  */
/*									  */
/* Redistribution and use in source and binary forms, with or without	  */
/* modification, are permitted provided that the following conditions	  */
/* are met:								  */
/* 1. Redistributions of source code must retain the above copyright	  */
/*    notice, this list of conditions and the following disclaimer.	  */
/* 2. Redistributions in binary form must reproduce the above copyright	  */
/*    notice, this list of conditions and the following disclaimer in 	  */
/*    the documentation and/or other materials provided with the 	  */
/*    distribution.							  */
/*									  */
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
/* Defines for AIM/V7 protocols (protocol configuration structs)  	  */
/*                                                                        */
/**************************************************************************/

#ifndef _AIM_PROTO_H
#define _AIM_PROTO_H

/* variable structure that can be linked in list */
typedef struct aim_variable
{
   char *name;
   char *svalue;
   unsigned long lvalue;
   struct *aim_variable next;

} aim_variable;

/* rate limit class description */
typedef struct rate_class_description
{
   unsigned short index;
   struct *aim_variable var_list;   
   struct *rate_class_description next;
   
} rate_class_description;

/* every subtype have related rate class number, 0 mean no-class */
typedef struct snac_subtype
{
   unsigned short subtype;
   unsigned short rate_class;
   struct *snac_subtype next;
   
} snac_subtype;

/* aim service root structure */
typedef struct snac_family
{
   unsigned short type;
   unsigned short version;
   struct *aim_variable var_list;
   struct *snac_subtype subtype;
   struct *snac_family next;

} snac_family;

/* aim protocol configuration structure */
typedef struct aim_proto_data
{
   struct *rate_class_description rate_classes;
   struct *snac_family services;

} aim_proto_data;

#endif /* _AIM_PROTO_H */
