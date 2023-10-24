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
/* Actions processor header file 					  */
/*                                                                        */
/**************************************************************************/

/* prototypes definition */
void process_actions();
void send_event2ap(Packet &apack, unsigned short atype, unsigned long pl1, unsigned long pl2, unsigned long pl3, unsigned long pl4, unsigned long date, char *str11);
int  recv_event2ap(Packet &apack, unsigned short &atype, unsigned long &pl1, unsigned long &pl2, unsigned long &pl3, unsigned long &pl4, unsigned long &date, char *str11);
void execute_rule(struct rule_node *rule, unsigned short atype, unsigned long pl1, unsigned long pl2, unsigned long pl3, unsigned long pl4, unsigned long date, char *par11);

void execute_rule_msg(struct rule_node *rule, unsigned short atype, unsigned long pl1, unsigned long pl2, unsigned long pl3, unsigned long pl4, unsigned long date, char *par11);
void execute_rule_run(struct rule_node *rule, unsigned short atype, unsigned long pl1, unsigned long pl2, unsigned long pl3, unsigned long pl4, unsigned long date, char *par11);
void execute_rule_log(struct rule_node *rule, unsigned short atype, unsigned long pl1, unsigned long pl2, unsigned long pl3, unsigned long pl4, unsigned long date, char *par11);
void execute_rule_mail(struct rule_node *rule, unsigned short atype, unsigned long pl1, unsigned long pl2, unsigned long pl3, unsigned long pl4, unsigned long date, char *par11);

char *action2string(int action);
struct template_record *load_template(char *tname);
char *execute_template_subst(template_record *ptt, struct rule_node *rule, unsigned short atype, unsigned long pl1, unsigned long pl2, unsigned long pl3, unsigned long pl4, unsigned long date, char *par11);
