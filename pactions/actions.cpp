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
/* This unit implements actions processor functions 			  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* main loop for actions processor					  */
/**************************************************************************/
void process_actions()
{
   Packet apack;
   unsigned short ap_enabled = True;
   struct rule_node *current_rule;
   unsigned short atype;
   unsigned long  pl1,pl2,pl3,pl4,date;
   char par11[255];
   
   /* AP is starting up... we should parse config file and init all       */
   /* necessary structures and log stats information                      */
   parse_config_file(lp_actions_conf(), CONFIG_TYPE_ACTIONS);
   if (rsignal(SIGCHLD, (void(*)(int))myAPSIGCHLDHandler) == SIG_ERR)
   {
      LOG_SYS(0, ("AP: Can't install SIGCHLD signal handler. AP disabled\n"));
      ap_enabled = False;
   }

   log_parse_stats();
   usersdb_connect(); /* we need db connection for variable subs */
   
   /* loop... receive packet... process pack and again... circle of live  */   
   while (1)
   {
      /* ---------------------------------------------------------------- */
      /* atype -- action type                                             */
      /* pl1   -- user uin or error code                                  */
      /* pl2   -- user status                                             */
      /* pl3   -- user ip address                                         */
      /* pl4   -- search code or new status                               */
      /* pl5   -- time and date of action                                 */
      /* sl6   -- optional string                                         */
      /* ---------------------------------------------------------------- */
      if (recv_event2ap(apack, atype, pl1, pl2, pl3, pl4, date, (char *)par11) > 0)
      {
         /* if section for atype not empty & ap enabled - iterate thru all rules */
         if ((rules_root.sections_root[atype] != NULL) && (ap_enabled))
         {
            current_rule = rules_root.sections_root[atype];
	    while (current_rule != NULL)
	    {
	       if ((is_uin_match(pl1, current_rule->for_uins) == 1) && 
	           (current_rule->enabled == True))
	       {
	          if (current_rule->action == A_STOP) break;
	          
		  /* now we can make additional checks and run current_rule */
		  DEBUGADD(100, ("--> Current_rule->status = %lu\n", current_rule->status));
		  if (  ((atype == ACT_STATUS) && (pl4 == current_rule->status)) 
		     || ((atype == ACT_STATUS) && (current_rule->status == ICQ_STATUS_ANY))
		     ||  (atype != ACT_STATUS))
		  {
		     execute_rule(current_rule, atype, pl1, pl2, pl3, pl4, date, par11);
		  }
		  
		  if (current_rule->astop) break;
	       }
	       current_rule = current_rule->next;
	    }
         }
      }
   }
}


/**************************************************************************/
/* function to execute rule						  */
/**************************************************************************/
void execute_rule(struct rule_node *rule, unsigned short atype, 
                  unsigned long pl1, unsigned long pl2, 
                  unsigned long pl3, unsigned long pl4, 
		  unsigned long date, char *par11)
{
   switch (rule->action)
   {
      case A_MSG:
                   DEBUGADD(50, ("--> Action A_MSG was passed to rule executer.\n"));
		   execute_rule_msg(rule, atype, pl1, pl2, pl3, pl4, date, par11);
		   break;
      case A_LOG:
    		   DEBUGADD(50, ("--> Action A_LOG was passed to rule executer.\n"));
		   execute_rule_log(rule, atype, pl1, pl2, pl3, pl4, date, par11);
		   break;
      case A_MAIL:    
    		   DEBUGADD(50, ("--> Action A_MAIL was passed to rule executer.\n"));
		   execute_rule_mail(rule, atype, pl1, pl2, pl3, pl4, date, par11);
		   break;
      case A_RUN:
    		   DEBUGADD(50, ("--> Action A_RUN was passed to rule executer.\n"));
		   execute_rule_run(rule, atype, pl1, pl2, pl3, pl4, date, par11);
		   break;
		      
      default:	   LOG_SYS(0, ("ERR: Unknown action=%d passed to executer\n", rule->action));
		   break;
      
   }
}


/**************************************************************************/
/* function to send data to actions processor				  */
/**************************************************************************/
void send_event2ap(Packet &apack, unsigned short atype, 
                   unsigned long pl1, unsigned long pl2, 
                   unsigned long pl3, unsigned long pl4, 
		   unsigned long date, char *str11)
{
   apack.clearPacket();
   
   apack << (unsigned short)atype
         << (unsigned  long)pl1
	 << (unsigned  long)pl2
	 << (unsigned  long)pl3
	 << (unsigned  long)pl4
	 << (unsigned  long)date
	 << (unsigned short)(strlen(str11)+1)
	 << str11;
	 
   actions_send_packet(apack);   
}


/**************************************************************************/
/* function to receive data from actions IPC pipe			  */
/**************************************************************************/
int recv_event2ap(Packet &apack, unsigned short &atype, 
                   unsigned long &pl1, unsigned long &pl2, 
                   unsigned long &pl3, unsigned long &pl4, 
		   unsigned long &date, char *str11)
{
   unsigned short slen, i;
   int psize;
   char *dest = str11;
   
   psize = actions_receive_packet(apack);
   if (psize < 0) { return(-1); }
   
   /* apack contain all required variables... */
   apack.reset();
   apack >> atype >> pl1 >> pl2 >> pl3 >> pl4 >> date;
	 
   /* now we should get string parameter from received packet */
   apack >> slen;
   if (slen > 255) { slen = 255; }
   
   for(i=0; i<slen; i++) apack >> tempst[i];
   strncpy(dest, tempst, 255); 

   DEBUG(50, ("APEvent(%d) pls=%lu/%lu/%lu/%lu, date=%lu, str1=\'%s\'\n", 
            atype, pl1, pl2, pl3, pl4, date, str11));
   
   return (psize);
}


/**************************************************************************/
/* RULE_MESSAGE: function to execute send_message rule			  */
/**************************************************************************/
void execute_rule_msg(struct rule_node *rule, unsigned short atype, 
                      unsigned long pl1, unsigned long pl2, 
                      unsigned long pl3, unsigned long pl4, 
		      unsigned long date, char *par11)
{
   struct template_record *ptt = NULL;
   struct variable_record *var = NULL;
   struct msg_header msg_hdr;
   int msg_cnt = rule->tgt_uins->count;
   char *subs_buf = NULL;
   
   bzero((void *)&msg_hdr, sizeof(msg_hdr));

   /* check for sender uin */
   var = variable_lookup("MSG_SENDER_UIN");
   var != NULL ? msg_hdr.fromuin = var->num_val : msg_hdr.fromuin = 1001;

   msg_hdr.fromindex = 0;  /* we don't know index  */
   msg_hdr.mtype = 0x0001; /* normal message       */
   msg_hdr.mkind = 0x0001; /* plain text message   */

   /* rule->string_2 parameter is a pattern filename */
   ptt = load_template(rule->string_2);
   if (ptt != NULL)
   {
      subs_buf = execute_template_subst(ptt, rule, atype, pl1, pl2, 
                                        pl3, pl4, date, par11);

      /* max size for email message is 1024 bytes */
      if (strlen(subs_buf) > MAX_AMSG) 
      {
         LOG_SYS(0, ("Warn: truncating msg template: %s\n", ptt->filename));
         string_truncate(subs_buf, MAX_AEMAIL);
      }
	 
      /* it is time to get target uin(s) and send message(s) */
      msg_hdr.msglen = strlen(subs_buf);

      for (int i=0;i<msg_cnt;i++)
      {
         msg_hdr.touin = rule->tgt_uins->node[i].uin_1;
         process_message(msg_hdr, subs_buf);
      }
      
      free(subs_buf);
      return;
   }

   rule->enabled = False;
   return;

}


/**************************************************************************/
/* RULE_LOG: function to execute log rule				  */
/**************************************************************************/
void execute_rule_log(struct rule_node *rule, unsigned short atype, 
                      unsigned long pl1, unsigned long pl2, 
                      unsigned long pl3, unsigned long pl4, 
		      unsigned long date, char *par11)
{

}

/**************************************************************************/
/* RULE_RUN: function to execute run_program rule			  */
/**************************************************************************/
void execute_rule_run(struct rule_node *rule, unsigned short atype, 
                      unsigned long pl1, unsigned long pl2, 
                      unsigned long pl3, unsigned long pl4, 
		      unsigned long date, char *par11)
{
   FILE *rpipe = NULL;

   rpipe = dpopen(rule->string_1, "w");
   if (rpipe != NULL)
   {
      /* now we should pass all parameters to external program       */
      /* stdin stream - it is jast a set of strings:                 */
      /* 1 - action type string ("Online", "Offline", "Search", ...) */
      /* 2 - pl1 parameter (uin or error code)                       */
      /* 3 - user status code                                        */
      /* 4 - search code or new status                               */
      /* 5 - user ip address string (i.e. "10.10.10.2")              */
      /* 6 - optional text string                                    */
      
      fprintf(rpipe, "%s\n%lu\n%lu\n%s\n%lu\n%s\n\"%s\"\n", action2string(atype), 
              pl1, pl2, inet_ntoa(icqToIp(pl3)), pl4, time2str1(date), par11);
		 
      fflush(rpipe);
      fclose(rpipe);
   }
   else
   {
      rule->enabled = False;
   }
}


/**************************************************************************/
/* RULE_MAIL: function to execute send_mail rule			  */
/**************************************************************************/
void execute_rule_mail(struct rule_node *rule, unsigned short atype, 
                       unsigned long pl1, unsigned long pl2, 
                       unsigned long pl3, unsigned long pl4, 
		       unsigned long date, char *par11)
{
   fstring mail_program;
   struct template_record *ptt = NULL;
   struct variable_record *variable = NULL;
   char *subs_buf = NULL;
   FILE *rpipe = NULL;

   /* rule->string_1 parameter is a email address    */
   /* rule->string_2 parameter is a pattern filename */
   /* first we should make mail_program variable     */
   variable = variable_lookup("MAIL_PROGRAM");
   if (variable != NULL)
   {
      strncpy(mail_program, variable->str_val, sizeof(mail_program)-1);
      string_sub(mail_program, "%s", rule->string_1, sizeof(mail_program)-1);
      DEBUG(50, ("Command for mailer: %s\n", mail_program));
      
      /* now time to get template and make message */
      ptt = load_template(rule->string_2);
      if (ptt != NULL)
      {
         subs_buf = execute_template_subst(ptt, rule, atype, pl1, pl2, 
	                                   pl3, pl4, date, par11);

         /* max size for email message is 10000 bytes */
         if (strlen(subs_buf) > MAX_AEMAIL) 
	 {
	    LOG_SYS(0, ("Warn: truncating mail template: %s\n", ptt->filename));
	    string_truncate(subs_buf, MAX_AEMAIL);
	 }
	 
         /* wow... here we can send message... */
         rpipe = dpopen(mail_program, "w");
         if (rpipe != NULL)
         {      
	    fprintf(rpipe, "To: %s\n", rule->string_1);
            fprintf(rpipe, subs_buf);
            fflush(rpipe);
            fclose(rpipe);
	    free(subs_buf);
	    return;
         }
	 
	 free(subs_buf);
      }
   }

   rule->enabled = False;
   return;
}


/**************************************************************************/
/* convert action type to string					  */
/**************************************************************************/
struct template_record *load_template(char *tname)
{
   struct template_record *ptt = NULL;
   
   ptt = template_lookup(tname);
   if (ptt == NULL)
   {
      DEBUG(50, ("Template not found, loading from file...\n"));
      ptt = template_read(tname);
      if (ptt == NULL) return(NULL);
      return(ptt);
   }
   
   DEBUG(50, ("Template found in cache...\n"));
   return(ptt);
}


/**************************************************************************/
/* convert action type to string					  */
/**************************************************************************/
char *action2string(int action)
{
   static fstring actionBuf;
   
   switch (action)
   {
      case ACT_ONLINE:     strncpy(actionBuf, "Online", 15); break;                           
      case ACT_OFFLINE:    strncpy(actionBuf, "Offline", 15); break;
      case ACT_SAVEBASIC:  strncpy(actionBuf, "SaveBasicInfo", 15); break;
      case ACT_SEARCH:     strncpy(actionBuf, "Search", 15); break;
      case ACT_STATUS:     strncpy(actionBuf, "StatusChange", 15); break;
      case ACT_REGISTR:    strncpy(actionBuf, "Registration", 15); break;
      case ACT_RDBMSFAIL:  strncpy(actionBuf, "RDBMS_fail", 15); break;
      case ACT_RDBMSERR:   strncpy(actionBuf, "RDBMS_error", 15); break;
      case ACT_INTERR:     strncpy(actionBuf, "Internal_err", 15); break;
      case ACT_PPHUNG:     strncpy(actionBuf, "PPHung", 15); break;
   }
   
   return (actionBuf);
}




