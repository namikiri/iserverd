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
/* This unit implements V3 registration and autoregistration future	  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#include "v3_defines.h"

extern int v3_timeout; 
extern int v3_retries;


/**************************************************************************/
/* Handler for processing new user registration info 			  */
/**************************************************************************/
void v3_process_reg_newuser()
{
   unsigned long temp; int i;
   unsigned short seq1, seq2, cvers, str_len;
   struct full_user_info new_user;
   
   v3_send_ack(int_pack);
   int_pack.reset();
   int_pack >> temp >> seq1 >> seq2 >> temp >> temp >> cvers;
   
   memset(&new_user, 0, sizeof(new_user));
   
   LOG_USR(10, ("V3 Client with version %d start registration...\n", cvers));
  
   /*****---------------------------------------------------------*****/
   /* Here we will parse packet data */
  
   int_pack >> str_len;
   if (!islen_valid(str_len, 32, "nickname", int_pack))
   {
      v3_send_not_connected(int_pack);
      return;
 
   } else { for (i=0; i<str_len; i++) int_pack >> new_user.nick[i]; }

   convert_to_postgres(new_user.nick, sizeof(new_user.nick));

   int_pack >> str_len;
   if (!islen_valid(str_len, 32, "firstname", int_pack))
   {
     v3_send_not_connected(int_pack);
     return;
 
   } else { for (i=0; i<str_len; i++) int_pack >> new_user.first[i]; }

   convert_to_postgres(new_user.first, sizeof(new_user.first));

   int_pack >> str_len;
   if (!islen_valid(str_len, 32, "lastname", int_pack))
   {
     v3_send_not_connected(int_pack);
     return;
 
   } else { for (i=0; i<str_len; i++) int_pack >> new_user.last[i]; }

   convert_to_postgres(new_user.last, sizeof(new_user.last));

   int_pack >> str_len;
   if (!islen_valid(str_len, 64, "email", int_pack))
   {
     v3_send_not_connected(int_pack);
     return;
 
   } else { for (i=0; i<str_len; i++) int_pack >> new_user.email2[i]; }
   convert_to_postgres(new_user.email2, sizeof(new_user.email2));

   int_pack >> new_user.gender;   
   
   /* extract home address from packet */
   int_pack >> str_len;
   if (!islen_valid(str_len, 64, "haddr", int_pack))
   {
      v3_send_not_connected(int_pack);
      return;
	 
   } else { for(i=0; i<str_len; i++) int_pack >> new_user.haddr[i]; }      
   convert_to_postgres(new_user.haddr, sizeof(new_user.haddr));
   
   /* extract home city from packet */
   int_pack >> str_len;
   if (!islen_valid(str_len, 32, "hcity", int_pack))
   {
     v3_send_not_connected(int_pack);
     return;
	 
   } else { for (i=0; i<str_len; i++) int_pack >> new_user.hcity[i]; }
   convert_to_postgres(new_user.hcity, sizeof(new_user.hcity));

   /* extract home state name from packet */
   int_pack >> str_len;
   if (!islen_valid(str_len, 32, "hstate", int_pack))
   {
     v3_send_not_connected(int_pack);
     return;
 
   } else { for (i=0; i<str_len; i++) int_pack >> new_user.hstate[i]; }
   convert_to_postgres(new_user.hstate, sizeof(new_user.hstate));      
   
   int_pack >> new_user.hcountry;
      
   /* extract home phone number from packet */
   int_pack >> str_len;
   if (!islen_valid(str_len, 32, "hphone", int_pack))
   {
      v3_send_not_connected(int_pack);
      return;
	 
   } else { for (i=0; i<str_len; i++) int_pack >> new_user.hphone[i]; }
   convert_to_postgres(new_user.hphone, sizeof(new_user.hphone));

   /* extract home fax number from packet */
   int_pack >> str_len;
   if (!islen_valid(str_len, 32, "hfax", int_pack))
   {
      v3_send_not_connected(int_pack);
      return;
	 
   } else { for (i=0; i<str_len; i++) int_pack >> new_user.hfax[i]; }
   convert_to_postgres(new_user.hfax, sizeof(new_user.hfax));

   /* extract home cell number from packet */
   int_pack >> str_len;
   if (!islen_valid(str_len, 32, "hcell", int_pack))
   {
      v3_send_not_connected(int_pack);
      return;
	 
   } else { for (i=0; i<str_len; i++) int_pack >> new_user.hcell[i]; }
   convert_to_postgres(new_user.hcell, sizeof(new_user.hcell));

   int_pack >> (unsigned long&)new_user.hzip;
   if (new_user.hzip == 65535) new_user.hzip = -1;
   int_pack >> new_user.gender;
   int_pack >> new_user.age;
   int_pack >> new_user.bday;
   int_pack >> new_user.bmonth;
   int_pack >> new_user.byear;
   
   /* extract home web address */      
   int_pack >> str_len;
   if (!islen_valid(str_len, 128, "hweb", int_pack))
   {
      v3_send_not_connected(int_pack);
      return;
 	 
   } else { for (i=0; i<str_len; i++) int_pack >> new_user.hpage[i]; }
   convert_to_postgres(new_user.hpage, sizeof(new_user.hpage));
  
   /* extract waddr from packet */
   int_pack >> str_len;
   if (!islen_valid(str_len, 64, "waddr", int_pack))
   {
      v3_send_not_connected(int_pack);
      return;
 	 
   } else { for(i=0; i<str_len; i++) int_pack >> new_user.waddr[i]; }
   convert_to_postgres(new_user.waddr, sizeof(new_user.waddr));     
   
   /* extract work city from packet */
   int_pack >> str_len;
   if (!islen_valid(str_len, 32, "wcity", int_pack))
   {
      v3_send_not_connected(int_pack);
      return;
 	 
   } else { for (i=0; i<str_len; i++) int_pack >> new_user.wcity[i]; }
   convert_to_postgres(new_user.wcity, sizeof(new_user.wcity));

   /* extract work state name from packet */
   int_pack >> str_len;
   if (!islen_valid(str_len, 32, "wstate", int_pack))
   {
      v3_send_not_connected(int_pack);
      return;
 	 
   } else { for (i=0; i<str_len; i++) int_pack >> new_user.wstate[i]; }
   convert_to_postgres(new_user.wstate, sizeof(new_user.wstate));      
   
   int_pack >> new_user.wcountry;

   /* extract work company number from packet */
   int_pack >> str_len;
   if (!islen_valid(str_len, 32, "wcompany", int_pack))
   {
      v3_send_not_connected(int_pack);
      return;
 	 
   } else { for (i=0; i<str_len; i++) int_pack >> new_user.wcompany[i]; }
   convert_to_postgres(new_user.wcompany, sizeof(new_user.wcompany));

   /* extract work title number from packet */
   int_pack >> str_len;
   if (!islen_valid(str_len, 32, "wtitle", int_pack))
   {
      v3_send_not_connected(int_pack);
      return;

   } else { for (i=0; i<str_len; i++) int_pack >> new_user.wtitle[i]; }
   convert_to_postgres(new_user.wtitle, sizeof(new_user.wtitle));

   int_pack >> (unsigned long &)new_user.wdepart;
   if (new_user.wdepart == 65535) new_user.wdepart = -1;    
   
   /* extract work phone number from packet */
   int_pack >> str_len;
   if (!islen_valid(str_len, 32, "wphone", int_pack))
   {
      v3_send_not_connected(int_pack);
      return;
 	 
   } else { for (i=0; i<str_len; i++) int_pack >> new_user.wphone[i]; }
   convert_to_postgres(new_user.wphone, sizeof(new_user.wphone));
  
   /* extract work fax number from packet */
   int_pack >> str_len;
   if (!islen_valid(str_len, 32, "wfax", int_pack))
   {
      v3_send_not_connected(int_pack);
      return;
  
   } else { for (i=0; i<str_len; i++) int_pack >> new_user.wfax[i]; }
   convert_to_postgres(new_user.wfax, sizeof(new_user.wfax));

   /* extract work pager number from packet */
   int_pack >> str_len;
   if (!islen_valid(str_len, 32, "wpager", int_pack))
   {
      v3_send_not_connected(int_pack);
      return;
 	 
   } else { for (i=0; i<str_len; i++) int_pack >> new_user.wpager[i]; }
   convert_to_postgres(new_user.wpager, sizeof(new_user.wpager));

   int_pack >> ((unsigned  long &)new_user.wzip);
   if (new_user.wzip == 65535) new_user.wzip = -1;

   int_pack >> str_len;
   if (!islen_valid(str_len, 128, "wweb", int_pack))
   {
      v3_send_not_connected(int_pack);
      return;
 	 
   } else { for (i=0; i<str_len; i++) int_pack >> new_user.wpage[i]; }
   convert_to_postgres(new_user.wpage, sizeof(new_user.wpage));

   /* This was bad idea to publish private data addressed to admin 	  */
   /* int_pack >> str_len;                                                 */
   /* if (!islen_valid(str_len, 255, "ext data", int_pack))                */
   /* {                                                                    */
   /*   v3_send_not_connected(int_pack);                                   */
   /*   return;	                                                          */
   /* } else { for (i=0; i<str_len; i++) int_pack >> new_user.notes[i]; }  */
   /* convert_to_postgres(new_user.first, sizeof(new_user.notes)); 	  */
   
   /*****---------------------------------------------------------*/
  
   /* generate new uin and passwd */
   new_user.uin    = db_users_new_uin();
   generate_passwd(new_user.passwd, 6);
   new_user.can_broadcast = 0;
   new_user.wocup = -1;
   new_user.ch_password = 1;
   new_user.disabled = 0;
   new_user.cr_date = time(NULL);
   strncpy(new_user.email1, new_user.email2, sizeof(new_user.email1));
   strncpy(new_user.email3, new_user.email2, sizeof(new_user.email1));
   
   /* check for new_users_table */
   if (new_users_table_exist())
   {
      /*if exist we need to ajust uin number */
      temp = db_users_new_uin2();
      if (new_user.uin < temp) new_user.uin = temp;
      
   } else { create_new_users_table(); }
  
   if (lp_v3_autoregister()) 
   {
     LOG_ALARM(0, ("New user has been added to database...\n"));
     db_users_add_user(new_user);
   }
   else
   {
     LOG_ALARM(0, ("New registration request wating...\n"));
     db_new_add_user(new_user);
   }

   send_event2ap(papack, ACT_REGISTR, new_user.uin, 0,
                 ipToIcq(int_pack.from_ip), 0, longToTime(time(NULL)), "");
		 
   /* send registration info to user... */
   v3_send_registration_ok(int_pack, seq1, seq2, new_user);
}

