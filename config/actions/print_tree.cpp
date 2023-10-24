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
/* This file contain functions to print parse tree and parse stats 	  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

/*************************************************************************/
/* Print uin_list on std.						 */
/*************************************************************************/
void print_uin_node(struct uin_node *uin_lst)
{
   int i;
   
   if (uin_lst == NULL) 
   {
      printf("any ");
      return;
   }
   
   printf("[");
   
   for (i=0; i<uin_lst->count; i++)
   {
      if (i != 0) printf(",");
      
      if ((uin_lst->node[i].uin_1 != 0) && (uin_lst->node[i].uin_2 != 0))
      {
         printf("%lu-%lu", uin_lst->node[i].uin_1, uin_lst->node[i].uin_2);
      }
      
      if ((uin_lst->node[i].uin_1 != 0) && (uin_lst->node[i].uin_2 == 0))
      {
         printf("%lu", uin_lst->node[i].uin_1);
      }

      if ((uin_lst->node[i].uin_1 == 0))
      {
         printf("any");
      }      
   }
   
   printf("] ");
}


/**************************************************************************/
/* print rule on std							  */
/**************************************************************************/
void print_action(int action)
{
   switch(action)
   {
      case A_NONE:	printf("none "); break;
      case A_MSG:       printf("msg "); break;
      case A_LOG:	printf("log "); break;
      case A_STOP:	printf("stop"); break;
      case A_MAIL:	printf("mail "); break;
      case A_RUN:	printf("run "); break;
      default: printf("xxx_rule "); 
   }
}


/**************************************************************************/
/* print rule on std							  */
/**************************************************************************/
void print_status(int rstatus)
{
   switch (rstatus)
   {
      case T_S_ONLINE:		printf("S_ONLINE "); break;
      case T_S_OFFLINE:		printf("S_OFFLINE "); break;
      case T_S_NA:		printf("S_NA "); break;
      case T_S_AWAY:		printf("S_AWAY "); break;
      case T_S_DND:		printf("S_DND "); break;
      case T_S_PRIVACY:		printf("S_PIVATE "); break;
      case T_S_FFCH:		printf("S_FFCH "); break;
      case T_S_OCCUPIED:	printf("S_OCCUPIED ");break;
      
      case T_ANY_LEX:
      case 0:			printf("S_ANY "); break;
      default: 			printf("S_ANY (%d)", rstatus);      
   }
}

/**************************************************************************/
/* print rule on std							  */
/**************************************************************************/
void print_options(struct rule_node *rule)
{
   switch(rule->action)
   {
      case A_MSG:      print_uin_node(rule->tgt_uins);
		       printf("\"%s\"", rule->string_1);
		       break;
      case A_LOG:      printf("\"%s\"", rule->string_1);
                       break;
      case A_NONE:
      case A_STOP:     printf(" ");
                       break;
      case A_MAIL:     printf("\"%s\" \"%s\"", rule->string_1, rule->string_2);
                       break;
      case A_RUN:      printf("\"%s\" with \"%s\"", rule->string_1, rule->string_2);
                       break;
      default:	       printf(" ");
   }
}


/**************************************************************************/
/* print rule from begin 						  */
/**************************************************************************/
void print_rule(struct rule_node *rule)
{
   if (rule != NULL)
   {
      if (rule->for_uins != NULL)
      {
         printf("   for ");
         print_uin_node(rule->for_uins);
      
      }
      
      if ((rule->status != T_ANY_LEX) && (rule->status != 0))
      {
         if (rule->for_uins == NULL)
	 {
	    printf("   for any ");
	 }
	 
         printf("with ");
         print_status(rule->status);
      }
      else
      {
         if (rule->for_uins == NULL)
	 {
	    printf("   ");
	 }
      }
      
      print_action(rule->action);
      print_options(rule);
      
      if (rule->astop == 0)
      {
         printf(";\n");
      }
      else
      {
         printf(" stop;\n");
      }
   }
}


/**************************************************************************/
/* print rule chain from begin 						  */
/**************************************************************************/
void print_rule_chain(struct rule_node *r_begin)
{
   struct rule_node *r_current = r_begin;
   while (r_current != NULL)
   {
      print_rule(r_current);
      r_current = r_current->next;
   }
}


/**************************************************************************/
/* print rule chain from begin 						  */
/**************************************************************************/
void print_section(int section)
{
   switch (section)
   {
      case ACT_ONLINE:     printf("event Online {\n"); break;                           
      case ACT_OFFLINE:    printf("event Offline {\n"); break;
      case ACT_SAVEBASIC:  printf("event SaveBasicInfo {\n"); break;
      case ACT_SEARCH:     printf("event Search {\n"); break;
      case ACT_STATUS:     printf("event StatusChange {\n"); break;
      case ACT_REGISTR:    printf("event Registration {\n"); break;
      case ACT_RDBMSFAIL:  printf("event RDBMS_fail {\n"); break;
      case ACT_RDBMSERR:   printf("event RDBMS_error {\n"); break;
      case ACT_INTERR:     printf("event Internal_err {\n"); break;
      case ACT_PPHUNG:     printf("event PPHung {\n"); break;
   }
   
   print_rule_chain(rules_root.sections_root[section]);
   printf("};\n\n");
}


/**************************************************************************/
/* print section prefix 						  */
/**************************************************************************/
void print_section_prefix(int section)
{
   switch (section)
   {
      case ACT_ONLINE:     printf("evt Oln: "); break;                           
      case ACT_OFFLINE:    printf("evt Off: "); break;
      case ACT_SAVEBASIC:  printf("evt SvB: "); break;
      case ACT_SEARCH:     printf("evt Sch: "); break;
      case ACT_STATUS:     printf("evt Sts: "); break;
      case ACT_REGISTR:    printf("evt Reg: "); break;
      case ACT_RDBMSFAIL:  printf("evt DFl: "); break;
      case ACT_RDBMSERR:   printf("evt DEr: "); break;
      case ACT_INTERR:     printf("evt IEr: "); break;
      case ACT_PPHUNG:     printf("evt PPh: "); break;
   }
}


/**************************************************************************/
/* print parse tree	 						  */
/**************************************************************************/
void print_parse_tree()
{
   for (int i=0; i<MAX_SECTIONS; i++)
   {
      if (rules_root.sections_root[i] != NULL)
      {
         print_section(i);
      }
   }
}


/**************************************************************************/
/* calculate number of rules in single chain				  */
/**************************************************************************/
int calc_rule_number(struct rule_node *r_begin)
{
   struct rule_node *r_current = r_begin;
   int counter = 0;
   
   while (r_current != NULL)
   {
      counter++;
      r_current = r_current->next;
   }
   
   return (counter);
}


/**************************************************************************/
/* calculate number of rules in single chain				  */
/**************************************************************************/
int calc_vars_number()
{
   struct variable_record *var_current = var_list;
   int counter = 0;
   
   while (var_current != NULL)
   {
      counter++;
      var_current = var_current->next;
   }
   
   return (counter);
}


/**************************************************************************/
/* print parse stats (number of sections, number of rules, num of vars)   */
/**************************************************************************/
void print_parse_stats()
{
   int vars_cnt = 0;
   int rule_cnt = 0;
   int sect_cnt = 0;

   for (int i=0; i<MAX_SECTIONS; i++)
   {
      if (rules_root.sections_root[i] != NULL)
      {
         sect_cnt++;
         rule_cnt += calc_rule_number (rules_root.sections_root[i]);
      }
   }
   
   
   vars_cnt = calc_vars_number();
   
   printf("Parser stats: %d sections, %d rules, %d variables.\n", 
          sect_cnt, rule_cnt, vars_cnt);
}

/**************************************************************************/
/* log parse stats (number of sections, number of rules, num of vars)     */
/**************************************************************************/
void log_parse_stats()
{
   int vars_cnt = 0;
   int rule_cnt = 0;
   int sect_cnt = 0;

   for (int i=0; i<MAX_SECTIONS; i++)
   {
      if (rules_root.sections_root[i] != NULL)
      {
         sect_cnt++;
         rule_cnt += calc_rule_number (rules_root.sections_root[i]);
      }
   }
   
   
   vars_cnt = calc_vars_number();
   
   LOG_SYS(0, ("Init: AP found sections: %d, rules: %d, variables: %d\n", 
               sect_cnt, rule_cnt, vars_cnt));
}



/**************************************************************************/
/* print rules, that match given uin number and status			  */
/**************************************************************************/
void print_matched_rules(unsigned long uin)
{
   struct rule_node *r_current;
   
   for (int i=0; i<MAX_SECTIONS; i++)
   {
      if (rules_root.sections_root[i] != NULL)
      {
         r_current = rules_root.sections_root[i];
	 while (r_current != NULL)
	 {
	    if (is_uin_match(uin, r_current->for_uins) == 1)
	    {
	       print_section_prefix(i);
	       print_rule(r_current);
	    }
	    r_current = r_current->next;
	 }

      }
   }
   
   printf("\n");
}


/**************************************************************************/
/* print subtypes list   						  */
/**************************************************************************/
void print_subtypes(struct aim_family *fam)
{
   struct subtype *stypes = fam->subtypes;
   
   while (stypes)
   {
      printf("   subtype %d (rate %d)\n", stypes->num, stypes->rate_ind);
      stypes = stypes->next;
   }
}


/**************************************************************************/
/* print rate_class parameters list					  */
/**************************************************************************/
void print_rparam_list(struct param_list *plist)
{
   struct param_list *tplist = plist;
   
   while (tplist)
   {
      printf("   %s = %d\n", tplist->name, tplist->value);
      tplist = tplist->next;
   }
}


/**************************************************************************/
/* print aim config tree						  */
/**************************************************************************/
void print_aim_tree()
{
   struct rate_class *rcss = aim_root.rate_classes;
   struct aim_family *amfm = aim_root.aim_families;

   /* Stage one - print rate classes */
   while (rcss)
   {
      printf("rate class %d\n{\n", rcss->rate_index);
      print_rparam_list(rcss->plist);
      printf("}\n\n");
      rcss = rcss->next;
   }
   
   while (amfm)
   {
      printf("snac family %d\n{\n   version = %d\n\n", amfm->number, amfm->version);
      print_subtypes(amfm);
      printf("}\n\n");
      amfm = amfm->next;
   }
}


