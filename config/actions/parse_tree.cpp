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
/* This file contain functions to build parse  				  */
/* tree on actions configuration file 					  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

/*************************************************************************/
/* Create and init new uin node for rule.				 */
/*************************************************************************/
struct uin_node *create_uin_node()
{
   uin_node *rnode = new uin_node;
   rnode->count = 0;
   
   return (rnode);
}


/*************************************************************************/
/* delete uin_node							 */
/*************************************************************************/
void delete_uin_node(struct uin_node *node)
{
   delete node;
}


/*************************************************************************/
/* Add new uin or uin range to node.					 */
/*************************************************************************/
int add_uin2node(struct uin_node *rnode, unsigned long uin_1, 
					 unsigned long uin_2)
{
   /* try to add uin or range to node */
   if (rnode->count < MAX_UIN_NODES)
   {
      rnode->node[rnode->count].uin_1 = uin_1;
      rnode->node[rnode->count].uin_2 = uin_2;
      rnode->count++; 
      
      return(0);
   }
   else
   {
      /* node array overflowed */
      return (-1);   
   }
}


/*************************************************************************/
/* Check if given uin match rule clause.				 */
/*************************************************************************/
int is_uin_match(unsigned long uin, struct uin_node *rnode)
{
   int i;
   
   if (rnode != NULL)
   {
      for (i=0; i < rnode->count; i++)
      {
         if ((rnode->node[i].uin_1 > 0) && 
	     (rnode->node[i].uin_2 > 0))
	 {
	    /* here we compare uin against uin range */
	    if ((uin >= rnode->node[i].uin_1) &&
	        (uin <= rnode->node[i].uin_2))
	    {
	       /* given uin match clause */
	       return(1);
	    }
	    else
	    {
	       /* given uin is out of range */
	    }
	 }
	 
	 if ((rnode->node[i].uin_1 >  0) &&
	     (rnode->node[i].uin_2 == 0))
	 {
	    /* here we compare uin against single number */
	    if (uin == rnode->node[i].uin_1) 
	    {
	       return(1);
	    }
	 }
	 
	 if ((rnode->node[i].uin_1 == 0) &&
	     (rnode->node[i].uin_2 == 0))
	 {
	    /* well, one record in node contain "any" clause */
	    return(1);
	 }
      }
      
      /* well... it seems to me uin doesn't match clause */
      return (0);
   }
  
   /* uin list doesn't exist, so assume "any" clause */
   return(1);  
}


/*************************************************************************/
/* Add already created rule to list in proper section.			 */
/*************************************************************************/
void raw_add_rule_to_list(int section, struct rule_node *r_new)
{
   struct rule_node *r_prev, *r_current;
   r_current = rules_root.sections_root[section];
   
   
   if (rules_root.sections_root[section] != NULL)
   {
      /* loop from begin to end of dual linked list */
      while (r_current != NULL)
      {
         r_prev = r_current;
         r_current = r_current->next;
      }

      /* insert at the end given record */   
      r_prev->next = r_new;
      r_new->prev  = r_prev;
      r_new->next  = NULL;
   }
   else
   {
      /* well, there is no records in list */
      rules_root.sections_root[section] = r_new;
      rules_root.sections_root[section]->next = NULL;
      rules_root.sections_root[section]->prev = NULL;
   }
}


/*************************************************************************/
/* Create/fill and add rule to list in proper section.			 */
/*************************************************************************/
void add_rule_to_list(int section, unsigned short action, char *par1, 
		      char *par2, char *par3, struct uin_node *for_node, 
		      struct uin_node *tgt_node, unsigned short rstatus, 
		      int astop)
{
   /* create empty rule node structure */
   struct rule_node *r_new = new rule_node;

   /* alloc memory for param strings */
   string_init(&(r_new->string_1), par1);
   string_init(&(r_new->string_2), par2);
   string_init(&(r_new->string_3), par3);    
   r_new->status  = rstatus;
   
   r_new->action  = action;
   r_new->astop	  = astop;
   r_new->enabled = True;
   
   /* fill in rule node parameters */
   r_new->for_uins = for_node;
   r_new->tgt_uins = tgt_node;

   if (section > MAX_SECTIONS) 
   {
      yyerror("[Internal error] section number out of range.");
      string_free(&(r_new->string_1));
      string_free(&(r_new->string_2));
      string_free(&(r_new->string_3));
      delete r_new->for_uins;
      delete r_new->tgt_uins;
      delete r_new;
      return; 
   }
   
   /* switch on section variable and insert it to list */
   raw_add_rule_to_list(section, r_new);
}


/*************************************************************************/
/* Delete rules from single root node.					 */
/*************************************************************************/
void delete_node_list(struct rule_node *r_begin)
{
   struct rule_node *r_next, *r_current;
   r_current = r_begin;
   r_begin   = NULL;
   
   while(r_current != NULL)
   {
      r_next = r_current->next;
      free(r_current->string_1);
      free(r_current->string_2);
      free(r_current->string_3);
      delete r_current->for_uins;
      delete r_current->tgt_uins;
      delete r_current;
      r_current = r_next;
   }
}


/*************************************************************************/
/* Delete all rules.							 */
/*************************************************************************/
void delete_parse_tree()
{
   struct variable_record *var_current = var_list;
   struct variable_record *var_deleted;
   
   for (int i=0; i<MAX_SECTIONS; i++)
   {
      if (rules_root.sections_root[i] != NULL)
      {
         delete_node_list(rules_root.sections_root[i]);
         rules_root.sections_root[i] = NULL;
      }
   }

   while (var_current != NULL)
   {
      var_deleted = var_current;
      var_current = var_current->next;
      delete var_deleted;
   }
   
   var_list = NULL;
}


/*************************************************************************/
/* Init parser node tree.						 */
/*************************************************************************/
void init_parse_tree()
{
   for (int i=0; i<MAX_SECTIONS; i++)
   {
      rules_root.sections_root[i] = NULL;
   }
}


/*************************************************************************/
/* Add aim variable to list.						 */
/*************************************************************************/
void aim_var2list(struct variable_record *variable)
{
   struct variable_record *var_current;
   var_current = aim_list;

   /* check if list is empty */
   if (aim_list == NULL)
   {
      aim_list       = variable;
      aim_list->next = NULL;
      aim_list->prev = NULL;
      return;
   }
      
   /* insrt new variable to the begin of the list */
   aim_list          = variable;
   aim_list->next    = var_current;
   aim_list->prev    = NULL;   
   var_current->prev = aim_list;
   
}


/*************************************************************************/
/* Add variable to list.						 */
/*************************************************************************/
void add_var2list(struct variable_record *variable)
{
   struct variable_record *var_current;

   var_current = var_list;

   /* check if list is empty */
   if (var_list == NULL)
   {
      var_list = variable;
      var_list->next = NULL;
      var_list->prev = NULL;
      return;
   }
      
   /* insrt new variable to the begin of the list */
   var_list = variable;
   var_list->next = var_current;
   var_list->prev = NULL;   
   var_current->prev = var_list;
   
}


/*************************************************************************/
/* Create new aim str variable.						 */
/*************************************************************************/
struct variable_record *aim_create_variable(char *name, char *str_val)
{
   struct variable_record *new_var = new variable_record;
   
   strncpy(new_var->name, name, 255);
   strncpy(new_var->str_val, str_val, 255);
   
   new_var->type    = VAR_STR;
   new_var->num_val = 0;
   
   return (new_var);
}


/*************************************************************************/
/* Create new num/str variable.						 */
/*************************************************************************/
struct variable_record *create_variableA(char *name, char *str_val, 
					unsigned long num_val, int type)
{
   struct variable_record *new_var = new variable_record;
   
   strncpy(new_var->name, name, 255);
   strncpy(new_var->str_val, str_val, 255);
   
   new_var->type    = type;
   new_var->num_val = num_val;
   
   return (new_var);
}


/*************************************************************************/
/* Create new uins_list variable.					 */
/*************************************************************************/
struct variable_record *create_variableB(char *name, struct uin_node *uins)
{
   struct variable_record *new_var = new variable_record;
   
   strncpy(new_var->name, name, 255);
   strncpy(new_var->str_val, "", 255);
   
   new_var->type    = VAR_LST;
   new_var->num_val = 0;
   new_var->uins    = uins;
   
   return (new_var);
}


/*************************************************************************/
/* Lookup template by its filename.					 */
/*************************************************************************/
int calculate_dp_size(struct template_record *ptt, struct full_user_info user, 
                      struct full_user_info ruser, BOOL lok, BOOL rlok, 
                      unsigned short atype, unsigned long pl1, unsigned long pl2, 
                      unsigned long pl3, unsigned long pl4, unsigned long date, 
		      char *par11)
{
   int bufsize = ptt->size;
   char *s = ptt->smatrix;
   char bdate[32];
   
   for (int i=0; i<SUBS_NUM; i++) bufsize -= ptt->smatrix[i] * 5;

   /* now add size of all values */
   /* (!) adding new code don't forget to add it to subs & template_scan func */

   if (s[SUBS_UIN]) 
      bufsize += (strlen(pl1 > 1000 ? n2str(pl1) : "<n/a>") * s[SUBS_UIN]);
      
   if (s[SUBS_IP]) 
      bufsize += (strlen(pl3 > 0 ? inet_ntoa(icqToIp(pl3)) : "<n/a>") * s[SUBS_IP]);
   
   if (s[SUBS_ATYPE]) bufsize += (strlen(action2string(atype)) * s[SUBS_ATYPE]);
   if (s[SUBS_ADATE]) bufsize += (strlen(time2str1(date)) * s[SUBS_ADATE]);
   if (s[SUBS_CSTATUS]) bufsize += (strlen(n2str(pl2)) * s[SUBS_CSTATUS]);
   
   if (s[SUBS_NSTATUS]) 
      bufsize += (strlen(atype == ACT_STATUS ? n2str(pl4) : "<n/a>") * s[SUBS_NSTATUS]);
      
   if (s[SUBS_ERRCODE]) 
      bufsize += (strlen(atype == ACT_RDBMSERR ? n2str(pl1) : "<n/a>") * s[SUBS_ERRCODE]);
      
   if (s[SUBS_SRCHCODE]) 
      bufsize += (strlen(atype == ACT_SEARCH ? n2str(pl4) : "<n/a>") * s[SUBS_SRCHCODE]);
   
   if (s[SUBS_OPTSTRING]) bufsize += (strlen(par11) * s[SUBS_OPTSTRING]);
   if (s[SUBS_ISDVER]) bufsize += (strlen(Iversion) * s[SUBS_ISDVER]);
   if (s[SUBS_COMPILER]) bufsize += (strlen(COMPILER_NAME) * s[SUBS_COMPILER]);
   if (s[SUBS_OS]) bufsize += (strlen(SYSTEM_UNAME) * s[SUBS_OS]);
   
   if (s[SUBS_ONLNUSERS]) 
      bufsize += (strlen(n2str(ipc_vars->online_usr_num)) * s[SUBS_ONLNUSERS]);
      
   if (s[SUBS_SYSNAME]) bufsize += (strlen(SYSTEM_NAME) * s[SUBS_SYSNAME]);
   
   if (s[SUBS_STARTTIME]) 
      bufsize += (strlen(time2str1(server_started)) * s[SUBS_STARTTIME]);
   /* ------------------------------ */
   
   if (s[SUBS_DFNAME]) 
       bufsize += (strlen(lok == True ? user.first : "<n/a>") * s[SUBS_DFNAME]);
       
   if (s[SUBS_DLNAME]) 
       bufsize += (strlen(lok == True ? user.last : "<n/a>") * s[SUBS_DLNAME]);
       
   if (s[SUBS_DNNAME]) 
       bufsize += (strlen(lok == True ? user.nick : "<n/a>") * s[SUBS_DNNAME]);
       
   if (s[SUBS_DEMAIL1]) 
       bufsize += (strlen(lok == True ? user.email1 : "<n/a>") * s[SUBS_DEMAIL1]);
       
   if (s[SUBS_DEMAIL2]) 
       bufsize += (strlen(lok == True ? user.email2 : "<n/a>") * s[SUBS_DEMAIL2]);
       
   if (s[SUBS_DEMAIL3]) 
       bufsize += (strlen(lok == True ? user.email3 : "<n/a>") * s[SUBS_DEMAIL3]);
       
   if (s[SUBS_BDATE])
   {
       snprintf(bdate, 31, "%02d.%02d.%04d", user.bday, user.bmonth, user.byear);
       bufsize += (strlen(lok == True ? bdate : "<n/a>") * s[SUBS_BDATE]);
   }
   
   if (s[SUBS_CDATE]) 
       bufsize += (strlen(lok == True ? time2str1(user.cr_date) : "<n/a>") * s[SUBS_CDATE]);
       
   if (s[SUBS_LLDATE]) 
       bufsize += (strlen(lok == True ? time2str1(user.lastlogin) : "<n/a>") * s[SUBS_LLDATE]);
   /* ------------------------------ */

   if (s[SUBS_RFNAME]) 
       bufsize += (strlen(rlok == True ? ruser.first : "<n/a>") * s[SUBS_RFNAME]);  
       
   if (s[SUBS_RLNAME]) 
       bufsize += (strlen(rlok == True ? ruser.last : "<n/a>") * s[SUBS_RLNAME]);
       
   if (s[SUBS_RNNAME]) 
       bufsize += (strlen(rlok == True ? ruser.nick : "<n/a>") * s[SUBS_RNNAME]);
       
   if (s[SUBS_REMAIL]) 
       bufsize += (strlen(rlok == True ? ruser.email1 : "<n/a>") * s[SUBS_REMAIL]);

   return(bufsize+100);
}


/**************************************************************************/
/* run substitute on template						  */
/**************************************************************************/
char *execute_template_subst(template_record *ptt, struct rule_node *rule, 
			     unsigned short atype, 
                             unsigned long pl1, unsigned long pl2, 
                             unsigned long pl3, unsigned long pl4, 
		             unsigned long date, char *par11)
{
   char *buffer = NULL;
   int bufsize = 0;
   int bsize = 0;
   struct full_user_info user;
   struct full_user_info ruser;
   struct variable_record *variable = NULL;
   BOOL lsuccess = False;
   BOOL rlsuccess = False;
   int lookup_enabled = True;
   char *sm = ptt->smatrix;
   char bdate[32];

   variable = variable_lookup("ENABLE_DB_LOOKUPS");
   if (variable != NULL) lookup_enabled = (int)variable->num_val;

   /* check for users info data */
   if ((ptt->db_lookup_needed) && (pl1 > 1000) && lookup_enabled)
   {
      if (db_users_lookup(pl1, user) == 0) lsuccess = True;
      if (lsuccess) { DEBUG(50, ("Userinfo lookup success...\n")); }
   }

   /* check for registration info data */
   if ((ptt->rg_lookup_needed) && (atype == ACT_REGISTR) && lookup_enabled)
   {
      if (db_new_lookup(pl1, ruser) == 0) rlsuccess = True;
      if (rlsuccess) { DEBUG(50, ("Reginfo lookup success...\n")); }
   }
   
   bufsize = calculate_dp_size(ptt, user, ruser, lsuccess, rlsuccess, 
                               atype, pl1, pl2, pl3, pl4, date, par11);
			       
   buffer = (char*)malloc(bufsize);
   bsize = bufsize-1;
   memcpy(buffer, ptt->value, ptt->size);

   /* now we have all data to make substitute */
   /* (!) adding new code don't forget to add it to calc_dp & template_scan func */

   if (sm[SUBS_UIN]) 
      string_sub(buffer, "%uin%", pl1 > 1000 ? n2str(pl1) : "<n/a>", bsize);
      
   if (sm[SUBS_IP]) 
      string_sub(buffer, "%ips%", pl3 > 0 ? inet_ntoa(icqToIp(pl3)) : "<n/a>", bsize);
      
   if (sm[SUBS_ATYPE]) string_sub(buffer, "%atp%", action2string(atype), bsize);
   if (sm[SUBS_ADATE]) string_sub(buffer, "%atm%", time2str1(date), bsize);
   if (sm[SUBS_CSTATUS]) string_sub(buffer, "%cst%", n2str(pl2), bsize);
   
   if (sm[SUBS_NSTATUS]) 
      string_sub(buffer, "%nst%", atype == ACT_STATUS ? n2str(pl4) : "<n/a>", bsize);
      
   if (sm[SUBS_ERRCODE]) 
      string_sub(buffer, "%eec%", atype == ACT_RDBMSERR ? n2str(pl1) : "<n/a>", bsize);
      
   if (sm[SUBS_SRCHCODE]) 
      string_sub(buffer, "%scd%", atype == ACT_SEARCH ? n2str(pl4) : "<n/a>", bsize);
      
   if (sm[SUBS_OPTSTRING]) string_sub(buffer, "%ost%", par11, bsize);
   if (sm[SUBS_ISDVER]) string_sub(buffer, "%isv%", Iversion, bsize);
   if (sm[SUBS_COMPILER]) string_sub(buffer, "%cmp%", COMPILER_NAME, bsize);
   if (sm[SUBS_OS]) string_sub(buffer, "%osv%", SYSTEM_UNAME, bsize);
   if (sm[SUBS_ONLNUSERS]) string_sub(buffer, "%oun%",n2str(ipc_vars->online_usr_num), bsize);
   if (sm[SUBS_SYSNAME]) string_sub(buffer, "%snm%", SYSTEM_NAME, bsize);
   if (sm[SUBS_STARTTIME]) string_sub(buffer, "%sst%", time2str1(server_started), bsize);
   /* ------------------------------ */
   
   if (sm[SUBS_DFNAME]) 
       string_sub(buffer, "%dfn%", lsuccess == True ? user.first : "<n/a>", bsize);
       
   if (sm[SUBS_DLNAME]) 
       string_sub(buffer, "%dln%", lsuccess == True ? user.last : "<n/a>", bsize);
       
   if (sm[SUBS_DNNAME]) 
       string_sub(buffer, "%dnn%", lsuccess == True ? user.nick : "<n/a>", bsize);
       
   if (sm[SUBS_DEMAIL1]) 
       string_sub(buffer, "%em1%", lsuccess == True ? user.email1 : "<n/a>", bsize);
       
   if (sm[SUBS_DEMAIL2]) 
       string_sub(buffer, "%em2%", lsuccess == True ? user.email2 : "<n/a>", bsize);
       
   if (sm[SUBS_DEMAIL3]) 
       string_sub(buffer, "%em3%", lsuccess == True ? user.email3 : "<n/a>", bsize);
       
   if (sm[SUBS_BDATE])
   {
       snprintf(bdate, 31, "%02d.%02d.%04d", user.bday, user.bmonth, user.byear);
       string_sub(buffer, "%dbd%", lsuccess == True ? bdate : "<n/a>", bsize);
   }
   
   if (sm[SUBS_CDATE]) 
       string_sub(buffer, "%dcd%", lsuccess == True ? time2str1(user.cr_date) : "<n/a>", bsize);
       
   if (sm[SUBS_LLDATE]) 
       string_sub(buffer, "%dll%", lsuccess == True ? time2str1(user.lastlogin) : "<n/a>", bsize);
   /* ------------------------------ */

   if (sm[SUBS_RFNAME]) 
       string_sub(buffer, "%rfn%", rlsuccess == True ? ruser.first : "<n/a>", bsize);  
       
   if (sm[SUBS_RLNAME]) 
       string_sub(buffer, "%rln%", rlsuccess == True ? ruser.last : "<n/a>", bsize);
       
   if (sm[SUBS_RNNAME]) 
       string_sub(buffer, "%rnn%", rlsuccess == True ? ruser.nick : "<n/a>", bsize);
       
   if (sm[SUBS_REMAIL]) 
       string_sub(buffer, "%rem%", rlsuccess == True ? ruser.email1 : "<n/a>", bsize);

   /* that's all folks */   
   return(buffer);
}


/*************************************************************************/
/* Lookup template by its filename.					 */
/*************************************************************************/
struct template_record *template_lookup(char *name)
{
   struct template_record *ptt_current = ptt_list;

   /* check if list is empty */
   if (ptt_list == NULL) return(NULL);
   
   /* search thru whole list to end */
   while (ptt_current != NULL)
   {
      if (strncmp(ptt_current->filename, name, 255)==0) return (ptt_current);
      ptt_current  = ptt_current->next;
   }
   
   return(NULL);
}


/*************************************************************************/
/* Read (and init) template from file.					 */
/*************************************************************************/
struct template_record *template_read(char *name)
{
   struct template_record *ptt_current = ptt_list;
   struct template_record *ptt = new template_record;
   FILE *ptt_file;
   off_t fsize;
   fstring fname;
   fstrcpy(fname, name);

   if (file_exist(fname, NULL)) 
   {
      fsize = get_file_size(fname);
      if (fsize > 0)
      {
         if (fsize > 32768) fsize = 32768;
	 ptt_file = fopen(fname, "r");
	      
	 ptt->value = (char*)malloc(fsize+1);
	 fsize = fread(ptt->value, 1, fsize, ptt_file);
	 ptt->size = fsize+1;
	 fstrcpy(ptt->filename, fname);
	 fclose(ptt_file);
	       
	 ptt->value[fsize] = 0;
	 template_scan(ptt);
	 
	 /* now time to insert template into chain */
         if (ptt_list == NULL)
         {
            ptt_list = ptt;
            ptt_list->next = NULL;
            ptt_list->prev = NULL;
	    
	    DEBUGADD(2, ("Read template file ok: %s\n", fname));
            return(ptt);
         }
      
         /* insert new template to the begin of the list */
         ptt_list = ptt;
         ptt_list->next = ptt_current;
         ptt_list->prev = NULL;
         ptt_current->prev = ptt_list;
	       
	 DEBUGADD(2, ("Add template file ok: %s\n", fname));
         return(ptt);
      }    
   }

   LOG_SYS(0, ("WARN: Can't read from template file: %s\n", fname));
   return (NULL);
}


/*************************************************************************/
/* scan template for subs codes and fill scan matrix			 */
/*************************************************************************/
void template_scan(struct template_record *ptt)
{
   int i = 0;
   ptt->db_lookup_needed = False;
   ptt->rg_lookup_needed = False;

   /* dumb scan here, very bad solution :( */
   /* (!) adding new code don't forget to add it to calc_dp & subst func */
   ptt->smatrix[SUBS_UIN]       = count_subs(ptt->value, "%uin%");
   ptt->smatrix[SUBS_IP]        = count_subs(ptt->value, "%ips%");
   ptt->smatrix[SUBS_ERRCODE]   = count_subs(ptt->value, "%eec%");
   ptt->smatrix[SUBS_ATYPE]     = count_subs(ptt->value, "%atp%");
   ptt->smatrix[SUBS_CSTATUS]   = count_subs(ptt->value, "%cst%");
   ptt->smatrix[SUBS_NSTATUS]   = count_subs(ptt->value, "%nst%");
   ptt->smatrix[SUBS_SRCHCODE]  = count_subs(ptt->value, "%scd%");
   ptt->smatrix[SUBS_ADATE]     = count_subs(ptt->value, "%atm%");
   ptt->smatrix[SUBS_OPTSTRING] = count_subs(ptt->value, "%ost%");
   ptt->smatrix[SUBS_ISDVER]    = count_subs(ptt->value, "%isv%");
   ptt->smatrix[SUBS_COMPILER]  = count_subs(ptt->value, "%cmp%");
   ptt->smatrix[SUBS_OS]        = count_subs(ptt->value, "%osv%");
   ptt->smatrix[SUBS_ONLNUSERS] = count_subs(ptt->value, "%oun%");
   ptt->smatrix[SUBS_SYSNAME]   = count_subs(ptt->value, "%snm%");
   ptt->smatrix[SUBS_STARTTIME] = count_subs(ptt->value, "%sst%");
   
   ptt->smatrix[SUBS_DFNAME]    = count_subs(ptt->value, "%dfn%");
   ptt->smatrix[SUBS_DLNAME]    = count_subs(ptt->value, "%dln%");
   ptt->smatrix[SUBS_DNNAME]    = count_subs(ptt->value, "%dnn%");
   ptt->smatrix[SUBS_DEMAIL1]   = count_subs(ptt->value, "%em1%");
   ptt->smatrix[SUBS_DEMAIL2]   = count_subs(ptt->value, "%em2%");
   ptt->smatrix[SUBS_DEMAIL3]   = count_subs(ptt->value, "%em3%");
   ptt->smatrix[SUBS_BDATE]     = count_subs(ptt->value, "%dbd%");
   ptt->smatrix[SUBS_CDATE]     = count_subs(ptt->value, "%dcd%");
   ptt->smatrix[SUBS_LLDATE]    = count_subs(ptt->value, "%dll%");
   
   ptt->smatrix[SUBS_RFNAME]    = count_subs(ptt->value, "%rfn%");
   ptt->smatrix[SUBS_RLNAME]    = count_subs(ptt->value, "%rln%");
   ptt->smatrix[SUBS_RNNAME]    = count_subs(ptt->value, "%rnn%");
   ptt->smatrix[SUBS_REMAIL]    = count_subs(ptt->value, "%rem%");
   
   /* check if users_info_ext table lookup needed */
   for (i=SUBS_DIRECT; i<(SUBS_DIRECT+SUBS_USERS); i++)
   {
      if (ptt->smatrix[i] > 0) ptt->db_lookup_needed = True;
   }

   /* check if register_requests table lookup needed */
   for (i=SUBS_DIRECT+SUBS_USERS; i<SUBS_NUM; i++)
   {
      if (ptt->smatrix[i] > 0) ptt->rg_lookup_needed = True;
   }
   
   DEBUG(350, ("Scan complete: db_lookup=%d, rg_lookup=%d\n", 
              ptt->db_lookup_needed, ptt->rg_lookup_needed));
}


/*************************************************************************/
/* Lookup variable by its name.						 */
/*************************************************************************/
struct variable_record *variable_lookup(char *name)
{
   struct variable_record *var_current = var_list;

   /* check if list is empty */
   if (var_list == NULL) return(NULL);
   
   /* search thru whole list to end */
   while (var_current != NULL)
   {
      if (strncmp(var_current->name, name, 255)==0) return (var_current);
      var_current  = var_current->next;
   }
   
   return(NULL);
}


/*************************************************************************/
/* Lookup aim variable by its name.					 */
/*************************************************************************/
struct variable_record *aim_get_variable(char *name)
{
   struct variable_record *var_current = aim_list;

   /* check if list is empty */
   if (aim_list == NULL) return(NULL);
   
   /* search thru whole list to end */
   while (var_current != NULL)
   {
      if (strncmp(var_current->name, name, 255)==0) return (var_current);
      var_current  = var_current->next;
   }
   
   return(NULL);
}


/*====================== AIM PARSE FUNCS STARTS HERE ====================*/

/*************************************************************************/
/* Init aim node tree.							 */
/*************************************************************************/
void init_aim_tree()
{
   aim_root.aim_families = NULL;
   aim_root.rate_classes = NULL;
}


/*************************************************************************/
/* Add rate_class to aim_root rate_classes chain			 */
/*************************************************************************/
void add_rate_class(unsigned short index)
{
   struct rate_class *brc  = aim_root.rate_classes;
   struct rate_class *brc2 = aim_root.rate_classes;
   
   while(brc)
   {
      if (brc->rate_index == index)
      {
         plist_free(brc->plist);
         brc->plist = NULL;
	 return;
      }
      
      brc = brc->next;
   }
   
   brc = new rate_class;
   brc->rate_index = index;
   brc->plist = NULL;
   brc->next = NULL;
   
   if (!(aim_root.rate_classes))
   {
      brc->prev = NULL;
      aim_root.rate_classes = brc;
   }
   else
   {
      while (brc2)
      {
         if (brc2->next == NULL)
	 {
	    brc2->next = brc;
	    brc->prev = brc2;
	    return;
	 }
	 
	 brc2 = brc2->next;
      }
   }
}


/*************************************************************************/
/* Add parameter to rate_class with specified index			 */
/*************************************************************************/
void add_rate_variable(unsigned short index, char *pname, unsigned short pvalue)
{
   struct rate_class *brc = aim_root.rate_classes;
   struct param_list *prl, *prl2;
   
   while(brc)
   {
      if (brc->rate_index == index)
      {
         prl = new param_list;
	 prl->name = NULL;
	 string_set(&prl->name, pname);
	 prl->value = pvalue;
	 prl->next = NULL;
	 
	 if (!(brc->plist))
	 {
	    prl->prev = NULL;
	    brc->plist = prl;
	 }
	 else
	 {
	    prl2 = brc->plist;
	    
	    while (prl2)
	    {
	       if (prl2->next == NULL)
	       {
	          prl2->next = prl;
		  prl->prev = prl2;
		  prl->next = NULL;
	          return;
	       }
	       prl2 = prl2->next;
	    }
	 }

	 return;
      }
      
      brc = brc->next;
   }
}


/*************************************************************************/
/* Free memory allocated for param list					 */
/*************************************************************************/
void plist_free(struct param_list *plist)
{
   struct param_list *tplist = plist;
   struct param_list *slist;
   
   while(tplist)
   {
      string_free(&tplist->name);
      slist  = tplist;
      tplist = tplist->next;
      delete slist;
   }
}


/*************************************************************************/
/* Add snac_family							 */
/*************************************************************************/
void add_snac_family(unsigned short fnumber)
{
   struct aim_family *bam = aim_root.aim_families;
   struct aim_family *bam2 = aim_root.aim_families;
   
   while(bam)
   {
      if (bam->number == fnumber)
      {
         subtypes_list_free(bam->subtypes);
         bam->subtypes = NULL;
	 return;
      }
      
      bam = bam->next;
   }
   
   bam = new aim_family;
   bam->number    = fnumber;
   bam->subtypes  = NULL;
   bam->next = NULL;
   
   if (!(aim_root.aim_families))
   {
      bam->prev = NULL;
      aim_root.aim_families = bam;
   }
   else
   {
      while (bam2)
      {
         if (bam2->next == NULL)
	 {
	    bam2->next = bam;
	    bam->prev = bam2;
	    return;
	 }
	 
         bam2 = bam2->next;
      }
   }
}


/*************************************************************************/
/* Free memory allocated for subtypes list				 */
/*************************************************************************/
void subtypes_list_free(struct subtype *stlist)
{
   struct subtype *tstlist = stlist;
   struct subtype *slist;
   
   while(tstlist)
   {
      /* Here we should delete subtypes from group */
      slist   = tstlist;
      tstlist = tstlist->next;
      delete slist;
   }
}


/*************************************************************************/
/* Set snac_family version						 */
/*************************************************************************/
void family_set_version(unsigned short fnumber, unsigned short version)
{
   struct aim_family *bam = aim_root.aim_families;
   
   while(bam)
   {
      if (bam->number == fnumber)
      {
         bam->version = version;
	 return;
      }
      
      bam = bam->next;
   }
}


/*************************************************************************/
/* Add subtype to snac_family record					 */
/*************************************************************************/
void add_family_subtype(unsigned short fnumber, unsigned short rclass, 
                                                unsigned short stype_num)
{
   struct aim_family *bam = aim_root.aim_families;
   struct subtype *nst  = NULL;
   struct subtype *nst2 = NULL;
   
   while(bam)
   {
      if (bam->number == fnumber)
      {
         nst = new subtype;
	 nst->num = stype_num;
	 nst->rate_ind = rclass;
	 nst->next = NULL;
	 
	 if (!(bam->subtypes))
	 {
	    bam->subtypes = nst;
	    nst->prev = NULL;
	 }
	 else
	 {
	    nst2 = bam->subtypes;
	    
	    while(nst2)
	    {
	       if (nst2->next == NULL)
	       {
	          nst2->next = nst;
		  nst->prev = nst2;
		  return;
	       }
	       
	       nst2 = nst2->next;
	    }
	 }
	 
	 return;
      }
      
      bam = bam->next;
   }
}


/*************************************************************************/
/* Check if rate exist, overwise return first rate class index		 */
/*************************************************************************/
unsigned long rate_check(unsigned short index)
{
   struct rate_class *brc = aim_root.rate_classes;
   
   while(brc)
   {
      if (brc->rate_index == index)
      {
         return(index);
      }
      
      brc = brc->next;
   }

   /* Return first if index doesn't exist */
   if (aim_root.rate_classes != NULL)   
   {
      return(aim_root.rate_classes->rate_index);
   }
   
   /* What is the hell ? */
   return(0);
}



/*************************************************************************/
/* Check if family exist and return its version				 */
/*************************************************************************/
int get_family_version(unsigned short family)
{
   struct aim_family *bam = aim_root.aim_families;
   
   while(bam)
   {
      if (bam->number == family)
      {
         return(bam->version);
      }
      
      bam = bam->next;
   }

   return(-1);
}


/*************************************************************************/
/* Return number of rate classes					 */
/*************************************************************************/
int get_rate_classes_num()
{
   int rate_num = 0;

   struct rate_class *brc = aim_root.rate_classes;
   
   while(brc)
   {
      rate_num++;
      brc = brc->next;
   }
   
   return(rate_num);
}


/*************************************************************************/
/* Return number of rate classes					 */
/*************************************************************************/
unsigned long get_rate_param(unsigned short index, char *pname)
{
   struct rate_class *brc  = aim_root.rate_classes;
   struct param_list *plst = NULL;
   
   while(brc)
   {
      if (brc->rate_index == index)
      {
         plst = brc->plist;
	 
         while(plst)
	 {
	    if (strnequal(pname, plst->name, strlen(pname)))
	    {
	       return(plst->value);
	    }
	    plst = plst->next;
	 }
      }
      
      brc = brc->next;
   }
   
   return(0);
}


/*************************************************************************/
/* Return number of subtypes for specified rate_class			 */
/*************************************************************************/
unsigned short get_subtypes_num(unsigned short rate_ind)
{
   unsigned short rate_cnt = 0;
   struct aim_family *bam = aim_root.aim_families;
   struct subtype    *subtypes;
   
   while(bam)
   {
      subtypes = bam->subtypes;
      while (subtypes)
      {
         if (subtypes->rate_ind == rate_ind) rate_cnt++;
         subtypes = subtypes->next;
      }
      
      bam = bam->next;
   }
   
   return(rate_cnt);
}


