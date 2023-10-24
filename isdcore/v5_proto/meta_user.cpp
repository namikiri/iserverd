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
/* This file contain all function related to CMD_META_USER and META_USER  */
/* this include user_info commands, user_search cmd's and set_info cmd's  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#include "v5_defines.h"

/**************************************************************************/
/* RCV_META_USER packet handler					   	  */
/**************************************************************************/
void v5_process_user_meta()
{
  unsigned short pvers, pcomm, seq1, seq2, sub_cmd;
  unsigned long  uin_num, temp_stamp, session_id; 
  unsigned long  target_uin;
  
  struct online_user user;

  int_pack.reset();
  int_pack  >> pvers   >> temp_stamp
	    >> uin_num >> session_id
	    >> pcomm   >> seq1
            >> seq2    >> temp_stamp 
	    >> sub_cmd >> target_uin;

  if (db_online_lookup(uin_num, user) == 0)
  {
    if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
	(user.session_id == session_id))
    {
        
      v5_send_ack(int_pack);
		  
      /* this cmd is very complex - it is include retriving user info, */
      /* user search, set user information, change password and other  */
      switch (sub_cmd)
      {
         case CMD_META_LOGIN: 
	          v5_send_lmeta(user, seq2); break;
	 case CMD_META_USER_INFO:
	          v5_reply_metainfo_request(user, target_uin, seq2); break;
	 case CMD_META_USER_INFO2:
		  v5_reply_metafullinfo_request2(user, target_uin, seq2); break;
	 case CMD_META_USER_LOGININFO:
	 case CMD_META_USER_FULLINFO:
		  v5_reply_metafullinfo_request(user, target_uin, seq2); break;
	 case CMD_META_USER_LOGININFO2:
	          v5_reply_metafullinfo_request2(user, target_uin, seq2);break;		  
	 case CMD_META_SEARCH_UIN:
		  v5_search_by_uin(user, target_uin, seq2); break;
	 case CMD_META_SEARCH_UIN2:
		  v5_search_by_uin2(user, target_uin, seq2);break;
	 case CMD_META_SEARCH_EMAIL:
	          v5_search_by_email(int_pack, user, seq2); break;
	 case CMD_META_SEARCH_EMAIL2:
	          v5_search_by_email2(int_pack, user, seq2);break;
	 case CMD_META_SEARCH_NAME:
		  v5_search_by_name(int_pack, user, seq2);  break;
	 case CMD_META_SEARCH_NAME2:
		  v5_search_by_name2(int_pack, user, seq2); break;
	 case CMD_META_SEARCH_WHITE:
	          v5_search_by_white(int_pack, user, seq2); break;
	 case CMD_META_SEARCH_WHITE2:
	          v5_search_by_white2(int_pack, user, seq2);break;
	 case CMD_META_SET_BASIC:
	          v5_set_basic_info(int_pack, user, seq2);  break;
	 case CMD_META_SET_BASIC2:
	          v5_set_basic_info2(int_pack, user, seq2); break;
	 case CMD_META_SET_WORK:
	          v5_set_work_info(int_pack, user, seq2);   break;
	 case CMD_META_SET_WORK2:
	          v5_set_work_info2(int_pack, user, seq2);  break;
	 case CMD_META_SET_ABOUT:
	          v5_set_about_info(int_pack, user, seq2);  break;
	 case CMD_META_SET_MORE:
	          v5_set_more_info(int_pack, user, seq2);   break;
	 case CMD_META_SET_MORE2:
	          v5_set_more_info2(int_pack, user, seq2);  break;
	 case CMD_META_SET_SECURITY:
	          v5_set_secure_info(int_pack, user, seq2); break;
	 case CMD_META_SET_PASS:
	          v5_set_password(int_pack, user, seq2);    break;
	 case CMD_META_SET_HPCAT:
		  v5_set_hpcat_info(int_pack, user, seq2);  break;
	 case CMD_META_SET_INTERESTS:
	          v5_set_interests_info(int_pack, user, seq2); break;
	 case CMD_META_SET_AFFLATIONS:
	          v5_set_affilations_info(int_pack, user, seq2); break;
	 case CMD_META_USER_UNREGISTER:
	          v5_unregister_user(int_pack, user, seq2); break;
	 case CMD_META_USAGE_STATS:
	          DEBUG(10, ("Usage stats from client with uin=%lu\n", user.uin));
	          break;

	 default: 
	          /* we should dump unknown meta_command to debug.log */
	          DEBUG(10, ("Unknown user meta command from %s:%d (%lu) [%04X]\n",
		  inet_ntoa(int_pack.from_ip), int_pack.from_port, uin_num, sub_cmd));
		  log_alarm_packet(0, int_pack);
		  break;
      }

    }
    else
    {

       LOG_ALARM(0, ("Spoofed (%lu) meta message from %s:%d\n", uin_num,
		      inet_ntoa(int_pack.from_ip), int_pack.from_port));
       v5_send_not_connected(int_pack);
    }
  }
  else
  {
    v5_send_not_connected(int_pack);
  }
}


/**************************************************************************/
/* SET_PASSWORD packet handler					   	  */
/**************************************************************************/
void v5_set_password(Packet &pack, struct online_user &user, 
		     unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short sub_cmd;
   unsigned long  uin_num, temp_stamp;
   ffstring password;

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;
   
   v5_extract_string(password, int_pack, 32, "password", user);
   
   /* Now we can save password info */   
   if (db_users_setpassword(user.uin, password) != -1) 
   {
      v5_send_meta_set_ack(seq2, user, SRV_META_SET_PASS_ACK, True);
   } 
   else
   {            
      LOG_USR(0, ("Error saving password for %lu\n", user.uin));
      v5_send_meta_set_ack(seq2, user, SRV_META_SET_PASS_ACK, False);
   }
}


/**************************************************************************/
/* SET_INTERESTS_INFO packet handler				   	  */
/**************************************************************************/
void v5_set_interests_info(Packet &pack, struct online_user &user, 
		           unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short sub_cmd;
   unsigned long  uin_num, temp_stamp;
   struct ext_user_info fuser;

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;
   
   int_pack >> fuser.int_num;
  
   if ((fuser.int_num > 4) | (fuser.int_num < 0)) fuser.int_num = 0;

   int_pack >> fuser.int_ind1;
   v5_extract_string(fuser.int_key1, int_pack, 63, "interests key1", user);
   int_pack >> fuser.int_ind2;
   v5_extract_string(fuser.int_key2, int_pack, 63, "interests key2", user);
   int_pack >> fuser.int_ind3;
   v5_extract_string(fuser.int_key3, int_pack, 63, "interests key3", user);
   int_pack >> fuser.int_ind4;
   v5_extract_string(fuser.int_key4, int_pack, 63, "interests key4", user);

   /* Now we can save interests info */   
   if (db_users_setinterests_info(user.uin, fuser) != -1) 
   {
      v5_send_meta_set_ack(seq2, user, SRV_META_SET_INTERESTS_ACK, True);
   } 
   else
   {            
      LOG_USR(0, ("Error saving interests user information for %lu\n", user.uin));
      v5_send_meta_set_ack(seq2, user, SRV_META_SET_INTERESTS_ACK, False);
   }
}


/**************************************************************************/
/* UNREGISTER_USER meta command handler 			   	  */
/**************************************************************************/
void v5_unregister_user(Packet &pack, struct online_user &user, 
		        unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short sub_cmd;
   unsigned long  uin_num, temp_stamp;
   ffstring password;

   int_pack.reset();
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd >> uin_num;
   
   v5_extract_string(password, int_pack, 32, "unreg password", user);
   
   if (db_users_delete_user(uin_num, password) != -1) 
   {
      LOG_USR(0, ("User %lu deleted from users_info table [unregistration]\n", uin_num));
      v5_send_meta_set_ack(seq2, user, SRV_META_UNREG_ACK, True);
   } 
   else
   {            
      LOG_USR(0, ("Error unregistering user %lu", user.uin));
      v5_send_meta_set_ack(seq2, user, SRV_META_UNREG_ACK, False);
   }
}


/**************************************************************************/
/* SET_AFFILATIONS_INFO packet handler				   	  */
/**************************************************************************/
void v5_set_affilations_info(Packet &pack, struct online_user &user, 
		           unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short sub_cmd;
   unsigned long  uin_num, temp_stamp;
   struct ext_user_info fuser;

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;

   int_pack >> fuser.past_num;
  
   if ((fuser.past_num > 3) | (fuser.past_num < 0)) fuser.past_num = 0;

   int_pack >> fuser.past_ind1;
   v5_extract_string(fuser.past_key1, int_pack, 63, "past keys1", user);
   int_pack >> fuser.past_ind2;
   v5_extract_string(fuser.past_key2, int_pack, 63, "past key2", user);
   int_pack >> fuser.past_ind3;
   v5_extract_string(fuser.past_key3, int_pack, 63, "past key3", user);

   int_pack >> fuser.aff_num;
  
   if ((fuser.aff_num > 3) | (fuser.aff_num < 0)) fuser.aff_num = 0;

   int_pack >> fuser.aff_ind1;
   v5_extract_string(fuser.aff_key1, int_pack, 63, "aff keys1", user);
   int_pack >> fuser.aff_ind2;
   v5_extract_string(fuser.aff_key2, int_pack, 63, "aff key2", user);
   int_pack >> fuser.aff_ind3;
   v5_extract_string(fuser.aff_key3, int_pack, 63, "aff key3", user);
   
   /* Now we can save affilations info */   
   if (db_users_setaffilations_info(user.uin, fuser) != -1) 
   {
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_AFFILAT_ACK, True);
   } 
   else
   {            
        LOG_USR(0, ("Error saving affilations user information for %lu", user.uin));
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_AFFILAT_ACK, False);
   }
}


/**************************************************************************/
/* SET_BASIC_INFO packet handler				   	  */
/**************************************************************************/
void v5_set_basic_info(Packet &pack, struct online_user &user, 
		       unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short sub_cmd;
   unsigned long  uin_num, temp_stamp;
   struct full_user_info fuser;

   send_event2ap(papack, ACT_SAVEBASIC, user.uin, user.status, ipToIcq(user.ip), 
                 0, longToTime(time(NULL)), "");                     

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;
   
   /* first block - personal information clauses */
   v5_extract_string(fuser.nick,   int_pack, 31, "nick name", user);
   v5_extract_string(fuser.first,  int_pack, 31, "first name", user);
   v5_extract_string(fuser.last,   int_pack, 31, "last name", user);
   v5_extract_string(fuser.email1, int_pack, 63, "email1", user);
   v5_extract_string(fuser.email2, int_pack, 63, "email2", user);
   v5_extract_string(fuser.email3, int_pack, 63, "email3", user);
   v5_extract_string(fuser.hcity,  int_pack, 31, "hcity", user);
   v5_extract_string(fuser.hstate, int_pack, 31, "hstate", user);
   v5_extract_string(fuser.hphone, int_pack, 31, "hphone", user);
   v5_extract_string(fuser.hfax,   int_pack, 31, "hfax", user);
   v5_extract_string(fuser.haddr,  int_pack, 63, "haddr", user);
   v5_extract_string(fuser.hcell,  int_pack, 31, "hcell", user);

   int_pack >> fuser.hzip;
   int_pack >> fuser.hcountry;
   int_pack >> fuser.gmt_offset;
   int_pack >> fuser.e1publ;
   
   /* Now we can save basic info */   
   if (db_users_setbasic_info2(user.uin, fuser) != -1) 
   {
      v5_send_meta_set_ack(seq2, user, SRV_META_SET_BASIC_ACK, True);
   } 
   else
   {            
      LOG_USR(0, ("Error saving basic user information for %lu", user.uin));
      v5_send_meta_set_ack(seq2, user, SRV_META_SET_BASIC_ACK, False);
   }
}


/**************************************************************************/
/* SET_MORE_INFO packet handler					   	  */
/**************************************************************************/
void v5_set_more_info(Packet &pack, struct online_user &user, 
		       unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short sub_cmd;
   unsigned long  uin_num, temp_stamp;
   struct full_user_info fuser;
   char temp_char;

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;
   
   int_pack >> fuser.age
            >> fuser.gender;

   v5_extract_string(fuser.hpage,  int_pack, 127, "hpage", user);
   
   int_pack >> temp_char;
   
   /* they expand year var to 2 bytes in 99b (save_more2) */
   fuser.byear = 1900 + temp_char;
   
   int_pack >> fuser.bmonth
	    >> fuser.bday
	    >> fuser.lang1
	    >> fuser.lang2
	    >> fuser.lang3;
	    
   /* Now we can save more info */   
   if (db_users_setV5more_info(user.uin, fuser) != -1) 
   {
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_MORE_ACK, True);
   } 
   else
   {            
        LOG_USR(0, ("Error saving more user information for %lu", user.uin));
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_MORE_ACK, False);
   }
}


/**************************************************************************/
/* SET_MORE_INFO packet handler					   	  */
/**************************************************************************/
void v5_set_more_info2(Packet &pack, struct online_user &user, 
		       unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short sub_cmd;
   unsigned long  uin_num, temp_stamp;
   struct full_user_info fuser;

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;
   
   int_pack >> fuser.age
            >> fuser.gender;

   v5_extract_string(fuser.hpage,  int_pack, 127, "hpage", user);
   
   int_pack >> fuser.byear
            >> fuser.bmonth
	    >> fuser.bday
	    >> fuser.lang1
	    >> fuser.lang2
	    >> fuser.lang3;
	    
   /* Now we can save more info */   
   if (db_users_setV5more_info(user.uin, fuser) != -1) 
   {
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_MORE_ACK, True);
   } 
   else
   {            
        LOG_USR(0, ("Error saving more user information for %lu", user.uin));
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_MORE_ACK, False);
   }
}


/**************************************************************************/
/* SET_MORE_INFO packet handler					   	  */
/**************************************************************************/
void v5_set_hpcat_info(Packet &pack, struct online_user &user, 
		       unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short sub_cmd;
   unsigned long  uin_num, temp_stamp;
   fstring description;
   char enabled;
   short hpindex;

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;
   
   int_pack >> enabled
            >> hpindex;

   v5_extract_string(description,  int_pack, 127, "hpage_txt", user);
   
   /* Now we can save more info */   
   if (db_users_sethpagecat_info(user.uin, enabled, hpindex, description) != -1) 
   {
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_HPCAT_ACK, True);
   } 
   else
   {            
        LOG_USR(0, ("Error saving hpcat user information for %lu", user.uin));
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_HPCAT_ACK, False);
   }
}


/**************************************************************************/
/* SET_SECURE_INFO packet handler					   	  */
/**************************************************************************/
void v5_set_secure_info(Packet &pack, struct online_user &user, 
		       unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short sub_cmd;
   unsigned long  uin_num, temp_stamp;
   char auth, dc_perms, webaware;

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;
   
   int_pack >> auth
            >> webaware
	    >> dc_perms; /* 0 - dc with any user, 1 - hide ip */
   
   if ((dc_perms < 0) || (dc_perms > 1)) dc_perms = 1;

   /* Now we can save security info */   
   if (db_users_setsecure_info(user.uin, auth, dc_perms, webaware) != -1) 
   {
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_SECURE_ACK, True);
   } 
   else
   {            
        LOG_USR(0, ("Error saving secure user information for %lu", user.uin));
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_SECURE_ACK, False);
   }
}


/**************************************************************************/
/* SET_BASIC_INFO2 packet handler					  */
/**************************************************************************/
void v5_set_basic_info2(Packet &pack, struct online_user &user, 
		        unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short sub_cmd;
   unsigned long  uin_num, temp_stamp;
   struct full_user_info fuser;
   ffstring hzip;

   send_event2ap(papack, ACT_SAVEBASIC, user.uin, user.status, ipToIcq(user.ip), 
                 0, longToTime(time(NULL)), "");

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;

   /* first block - personal information clauses */
   v5_extract_string(fuser.nick,   int_pack, 31, "nick name", user);   
   v5_extract_string(fuser.first,  int_pack, 31, "first name", user);
   v5_extract_string(fuser.last,   int_pack, 31, "last name", user);
   v5_extract_string(fuser.email1, int_pack, 63, "email1", user);
   v5_extract_string(fuser.email2, int_pack, 63, "email2", user);
   v5_extract_string(fuser.email3, int_pack, 63, "email3", user);
   v5_extract_string(fuser.hcity,  int_pack, 31, "hcity", user);
   v5_extract_string(fuser.hstate, int_pack, 31, "hstate", user);
   v5_extract_string(fuser.hphone, int_pack, 31, "hphone", user);
   v5_extract_string(fuser.hfax,   int_pack, 31, "hfax", user);
   v5_extract_string(fuser.haddr,  int_pack, 63, "haddr", user);
   v5_extract_string(fuser.hcell,  int_pack, 31, "hcell", user);
   v5_extract_string(hzip, 	   int_pack, 31, "hzip", user);

   int_pack >> fuser.hcountry;
   int_pack >> fuser.gmt_offset;
   int_pack >> fuser.e1publ;
   fuser.hzip = (unsigned long)atoul(hzip);

   /* Now we can save basic info */   
   if (db_users_setbasic_info2(user.uin, fuser) != -1) 
   {
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_BASIC_ACK, True);
   } 
   else
   {            
        LOG_USR(0, ("Error saving basic user information for %lu", user.uin));
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_BASIC_ACK, False);
   }
}


/**************************************************************************/
/* SET_WORK_INFO packet handler					  	  */
/**************************************************************************/
void v5_set_work_info(Packet &pack, struct online_user &user, 
		        unsigned short seq2)
{
   BOOL parse_ok = True;
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short sub_cmd;
   unsigned long  uin_num, temp_stamp;
   struct full_user_info fuser;

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;

   if(!v5_extract_string(fuser.wcity,     int_pack, sizeof(fuser.wcity)-1, "work city",    user)) parse_ok = False;
   if(!v5_extract_string(fuser.wstate,    int_pack, sizeof(fuser.wstate)-1, "work state",   user)) parse_ok = False;
   if(!v5_extract_string(fuser.wphone,    int_pack, sizeof(fuser.wphone)-1, "work phone",   user)) parse_ok = False;
   if(!v5_extract_string(fuser.wfax,      int_pack, sizeof(fuser.wfax)-1, "work fax",     user)) parse_ok = False;
   if(!v5_extract_string(fuser.waddr,     int_pack, sizeof(fuser.waddr)-1, "work address", user)) parse_ok = False;
   int_pack >> fuser.wzip;
   int_pack >> fuser.wcountry;
   if(!v5_extract_string(fuser.wcompany,  int_pack, sizeof(fuser.wcompany)-1, "work company", user)) parse_ok = False;
   if(!v5_extract_string(fuser.wdepart2,  int_pack, sizeof(fuser.wdepart2)-1, "work depart",  user)) parse_ok = False;
   if(!v5_extract_string(fuser.wtitle,    int_pack, sizeof(fuser.wtitle)-1, "work position",user)) parse_ok = False;
   int_pack >> fuser.wocup;
   if(!v5_extract_string(fuser.wpage,     int_pack, sizeof(fuser.wpage)-1, "work page",    user)) parse_ok = False;

   /* Now we can save work info */   
   if ((parse_ok) && (db_users_setwork_info2(user.uin, fuser) != -1))
   {
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_WORK_ACK, True);
   } 
   else
   {            
        LOG_USR(0, ("Error saving work user information for %lu", user.uin));
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_WORK_ACK, False);
   }
}


/**************************************************************************/
/* SET_WORK_INFO2 packet handler					  */
/**************************************************************************/
void v5_set_work_info2(Packet &pack, struct online_user &user, 
		        unsigned short seq2)
{
   BOOL parse_ok = True;
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short sub_cmd;
   unsigned long  uin_num, temp_stamp;
   struct full_user_info fuser;
   ffstring wzip;

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;

   if(!v5_extract_string(fuser.wcity,    int_pack, sizeof(fuser.wcity)-1, "work city",    user)) parse_ok = False;
   if(!v5_extract_string(fuser.wstate,   int_pack, sizeof(fuser.wstate)-1, "work state",   user)) parse_ok = False;
   if(!v5_extract_string(fuser.wphone,   int_pack, sizeof(fuser.wphone)-1, "work phone",   user)) parse_ok = False;
   if(!v5_extract_string(fuser.wfax,     int_pack, sizeof(fuser.wfax)-1, "work fax",     user)) parse_ok = False;
   if(!v5_extract_string(fuser.waddr,    int_pack, sizeof(fuser.waddr)-1, "work address", user)) parse_ok = False;
   if(!v5_extract_string(wzip,           int_pack, sizeof(wzip)-1, "work zip", user)) parse_ok = False;
   int_pack >> fuser.wcountry;
   if(!v5_extract_string(fuser.wcompany, int_pack, sizeof(fuser.wcompany)-1, "work company", user)) parse_ok = False;
   if(!v5_extract_string(fuser.wdepart2, int_pack, sizeof(fuser.wdepart2)-1, "work depart",  user)) parse_ok = False;
   if(!v5_extract_string(fuser.wtitle,   int_pack, sizeof(fuser.wtitle)-1, "work position",user)) parse_ok = False;
   int_pack >> fuser.wocup;
   if(!v5_extract_string(fuser.wpage,    int_pack, sizeof(fuser.wpage)-1, "work page",    user)) parse_ok = False;

   fuser.wzip = (unsigned long)atoul(wzip);

   /* Now we can save work info */
   if ((parse_ok) && (db_users_setwork_info2(user.uin, fuser) != -1))
   {
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_WORK_ACK, True);
   }
   else
   {            
        LOG_USR(0, ("Error saving work user information for %lu", user.uin));
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_WORK_ACK, False);
   }
}

/**************************************************************************/
/* This used to send reply packet on META_WANT_INFO                       */
/**************************************************************************/
void v5_reply_metainfo_request(struct online_user &user, 
			       unsigned long target_uin, unsigned short seq2)
{
   struct full_user_info tuser;
   
   if (db_users_lookup(target_uin, tuser) >= 0)
   {
      /* we found user info - make send it to user */
      v5_send_meta_info(seq2, user, tuser, True);
   }
   else
   {
      /* user not found or database problem */
      v5_send_meta_info(seq2, user, tuser, False);
   }
}


/**************************************************************************/
/* This used to send reply packet on META_WANT_INFO2                      */
/**************************************************************************/
void v5_reply_metainfo2_request(struct online_user &user, 
			       unsigned long target_uin, unsigned short seq2)
{
   struct full_user_info tuser;
   
   if (db_users_lookup(target_uin, tuser) >= 0)
   {
      /* we found user info - make send it to user */
      v5_send_meta_info2(seq2, user, tuser, True);
   }
   else
   {
      /* user not found ot database problem */
      v5_send_meta_info2(seq2, user, tuser, False);
   }
}


/**************************************************************************/
/* This used to send reply packet on META_WANT_FULLINFO                   */
/**************************************************************************/
void v5_reply_metafullinfo_request(struct online_user &user, 
			       unsigned long target_uin, unsigned short seq2)
{
   struct full_user_info tuser;
   struct notes_user_info notes;
   
   if (db_users_lookup(target_uin, tuser) >= 0)
   {
      db_users_notes(target_uin, notes);
  
      /* we found user info - make send it to user */
      v5_send_meta_info2(seq2, user, tuser, True);
      v5_send_meta_more(seq2, user, tuser, True);
      v5_send_meta_hpage_cat(seq2, user, tuser, True);
      v5_send_meta_work(seq2, user, tuser, True);
      v5_send_meta_about(seq2, user, notes, True);
      v5_send_meta_interestsinfo(seq2, user, tuser, True);
      v5_send_meta_affilationsinfo(seq2, user, tuser, True);
   }
   else
   {
      /* user not found ot database problem */
      v5_send_meta_info2(seq2, user, tuser, False);
   }
}


/**************************************************************************/
/* This used to send reply packet on META_WANT_FULLINFO                   */
/**************************************************************************/
void v5_reply_metafullinfo_request2(struct online_user &user, 
			       unsigned long target_uin, unsigned short seq2)
{
   struct full_user_info tuser;
   struct notes_user_info notes;
   
   DEBUG(10, ("Fullinfo request2 called\n"));

   if (db_users_lookup(target_uin, tuser) >= 0)
   {
      db_users_notes(target_uin, notes);
  
      /* we found user info - make send it to user */
      v5_send_meta_info3(seq2, user, tuser, True);
      v5_send_meta_more2(seq2, user, tuser, True);
      v5_send_meta_hpage_cat(seq2, user, tuser, True);
      v5_send_meta_work2(seq2, user, tuser, True);
      v5_send_meta_about(seq2, user, notes, True);
      v5_send_meta_interestsinfo(seq2, user, tuser, True);
      v5_send_meta_affilationsinfo(seq2, user, tuser, True);
   }
   else
   {
      /* user not found ot database problem */
      v5_send_meta_info2(seq2, user, tuser, False);
   }
}


/**************************************************************************/
/* SET_ABOUT_INFO packet handler					  */
/**************************************************************************/
void v5_set_about_info(Packet &pack, struct online_user &user, 
		       unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short sub_cmd;
   unsigned long  uin_num, temp_stamp;
   struct notes_user_info notes;

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;

   if ((v5_extract_string(notes.notes, int_pack, sizeof(notes.notes)-1, "set notes", user)) &&
       (db_users_setnotes(user.uin, notes) != -1))
   {
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_ABOUT_ACK, True);
   }
   else
   {            
        LOG_USR(0, ("Error saving notes user information for %lu", user.uin));
	v5_send_meta_set_ack(seq2, user, SRV_META_SET_ABOUT_ACK, False);
   }
}

