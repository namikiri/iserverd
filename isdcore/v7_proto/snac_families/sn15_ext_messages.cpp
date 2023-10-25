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
/* This module contain snac creating/processing functions                 */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

char err_code;
extern int serverzone;

/**************************************************************************/
/* Extended ICQ services SNAC family packets handler		  	  */
/**************************************************************************/
void process_snac_ext_messages(struct snac_header &snac, Packet &pack)
{
   switch (snac.subfamily)
   {
      case SN_IME_MULTIxMESSxREQ:     process_ime_multi_req(snac, pack); break;
      
      default: DEBUG(10, ("Unknown icq extended SNAC(0x15, %04X)\n", snac.subfamily));
   }
}


/**************************************************************************/
/* Multi-purpose request packets handler		  		  */
/**************************************************************************/
void process_ime_multi_req(struct snac_header &snac, Packet &pack)
{
   class tlv_chain_c tlv_chain;   
   class tlv_c *tlv;
   unsigned short remaining_size;
   unsigned short req_cmd;
   unsigned long requester_uin;
   struct online_user *user;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s:%d sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip), pack.from_port));

      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }
   
   tlv_chain.read(pack);
   
   if ((tlv = tlv_chain.get(0x1)) == NULL)
   {
      DEBUG(10, ("ERROR: Mailformed multi-request packet from %s:%d (tlv 0x1 )\n",
                inet_ntoa(pack.from_ip), pack.from_port));
		
      send_snac_error(snac.family, ERR_SNAC_DATA_INVALID, snac.id, pack);
      return;
   }
   
   /* OK... we have tlv 1 ready */
   tlv->intel_order();
   *tlv >> remaining_size;
   
   if (tlv->size != (remaining_size+sizeof(remaining_size)))
   {
      DEBUG(10, ("ERROR: Mailformed multi-request packet from %s:%d (tlv rem len)\n",
                inet_ntoa(pack.from_ip), pack.from_port));
		
      send_snac_error(snac.family, ERR_SNAC_DATA_INVALID, snac.id, pack);
      return;      
   }
   
   *tlv >> requester_uin;
   
   if (requester_uin != user->uin)
   {
      DEBUG(10, ("ERROR: Mailformed multi-request packet from %s:%d (req uin)\n",
                inet_ntoa(pack.from_ip), pack.from_port));
		
      send_snac_error(snac.family, ERR_SNAC_DATA_INVALID, snac.id, pack);
      return;         
   }
   
   *tlv >> req_cmd;
   
   switch (req_cmd)
   {
      case META_REQ_OFFLINE_MSG:  process_mr_messages_request(pack, user, snac, tlv);
                                  break;
      case META_ACK_OFFLINE_MSG:  process_mr_delete_messages(pack, user, snac);
                                  break;
      case META_REQ_INFORMATION:  process_mr_information_request(pack, user, snac, tlv); 
                                  break;
      
      default: DEBUG(10, ("WARN: Unknown multi-request command %04X from %s:%d\n",
                     req_cmd, inet_ntoa(pack.from_ip), pack.from_port)); break;
   }
}


/**************************************************************************/
/* Client ask us to delete offline messages 			  	  */
/**************************************************************************/
void process_mr_delete_messages(Packet &pack, struct online_user *user, 
                                struct snac_header &snac)
{
   DEBUG(50, ("Processing aim offline msgs delete request from %lu\n", user->uin));
      
   db_del_messages(user->uin, 0xFFFFFFFF);
   
   return;
}


/**************************************************************************/
/* Offline messages request packets handler			  	  */
/**************************************************************************/
void process_mr_messages_request(Packet &pack, struct online_user *user, 
                                 struct snac_header &snac, struct tlv_c *tlv)
{
   unsigned short mess_num, pack_num;
   unsigned short req_seq;

   struct msg_header msg_hdr;
   fstring dbcomm_str; 
   PGresult *res;
   pack_num = 0;
   
   *tlv >> req_seq;

   bzero((void *)&msg_hdr, sizeof(msg_hdr));
   DEBUG(10, ("Get offline msg request from %s (uin=%lu)\n", 
              inet_ntoa(pack.from_ip), user->uin));
     
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, 
           "DECLARE mportal CURSOR FOR SELECT * FROM Users_Messages WHERE to_uin=%lu ORDER BY msg_date", 
	    user->uin);

   PQclear(PQexec(users_dbconn, "BEGIN;"));

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[V7 DECLARE PORTAL (msgs)]");
      return;
   }
   else
   {
      PQclear(res);
   }
        
            
   for (mess_num=0;;mess_num++)
   {
      res = PQexec(users_dbconn, "FETCH IN mportal");
      if (PQresultStatus(res) != PGRES_TUPLES_OK)
      {
         handle_database_error(res, "[V5 FETCH FROM PORTAL (msgs)]");
         break;
      }
  	 
      if (PQntuples(res) == 0)
      {
         PQclear(res);
         break;
      }

      if (PQnfields(res) != MESS_TBL_FIELDS)
      {
         LOG_SYS(0, ("Corrypted usr_messages table structure in db: \"%s\"\n", 
                      lp_db_users()));
			    
         exit(EXIT_ERROR_DB_STRUCTURE);
      }
      
      strncpy(msg_buff, PQgetvalue(res, 0, 4), sizeof(msg_buff)-1);
      ITrans.translateToClient(msg_buff);
    
      msg_hdr.touin     = atoul(PQgetvalue(res, 0, 0));
      msg_hdr.fromuin   = atoul(PQgetvalue(res, 0, 1));
      msg_hdr.mtime     = atoul(PQgetvalue(res, 0, 2));
      msg_hdr.mtype     =  atol(PQgetvalue(res, 0, 3));
      msg_hdr.msglen    = strlen(msg_buff);

      mr_send_offline_message(pack, user, snac, req_seq, 0x0001, msg_hdr, msg_buff);

      PQclear(res);
   }
      
   PQclear(PQexec(users_dbconn, "CLOSE mportal"));
   PQclear(PQexec(users_dbconn, "END;"));

   mr_send_end_of_messages(pack, user, snac, req_seq, 0);

   return;
}


/**************************************************************************/
/* Multi-purpose information request packets handler		  	  */
/**************************************************************************/
void process_mr_information_request(Packet &pack, struct online_user *user, 
                                    struct snac_header &snac, struct tlv_c *tlv)
{
   unsigned short req_seq;
   unsigned short sub_cmd;
   unsigned long  to_uin;
   unsigned short success = False;
   struct full_user_info finfo;
   
   bzero((void *)&finfo, sizeof(finfo));
   
   *tlv >> req_seq
        >> sub_cmd;
   
   /* There are many request types */
   switch (sub_cmd)
   {
      case META_INFO_REQ_HOMEINFO: 
              *tlv >> to_uin;
	      
	      if (db_users_lookup(to_uin, finfo) >= 0) success = True;
	      
	      /* here flags used to show that sequence finished/not finished */
              mr_send_home_info(pack, user, snac, req_seq, finfo, success,  1);
	      mr_send_more_info(pack, user, snac, req_seq, finfo, success,  1);
	      mr_send_email_info(pack, user, snac, req_seq, finfo, success, 1);
	      mr_send_hpage_info(pack, user, snac, req_seq, finfo, success, 1);
	      mr_send_work_info(pack, user, snac, req_seq, finfo, success,  1);
	      mr_send_about_info(pack, user, snac, req_seq, to_uin, 1);
	      mr_send_interests_info(pack, user, snac, req_seq, finfo, success, 1);
	      mr_send_affilations_info(pack, user, snac, req_seq, finfo, success, 0);
	      
	      break;

      case META_INFO_REQ_SHORTINFO:
              *tlv >> to_uin;
	      if (db_users_lookup(to_uin, finfo) >= 0) success = True;
	      mr_send_short_info(pack, user, snac, req_seq, finfo, success, 0);
	      
	      break;
	      
      case META_INFO_REQ_FULLINFO:
              *tlv >> to_uin;
	      if (db_users_lookup(to_uin, finfo) >= 0) success = True;
	      
              mr_send_home_info(pack, user, snac, req_seq, finfo, success,  1);
	      mr_send_more_info(pack, user, snac, req_seq, finfo, success,  1);
	      mr_send_email_info(pack, user, snac, req_seq, finfo, success, 1);
	      mr_send_hpage_info(pack, user, snac, req_seq, finfo, success, 1);
	      mr_send_work_info(pack, user, snac, req_seq, finfo, success,  1);
	      mr_send_about_info(pack, user, snac, req_seq, to_uin, 1);
	      mr_send_interests_info(pack, user, snac, req_seq, finfo, success, 1);
	      mr_send_affilations_info(pack, user, snac, req_seq, finfo, success, 0);
	      
	      break;	      

      case META_INFO_REQ_WP_UIN:
             *tlv >> to_uin;
	     if (db_users_lookup(to_uin, finfo) >= 0) success = True;
             mr_send_wp_found(pack, user, snac, req_seq, finfo, success, 0, True, 0); break;

      case META_INFO_SET_PERMS:
      	     mr_set_user_perms_info(pack, user, snac, req_seq, *tlv); break;
	     
      case META_INFO_SET_PASSWORD:      
             mr_set_user_pass_info(pack, user, snac, req_seq, *tlv); break;
	     
      case META_INFO_SET_HOMEINFO:
             mr_set_user_home_info(pack, user, snac, req_seq, *tlv); break;

      case META_INFO_SET_MOREINFO:
             mr_set_user_more_info(pack, user, snac, req_seq, *tlv); break;
      
      case META_INFO_SET_NOTESINFO:
             mr_set_user_notes_info(pack, user, snac, req_seq, *tlv); break;

      case META_INFO_SET_WORKINFO:
             mr_set_user_work_info(pack, user, snac, req_seq, *tlv); break;

      case META_INFO_SET_EMAILINFO:
             mr_set_user_email_info(pack, user, snac, req_seq, *tlv); break;
	     
      case META_INFO_SET_INTINFO:
             mr_set_user_interests_info(pack, user, snac, req_seq, *tlv); break;
	     
      case META_INFO_SET_AFFILAT:
             mr_set_user_affilations_info(pack, user, snac, req_seq, *tlv); break;
	     
      case META_INFO_SET_HPCAT:
             mr_set_user_hpcat_info(pack, user, snac, req_seq, *tlv); break;
	     
      case META_INFO_SET_ICQPHONE:
             mr_set_user_icqphone_info(pack, user, snac, req_seq, *tlv); break;
	
      case META_XML_REQ_DATA:
             process_mr_xml_request(pack, user, snac, req_seq, *tlv); break;
      
      case META_SEND_SMS:
             process_mr_sms_request(pack, user, snac, req_seq, *tlv); break;

      /* aol company made a wildcard search, but I think this is foolish   */
      /* so i wrapped this requests to non-wildcard handlers and wildcards */
      /* is not supported by iserverd. */
      case META_SEARCH_BY_EMAIL:
      case META_SEARCH_BY_EMAIL2:   /* wildcard search is wrapped to simple search */
             mr_search_by_email(pack, user, snac, req_seq, finfo, *tlv); break;

      case META_SEARCH_BY_EMAIL3:
             mr_search_by_email2(pack, user, snac, req_seq, finfo, *tlv); break;

      case META_SEARCH_BY_UIN2:
             mr_search_by_uin2(pack, user, snac, req_seq, finfo, *tlv); break;

      case META_SEARCH_BY_DETAILS:
      case META_SEARCH_BY_DETAILS2: /* wildcard search is wrapped to simple search */
             mr_search_by_details(pack, user, snac, req_seq, finfo, *tlv); break;
	    
      case META_SEARCH_WHITE:
      case META_SEARCH_WHITE2:      /* wildcard search is wrapped to simple search */
             mr_search_white(pack, user, snac, req_seq, finfo, *tlv); break;
	     
      case META_UNREGISTER_USER:
             mr_unregister_user(pack, user, snac, req_seq, finfo, *tlv); break;

      case META_SEARCH_WHITE3:
    	    mr_search_white2(pack, user, snac, req_seq, finfo, *tlv); break;
	    
      case META_SEARCH_RANDOM:     /* This is only stub */
            mr_search_random(pack, user, snac, req_seq, finfo, *tlv); break;

      /* [03.05.2003] New way to update own information, currently used by ICQLite */
      /*              It just contain TLV chain with data client want to change    */
      case META_INFO_SET_INFO:
            mr_set_user_info(pack, user, snac, req_seq, finfo, *tlv); break;
      
      /* This is just some stats/random garbage...  So we can skip this */
      case META_INFO_SET_RANDOM:
      case META_STAT_0A8C:
      case META_STAT_0A96:
      case META_STAT_0AAA:
      case META_STAT_0AB4:
      case META_STAT_0AB9:
      case META_STAT_0ABE:
      case META_STAT_0AC8:
      case META_STAT_0ACD:
      case META_STAT_0AD2:
      case META_STAT_0AD7: /* ignore */ break;
      
      case META_INFO_SEARCH_REQ:      /* new utf8 request */
             mr_search_info_req(pack, user, snac, req_seq, finfo, *tlv);
	      break;

      case META_INFO_SET_REQ:      /* new utf8 set request */
             mr_utf8_set_req(pack, user, snac, req_seq, finfo, *tlv);
	      break;


      default:  DEBUG(10, ("WARN: Unknown info-request type %04X from %s\n",
                      sub_cmd, inet_ntoa(pack.from_ip))); 
		      log_alarm_packet(0, pack);
		      break;
   }
}


/**************************************************************************/
/* Update user information command processor.  			  	  */
/**************************************************************************/
void mr_set_user_info(Packet &pack, struct online_user *user, 
                      struct snac_header &snac, unsigned short req_seq, 
		      struct full_user_info &finfo, class tlv_c &tlv)
{
   /* Ok, now some information about this command. First it was explored */
   /* in ICQLite 1066 session. This snac contain tlv chain with data     */
   /* client wants to change. Client may use it to change all its info   */
   /* or only part of it. This tlv chain may contain several TLVs of one */
   /* type. For example it may contain three TLV(0x0186) - spoken lang   */
   /* or several TLV(0x015E) - email.                                    */
   PGresult *res;
   int i;
   char ctemp;
   unsigned short stemp, stemp2, stemp3;
   unsigned long ltemp;
   unsigned short int_num = 0;
   unsigned short tlv_num = 0;
   unsigned short aff_num = 0;
   unsigned short pst_num = 0;
   BOOL is_first_clause = True;
   BOOL save_error = False;
   class tlv_chain_c tlv_chain;
   struct notes_user_info notes;
   class tlv_c *stlv;
   char clause_temp[512];
   
   tlv_chain.readXXX(tlv);
   tlv_chain.intel_order();
   
   DEBUG(50, ("SaveInfo tlv-chain tlvs num=%d\n", tlv_chain.num()));

   /* first we should take user notice field (max - 500 chars) */
   if ((stlv = tlv_chain.get(INF_TLV_USERNOTES)) != NULL)
   {
       if (v7_extract_string(notes.notes, *stlv, sizeof(notes.notes)-1, 
                             "si notes", *user, pack))
       {
          if (db_users_setnotes(user->uin, notes) == -1) save_error = True;
          tlv_chain.remove(INF_TLV_USERNOTES);
       }
       else save_error = True;
   }

   /* First part of the SQL update command */   
   strncpy(tempst, "UPDATE Users_Info_Ext Set ", 128);
   
   /* === Field: nick name ---> TLV(0x0154) */
   if (((stlv = tlv_chain.get(INF_TLV_NICKNAME)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.nick, *stlv, sizeof(finfo.nick)-1, 
                            "si nick", *user, pack))
      {
         convert_to_postgres(finfo.nick, sizeof(finfo.nick)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "nick='%s'", finfo.nick);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_NICKNAME);
      }
      else save_error = True;
   }

   /* === Field: first name ---> TLV(0x0140) */
   if (((stlv = tlv_chain.get(INF_TLV_FIRSTNAME)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.first, *stlv, sizeof(finfo.first)-1, 
                            "si first", *user, pack))
      {
         convert_to_postgres(finfo.first, sizeof(finfo.first)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "frst='%s'", finfo.first);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_FIRSTNAME);
      }
      else save_error = True;
   }

   /* === Field: last name ---> TLV(0x014A) */
   if (((stlv = tlv_chain.get(INF_TLV_LASTNAME)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.last, *stlv, sizeof(finfo.last)-1, 
                            "si last", *user, pack))
      {
         convert_to_postgres(finfo.last, sizeof(finfo.last)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "last='%s'", finfo.last);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_LASTNAME);
      }
      else save_error = True;
   }

   /* === Fields: email1,email2,email3 ---> TLV(0x015E) */
   tlv_num = tlv_chain.num(INF_TLV_EMAIL);
   for (i=0;i<tlv_num-3;i++) tlv_chain.remove(INF_TLV_EMAIL);
   
   for (i=3;i>0;i--)
   {
      if (((stlv = tlv_chain.get(INF_TLV_EMAIL)) != NULL) && 
           (save_error != True))
      {
         if (v7_extract_string(finfo.email1, *stlv, sizeof(finfo.email1)-1, 
                              "si emailx", *user, pack))
         {
	    *stlv >> ctemp;
            convert_to_postgres(finfo.email1, sizeof(finfo.email1)-1);

	    if (i == 1)
	    {
               snprintf(clause_temp, sizeof(clause_temp)-1, 
	                "email%d='%s',e1publ=%d", i, finfo.email1, ctemp);
	    }
	    else
	    {
	       snprintf(clause_temp, sizeof(clause_temp)-1, 
	                "email%d='%s'", i, finfo.email1);
	    }
	    
            if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
            safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
            is_first_clause = False;
	    tlv_chain.remove(INF_TLV_EMAIL);
         }
         else save_error = True;
      }
   }

   /* === Field: home city ---> TLV(0x0190) */
   if (((stlv = tlv_chain.get(INF_TLV_HOMECITY)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.hcity, *stlv, sizeof(finfo.hcity)-1, 
                            "si hcity", *user, pack))
      {
         convert_to_postgres(finfo.hcity, sizeof(finfo.hcity)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "hcity='%s'", finfo.hcity);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_HOMECITY);
      }
      else save_error = True;
   }

   /* === Field: home state ---> TLV(0x019A) */
   if (((stlv = tlv_chain.get(INF_TLV_HOMESTATE)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.hstate, *stlv, sizeof(finfo.hstate)-1, 
                            "si hstate", *user, pack))
      {
         convert_to_postgres(finfo.hstate, sizeof(finfo.hstate)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "hstate='%s'", finfo.hstate);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_HOMESTATE);
      }
      else save_error = True;
   }

   /* === Field: work company ---> TLV(0x01AE) */
   if (((stlv = tlv_chain.get(INF_TLV_WCOMPANY)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.wcompany, *stlv, sizeof(finfo.wcompany)-1, 
                            "si wcompany", *user, pack))
      {
         convert_to_postgres(finfo.wcompany, sizeof(finfo.wcompany)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "wcompany='%s'", finfo.wcompany);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_WCOMPANY);
      }
      else save_error = True;
   }

   /* === Field: work department ---> TLV(0x01B8) */
   if (((stlv = tlv_chain.get(INF_TLV_WDEPARTMENT)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.wdepart2, *stlv, sizeof(finfo.wdepart2)-1, 
                            "si wdepartment", *user, pack))
      {
         convert_to_postgres(finfo.wdepart2, sizeof(finfo.wdepart2)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "wdepart2='%s'", finfo.wdepart2);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_WDEPARTMENT);
      }
      else save_error = True;
   }

   /* === Field: work position ---> TLV(0x01C2) */
   if (((stlv = tlv_chain.get(INF_TLV_WPOSITION)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.wtitle, *stlv, sizeof(finfo.wtitle)-1, 
                            "si wposition", *user, pack))
      {
         convert_to_postgres(finfo.wtitle, sizeof(finfo.wtitle)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "wtitle='%s'", finfo.wtitle);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_WPOSITION);
      }
      else save_error = True;
   }

   /* === Field: home webpage url ---> TLV(0x0213) */
   if (((stlv = tlv_chain.get(INF_TLV_HOMEWEBPAGE)) != NULL) && 
        (save_error != True))
   {
      *stlv >> stemp; /* homepage category ??? - i'm not sure */
      
      /* Format was changed so we should check this */
      if ((stemp + SIZEOF_UNSIGNED_SHORT) == stlv->size)
      {
	 stlv->reset(); stlv->intel_order();
         DEBUG(50, ("New INF_TLV_HOMEWEBPAGE format detected... TLV Reset OK\n"));
      }
      
      if (v7_extract_string(finfo.hpage, *stlv, sizeof(finfo.hpage)-1, 
                            "si hpage", *user, pack))
      {
         convert_to_postgres(finfo.hpage, sizeof(finfo.hpage)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "hweb='%s'", finfo.hpage);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_HOMEWEBPAGE);
      }
      else save_error = True;
   }

   /* === Field: home street address ---> TLV(0x0262) */
   if (((stlv = tlv_chain.get(INF_TLV_HOMEADDR)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.haddr, *stlv, sizeof(finfo.haddr)-1, 
                            "si haddr", *user, pack))
      {
         convert_to_postgres(finfo.haddr, sizeof(finfo.haddr)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "haddr='%s'", finfo.haddr);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_HOMEADDR);
      }
      else save_error = True;
   }

   /* === Field: home phone ---> TLV(0x0276) */
   if (((stlv = tlv_chain.get(INF_TLV_HOMEPHONE)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.hphone, *stlv, sizeof(finfo.hphone)-1, 
                            "si hphone", *user, pack))
      {
         convert_to_postgres(finfo.hphone, sizeof(finfo.hphone)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "hphon='%s'", finfo.hphone);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_HOMEPHONE);
      }
      else save_error = True;
   }

   /* === Field: home fax ---> TLV(0x0280) */
   if (((stlv = tlv_chain.get(INF_TLV_HOMEFAX)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.hfax, *stlv, sizeof(finfo.hfax)-1, 
                            "si hfax", *user, pack))
      {
         convert_to_postgres(finfo.hfax, sizeof(finfo.hfax)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "hfax='%s'", finfo.hfax);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_HOMEFAX);
      }
      else save_error = True;
   }

   /* === Field: cellular phone ---> TLV(0x028A) */
   if (((stlv = tlv_chain.get(INF_TLV_CELLULAR)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.hcell, *stlv, sizeof(finfo.hcell)-1, 
                            "si hcell", *user, pack))
      {
         convert_to_postgres(finfo.hcell, sizeof(finfo.hcell)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "hcell='%s'", finfo.hcell);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_CELLULAR);
      }
      else save_error = True;
   }

   /* === Field: work address ---> TLV(0x0294) */
   if (((stlv = tlv_chain.get(INF_TLV_WORKADDR)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.waddr, *stlv, sizeof(finfo.waddr)-1, 
                            "si waddr", *user, pack))
      {
         convert_to_postgres(finfo.waddr, sizeof(finfo.waddr)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "waddr='%s'", finfo.waddr);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_WORKADDR);
      }
      else save_error = True;
   }

   /* === Field: work city ---> TLV(0x029E) */
   if (((stlv = tlv_chain.get(INF_TLV_WORKCITY)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.wcity, *stlv, sizeof(finfo.wcity)-1, 
                            "si wcity", *user, pack))
      {
         convert_to_postgres(finfo.wcity, sizeof(finfo.wcity)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "wcity='%s'", finfo.wcity);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_WORKCITY);
      }
      else save_error = True;
   }

   /* === Field: work state ---> TLV(0x02A8) */
   if (((stlv = tlv_chain.get(INF_TLV_WORKSTATE)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.wstate, *stlv, sizeof(finfo.wstate)-1, 
                            "si wstate", *user, pack))
      {
         convert_to_postgres(finfo.wstate, sizeof(finfo.wstate)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "wstate='%s'", finfo.wstate);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_WORKSTATE);
      }
      else save_error = True;
   }

   /* === Field: work phone ---> TLV(0x02C6) */
   if (((stlv = tlv_chain.get(INF_TLV_WORKPHONE)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.wphone, *stlv, sizeof(finfo.wphone)-1, 
                            "si wphone", *user, pack))
      {
         convert_to_postgres(finfo.wphone, sizeof(finfo.wphone)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "wphon='%s'", finfo.wphone);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_WORKPHONE);
      }
      else save_error = True;
   }

   /* === Field: work fax ---> TLV(0x02D0) */
   if (((stlv = tlv_chain.get(INF_TLV_WORKFAX)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.wfax, *stlv, sizeof(finfo.wfax)-1, 
                            "si wfax", *user, pack))
      {
         convert_to_postgres(finfo.wfax, sizeof(finfo.wfax)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "wfax='%s'", finfo.wfax);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_WORKFAX);
      }
      else save_error = True;
   }

   /* === Field: work web page ---> TLV(0x02DA) */
   if (((stlv = tlv_chain.get(INF_TLV_WORKWEBPAGE)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.wpage, *stlv, sizeof(finfo.wpage)-1, 
                            "si wpage", *user, pack))
      {
         convert_to_postgres(finfo.wpage, sizeof(finfo.wpage)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "wweb='%s'", finfo.wpage);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_WORKWEBPAGE);
      }
      else save_error = True;
   }

   /* === Field: age ---> TLV(0x0172) */
   if (((stlv = tlv_chain.get(INF_TLV_AGE)) != NULL) && 
        (save_error != True))
   {
      *stlv >> stemp;
       snprintf(clause_temp, 255, "age=%d", (short)stemp);
       if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
       safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
       is_first_clause = False;
       tlv_chain.remove(INF_TLV_AGE);
   }

   /* === Field: gender ---> TLV(0x017C) */
   if (((stlv = tlv_chain.get(INF_TLV_GENDER)) != NULL) && 
        (save_error != True))
   {
      *stlv >> ctemp;
       snprintf(clause_temp, 255, "sex=%d", ctemp);
       if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
       safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
       is_first_clause = False;
       tlv_chain.remove(INF_TLV_GENDER);
   }

   /* === Field: spoken language ---> TLV(0x0186) */
   tlv_num = tlv_chain.num(INF_TLV_SPOKENLANG);
   for (i=0;i<tlv_num-3;i++) tlv_chain.remove(INF_TLV_SPOKENLANG);
   
   for (i=3;i>0;i--)
   {
      if (((stlv = tlv_chain.get(INF_TLV_SPOKENLANG)) != NULL) && 
           (save_error != True))
      {
         *stlv >> ctemp;
          snprintf(clause_temp, 255, "lang%d=%d", i, ctemp);
          if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
          safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
          is_first_clause = False;
          tlv_chain.remove(INF_TLV_SPOKENLANG);
      }
   }
   
   /* === Field: home country ---> TLV(0x01A4) */
   if (((stlv = tlv_chain.get(INF_TLV_HOMECOUNTRY)) != NULL) && 
        (save_error != True))
   {
      *stlv >> stemp;
       snprintf(clause_temp, 255, "hcountry=%d", stemp);
       if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
       safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
       is_first_clause = False;
       tlv_chain.remove(INF_TLV_HOMECOUNTRY);
   }

   /* === Field: work ocupation code ---> TLV(0x01CC) */
   if (((stlv = tlv_chain.get(INF_TLV_WOCUPATION)) != NULL) && 
        (save_error != True))
   {
      *stlv >> stemp;
       snprintf(clause_temp, 255, "wocup=%d", (short)stemp);
       if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
       safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
       is_first_clause = False;
       tlv_chain.remove(INF_TLV_WOCUPATION);
   }

   /* === Field: home zip code ---> TLV(0x026C) */
   if (((stlv = tlv_chain.get(INF_TLV_HOMEZIP)) != NULL) && 
        (save_error != True))
   {
      *stlv >> ltemp;
       if ((ltemp < 1) || (ltemp > 999999)) ltemp = 1;
       snprintf(clause_temp, 255, "hzip='%lu'", ltemp);
       if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
       safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
       is_first_clause = False;
       tlv_chain.remove(INF_TLV_HOMEZIP);
   }

   /* === Field: work country ---> TLV(0x02B2) */
   if (((stlv = tlv_chain.get(INF_TLV_WORKCOUNTRY)) != NULL) && 
        (save_error != True))
   {
      *stlv >> stemp;
       snprintf(clause_temp, 255, "wcountry=%d", stemp);
       if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
       safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
       is_first_clause = False;
       tlv_chain.remove(INF_TLV_WORKCOUNTRY);
   }

   /* === Field: work zip code ---> TLV(0x02BC) */
   if (((stlv = tlv_chain.get(INF_TLV_WORKZIP)) != NULL) && 
        (save_error != True))
   {
      *stlv >> ltemp;
       if ((ltemp < 1) || (ltemp > 999999)) ltemp = 1;
       snprintf(clause_temp, 255, "wzip='%lu'", ltemp);
       if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
       safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
       is_first_clause = False;
       tlv_chain.remove(INF_TLV_WORKZIP);
   }

   /* === Field: webaware flag ---> TLV(0x02F8) */
   if (((stlv = tlv_chain.get(INF_TLV_WEBPERMS)) != NULL) && 
        (save_error != True))
   {
      *stlv >> ctemp;
       snprintf(clause_temp, 255, "webaware=%d", ctemp);
       if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
       safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
       is_first_clause = False;
       tlv_chain.remove(INF_TLV_WEBPERMS);
   }

   /* === Field: auth flag ---> TLV(0x030C) */
   if (((stlv = tlv_chain.get(INF_TLV_AUTH)) != NULL) && 
        (save_error != True))
   {
      *stlv >> ctemp;
       snprintf(clause_temp, 255, "auth=%d", ctemp);
       if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
       safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
       is_first_clause = False;
       tlv_chain.remove(INF_TLV_AUTH);
   }

   /* === Field: GMT offset ---> TLV(0x0316) */
   if (((stlv = tlv_chain.get(INF_TLV_GMTOFFSET)) != NULL) && 
        (save_error != True))
   {
      *stlv >> ctemp;
       snprintf(clause_temp, 255, "gmtoffs=%d", (short)ctemp);
       if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
       safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
       is_first_clause = False;
       tlv_chain.remove(INF_TLV_GMTOFFSET);
   }

   /* === Field: birthday ---> TLV(0x023A) */
   if (((stlv = tlv_chain.get(INF_TLV_BIRTHDAY)) != NULL) && 
        (save_error != True))
   {
      *stlv >> stemp >> stemp2 >> stemp3;
       snprintf(clause_temp, 255, "byear=%d,bmon=%d,bday=%d", 
                (short)stemp, (short)stemp2, (short)stemp3);
       if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
       safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
       is_first_clause = False;
       tlv_chain.remove(INF_TLV_BIRTHDAY);
   }

   /* === Field: interests ---> TLV(0x01EA) */
   tlv_num = tlv_chain.num(INF_TLV_INTERESTS);
   for (i=0;i<tlv_num-4;i++) tlv_chain.remove(INF_TLV_INTERESTS);
   
   for (i=4;i>0;i--)
   {
      if (((stlv = tlv_chain.get(INF_TLV_INTERESTS)) != NULL) && 
           (save_error != True))
      {
         *stlv >> stemp; /* interest category */
      
         /* We'll use hpage text buffer for interests keywords string */
         if (v7_extract_string(finfo.hpage, *stlv, sizeof(finfo.hpage)-1, 
                               "si interest", *user, pack))
         {
            convert_to_postgres(finfo.hpage, sizeof(finfo.hpage)-1);
            snprintf(clause_temp, sizeof(clause_temp)-1, 
	             "int_ind%d=%d,int_key%d='%s'", i, stemp, i, finfo.hpage);
            if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
            safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
            is_first_clause = False;
            tlv_chain.remove(INF_TLV_INTERESTS);
	    int_num++;
         }
         else save_error = True;
      }
   }
   
   if (int_num > 0)
   {
      snprintf(clause_temp, sizeof(clause_temp)-1, ",int_num=%d", int_num);
      safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
   }

   /* === Field: affilations ---> TLV(0x01D6) */
   tlv_num = tlv_chain.num(INF_TLV_AFFILATIONS);
   for (i=0;i<tlv_num-3;i++) tlv_chain.remove(INF_TLV_AFFILATIONS);
   
   for (i=3;i>0;i--)
   {
      if (((stlv = tlv_chain.get(INF_TLV_AFFILATIONS)) != NULL) && 
           (save_error != True))
      {
         *stlv >> stemp; /* affilations category */
      
         /* We'll use hpage text buffer for affilations keywords string */
         if (v7_extract_string(finfo.hpage, *stlv, sizeof(finfo.hpage)-1, 
                               "si affilation", *user, pack))
         {
            convert_to_postgres(finfo.hpage, sizeof(finfo.hpage)-1);
            snprintf(clause_temp, sizeof(clause_temp)-1, 
	             "aff_ind%d=%d,aff_key%d='%s'", i, stemp, i, finfo.hpage);
            if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
            safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
            is_first_clause = False;
            tlv_chain.remove(INF_TLV_AFFILATIONS);
	    aff_num++;
         }
         else save_error = True;
      }
   }
   
   if (aff_num > 0)
   {
      snprintf(clause_temp, sizeof(clause_temp)-1, ",aff_num=%d", aff_num);
      safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
   }

   /* === Field: past info ---> TLV(0x01FE) */
   tlv_num = tlv_chain.num(INF_TLV_PASTINFO);
   for (i=0;i<tlv_num-3;i++) tlv_chain.remove(INF_TLV_PASTINFO);
   
   for (i=3;i>0;i--)
   {
      if (((stlv = tlv_chain.get(INF_TLV_PASTINFO)) != NULL) && 
           (save_error != True))
      {
         *stlv >> stemp; /* past category */
      
         /* We'll use hpage text buffer for past info keywords string */
         if (v7_extract_string(finfo.hpage, *stlv, sizeof(finfo.hpage)-1, 
                               "si past info", *user, pack))
         {
            convert_to_postgres(finfo.hpage, sizeof(finfo.hpage)-1);
            snprintf(clause_temp, sizeof(clause_temp)-1, 
	             "past_ind%d=%d,past_key%d='%s'", i, stemp, i, finfo.hpage);
            if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
            safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
            is_first_clause = False;
            tlv_chain.remove(INF_TLV_PASTINFO);
	    pst_num++;
         }
         else save_error = True;
      }
   }
   
   if (pst_num > 0)
   {
      snprintf(clause_temp, sizeof(clause_temp)-1, ",past_num=%d", pst_num);
      safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
   }
   
   /* === Field: martial status ---> TLV(0x033E) */
   if (((stlv = tlv_chain.get(INF_TLV_MARTIAL)) != NULL) && (save_error != True))
   {
      *stlv >> ctemp;
      snprintf(clause_temp, 255, "martial=%d", ctemp);
      if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
      safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
      is_first_clause = False;
      tlv_chain.remove(INF_TLV_MARTIAL);
   }

   /* === Field: new work zip code ---> TLV(0x02BD) */
   if (((stlv = tlv_chain.get(INF_TLV_WORKZIP2)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.wzip2, *stlv, sizeof(finfo.wzip2)-1, 
                            "si wzip2", *user, pack))
      {
         convert_to_postgres(finfo.wzip2, sizeof(finfo.wzip2)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "wzip2='%s'", finfo.wzip2);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_WORKZIP2);
      }
   }

   /* === Field: new home zip code ---> TLV(0x026D) */
   if (((stlv = tlv_chain.get(INF_TLV_HOMEZIP2)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(finfo.hzip2, *stlv, sizeof(finfo.hzip2)-1, 
                            "si hzip2", *user, pack))
      {
         convert_to_postgres(finfo.hzip2, sizeof(finfo.hzip2)-1);
         snprintf(clause_temp, sizeof(clause_temp)-1, "hzip2='%s'", finfo.hzip2);
         if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
         safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
         is_first_clause = False;
	 tlv_chain.remove(INF_TLV_HOMEZIP2);
      }
   }

   /* Last part of the query - WHERE clause */
   snprintf(clause_temp, 255, " WHERE uin=%lu", user->uin);
   safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
   
   DEBUG(10, ("SQLQ: %s\n", tempst));
   
   /* Currently skipped fields */
   tlv_chain.remove(INF_TLV_OCITY);
   tlv_chain.remove(INF_TLV_OSTATE);
   tlv_chain.remove(INF_TLV_OCOUNTRY);
   
   /* Fuck... we can handle only three emails :( - db limitations */
   if ((stlv = tlv_chain.get(INF_TLV_EMAIL)) != NULL) save_error = True;
   if (is_first_clause) save_error = True;

   /* Check if we have unhandled TLVs in chain */
   if ((save_error != True) && (tlv_chain.num() != 0))
   {
      DEBUG(10, ("SI (for %lu) changed, added %d tlvs, please report ICQ version\n", user->uin, tlv_chain.num()));
      LOG_SYS(0, ("SI (for %lu) changed, added %d tlvs, please report ICQ version\n", user->uin, tlv_chain.num()));
   }

   /* execute database SQL query */
   if (save_error != True)
   {
      res = PQexec(users_dbconn, tempst);
      if (PQresultStatus(res) != PGRES_COMMAND_OK)
      {
         handle_database_error(res, "[SET INFO FULL]");
         save_error = True;
      }
      else
      {
         if (atoul(PQcmdTuples(res)) == 0) save_error = True;
         PQclear(res);  
      }
   }

   /* Finish him!!! (send saveinfo result to client) */   
   if (save_error)
   {
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SET_RESULT, False, 0);
   }
   else
   {
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SET_RESULT, True, 0);
   }
}

/**************************************************************************/
/* Random user search stub		 			  	  */
/**************************************************************************/
void mr_search_random(Packet &pack, struct online_user *user, 
                      struct snac_header &snac, unsigned short req_seq, 
		      struct full_user_info &finfo, class tlv_c &tlv)
{
   unsigned short group;
   tlv >> group;

   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_ICQxMESSxEXT
          << (unsigned short)SN_IME_MULTIxMESSxRESP
	  << (unsigned short)0x0000
	  << (unsigned  long)snac.id;

   /* I hate ICQ... I think AOL staff love to fuck standing in a hammock */
   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated(); 
   
   reply_pack << (unsigned  long)user->uin
	      << (unsigned short)META_RESP_INFORMATION
	      << (unsigned short)req_seq
	      << (unsigned short)META_RANDOM_FOUND
              << (char)META_EMPTY;
   	       
   /* now time to put data into arpack */
   arpack << (unsigned short)0x0001             
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending random search reply to user %lu [stub]\n", user->uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
   
}


/**************************************************************************/
/* Unregister user packet handler	 			  	  */
/**************************************************************************/
void mr_unregister_user(Packet &pack, struct online_user *user, 
                        struct snac_header &snac, unsigned short req_seq, 
		        struct full_user_info &finfo, class tlv_c &tlv)
{
   unsigned long uin;
   ffstring password;

   tlv >> uin;
   
   v7_extract_string(password, tlv, 32, "v7 unreg password", *user, pack);
   
   if (db_users_delete_user(uin, password) != -1) 
   {
      LOG_USR(0, ("User %lu deleted from users_info table [unregistration]\n", user->uin));
      mr_send_set_ack(pack, user, snac, req_seq, META_UNREGISTER_ACK, True, 0);
      /* should I drop connection to this user here ? */
   } 
   else
   {            
      LOG_USR(0, ("Error unregistering user %lu", user->uin));
      mr_send_set_ack(pack, user, snac, req_seq, META_UNREGISTER_ACK, False, 0);
   }
}


/**************************************************************************/
/* White pages search			 			  	  */
/**************************************************************************/
void mr_search_white(Packet &pack, struct online_user *user, 
                     struct snac_header &snac, unsigned short req_seq, 
		     struct full_user_info &finfo, class tlv_c &tlv)
{
   char dbcomm_str[4096];
   int fsend, keys_cnt;
   char *keys_ptr;
   fstring clause_temp, pkeys, interests_dir, affkeys, pagekeys, token;
   ffstring nick_str, first_str, last_str, email, city, state, company;
   ffstring department, position;
   unsigned short min_age, max_age, country, pcode;
   unsigned short int_index, aff_index, page_index;
   char gender, language, work_code, online_only;
   PGresult *res;
   BOOL not_implemented, at_least_one_key;

   send_event2ap(papack, ACT_SEARCH, user->uin, user->status,
                 ipToIcq(user->ip), 52, longToTime(time(NULL)), "");

   /* first block - personal information clauses */
   v7_extract_string(first_str, tlv, 32, "v7 wp frst", *user, pack);
   convert_to_postgres(first_str, sizeof(first_str));
   v7_extract_string(last_str,  tlv, 32, "v7 wp last", *user, pack);
   convert_to_postgres(last_str, sizeof(last_str));
   v7_extract_string(nick_str,  tlv, 32, "v7 wp nick", *user, pack);
   convert_to_postgres(nick_str, sizeof(nick_str));  
   v7_extract_string(email,  tlv, 63, "v7 wp email", *user, pack);
   convert_to_postgres(email, sizeof(email));
   
   /* Second block - age, gender, language information */
   tlv >> min_age >> max_age;
   tlv >> gender  >> language;
   
   /* Third block - location information */
   v7_extract_string(city,  tlv, 32, "v7 wp city", *user, pack);
   convert_to_postgres(city, sizeof(city));
   v7_extract_string(state, tlv, 32, "v7 wp state", *user, pack);
   convert_to_postgres(state, sizeof(state));
   tlv >> country;
   
   /* fourth block - work position information */
   v7_extract_string(company, tlv, 32, "v7 wp company", *user, pack);
   convert_to_postgres(company, sizeof(company));
   v7_extract_string(department, tlv, 32, "v7 wp department", *user, pack);
   convert_to_postgres(department, sizeof(department));
   v7_extract_string(position, tlv, 32, "v7 wp position", *user, pack);
   convert_to_postgres(position, sizeof(position));
   tlv >> work_code;

   /* fifth block - past information */
   tlv >> pcode;
   v7_extract_string(pkeys, tlv, 63, "v7 wp past", *user, pack);
   convert_to_postgres(pkeys, sizeof(pkeys));

   /* sixth block - interests information */   
   tlv >> int_index;
   v7_extract_string(interests_dir, tlv, 127, "v7 wp intdir", *user, pack);
   convert_to_postgres(interests_dir, sizeof(interests_dir));
   
   /* seventh block - affilations information */
   tlv >> aff_index;
   v7_extract_string(affkeys, tlv, 127, "v7 wp affilations", *user, pack);
   convert_to_postgres(affkeys, sizeof(affkeys));

   tlv >> page_index;
   v7_extract_string(pagekeys, tlv, 127, "v7 wp page", *user, pack);
   convert_to_postgres(pagekeys, sizeof(pagekeys));

   DEBUG(10, ("affkeys=%s, v7 page keys=%s\n", affkeys, pagekeys));
   
   tlv >> online_only;

   /* now all information is extracted from packet and we can preform      */
   /* white_pages search in database. There will be very complex SQL query */

   /* first query part - constant */
   not_implemented = True;
   
   snprintf(dbcomm_str, 4095, "SELECT uin,nick,frst,last,email2,auth,email1,e1publ,webaware FROM Users_Info_Ext WHERE (1=1) ");
   
   if (strlen(nick_str) != 0) 
   {
      snprintf(clause_temp, 255, "AND (upper(nick) like upper('%%%s%%')) ", nick_str);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   if (strlen(first_str) != 0) 
   {
      snprintf(clause_temp, 255, "AND (upper(frst) like upper('%%%s%%')) ", first_str);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   if (strlen(last_str) != 0) 
   {
      snprintf(clause_temp, 255, "AND (upper(last) like upper('%%%s%%')) ", last_str);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   if (strlen(email) != 0) 
   {
      snprintf(clause_temp, 255, "AND ((upper(email2) like upper('%%%s%%')) ", email);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      snprintf(clause_temp, 255, "OR (upper(email3) like upper('%%%s%%'))) ", email);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   if ((min_age != 0) && (max_age != 0) && (min_age < 20000) && (max_age < 20000)) 
   {
      snprintf(clause_temp, 255, "AND ((age >= %d) AND (age <= %d)) ", min_age, max_age);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   if ((gender < 16) && (gender > 0))
   {
      snprintf(clause_temp, 255, "AND (sex = %d) ", gender);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   if ((language > 0) && (language < 127))
   {
      snprintf(clause_temp, 255, "AND (%d in (lang1, lang2, lang3)) ", 
               language);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   if (strlen(city) != 0) 
   {
      snprintf(clause_temp, 255, "AND (upper(hcity) like upper('%%%s%%')) ", city);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   if (strlen(state) != 0) 
   {
      snprintf(clause_temp, 255, "AND ((upper(hstate) like upper('%%%s%%')) OR (upper(wstate) like upper('%%%s%%'))) ", state, state);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   if ((country > 0) && (country < 20000)) 
   {
      snprintf(clause_temp, 255, "AND ((hcountry=%d) OR (wcountry=%d)) ", country, country);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   if (strlen(company) != 0) 
   {
      snprintf(clause_temp, 255, "AND (upper(wcompany) like upper('%%%s%%')) ", company);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   if (strlen(position) != 0) 
   {
      snprintf(clause_temp, 255, "AND (upper(wtitle) like upper('%%%s%%')) ", position);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   if ((work_code > 0) && (work_code < 127))
   {
      snprintf(clause_temp, 255, "AND (wocup = %d) ", work_code);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   if ((pcode > 0) && (pcode < 60000))
   {
      snprintf(clause_temp, 255, "AND (((past_ind1 = %d) AND (", pcode);
      safe_strcat(dbcomm_str, clause_temp, 4095);

      keys_ptr = (char *)pkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(past_key1) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4095);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4095);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4095);
      }

      safe_strcat(dbcomm_str, "OR ", 4095);
      snprintf(clause_temp, 255, "((past_ind2 = %d) AND (", pcode);
      safe_strcat(dbcomm_str, clause_temp, 4095);

      keys_ptr = (char *)pkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(past_key2) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4095);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4095);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4095);
      }

      safe_strcat(dbcomm_str, "OR ", 4095);
      snprintf(clause_temp, 255, "((past_ind3 = %d) AND (", pcode);
      safe_strcat(dbcomm_str, clause_temp, 4095);

      keys_ptr = (char *)pkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(past_key3) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4095);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False ))", 4095);
      }
      else
      {
         safe_strcat(dbcomm_str, "True ))", 4095);
      }

      safe_strcat(dbcomm_str, ")", 4095);
      not_implemented = False;
   }

   if ((aff_index > 0) && (aff_index < 60000))
   {
      snprintf(clause_temp, 255, "AND (((aff_ind1 = %d) AND (", aff_index);
      safe_strcat(dbcomm_str, clause_temp, 4095);

      keys_ptr = (char *)affkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(aff_key1) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4095);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4095);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4095);
      }

      safe_strcat(dbcomm_str, "OR ", 4095);
      snprintf(clause_temp, 255, "((aff_ind2 = %d) AND (", aff_index);
      safe_strcat(dbcomm_str, clause_temp, 4095);

      keys_ptr = (char *)affkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(aff_key2) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4095);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4095);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4095);
      }

      safe_strcat(dbcomm_str, "OR ", 4095);
      snprintf(clause_temp, 255, "((aff_ind3 = %d) AND (", aff_index);
      safe_strcat(dbcomm_str, clause_temp, 4095);

      keys_ptr = (char *)affkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(aff_key3) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4095);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4095);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4095);
      }


      safe_strcat(dbcomm_str, ")", 4095);
      not_implemented = False;
   }

   if ((int_index > 0) && (int_index < 60000))
   {
      snprintf(clause_temp, 255, "AND (((int_ind1 = %d) AND (", int_index);
      safe_strcat(dbcomm_str, clause_temp, 4095);

      keys_ptr = (char *)interests_dir;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(int_key1) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4095);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4095);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4095);
      }

      safe_strcat(dbcomm_str, "OR ", 4095);
      snprintf(clause_temp, 255, "((int_ind2 = %d) AND (", int_index);
      safe_strcat(dbcomm_str, clause_temp, 4095);

      keys_ptr = (char *)interests_dir;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(int_key2) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4095);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4095);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4095);
      }

      safe_strcat(dbcomm_str, "OR ", 4095);
      snprintf(clause_temp, 255, "((int_ind3 = %d) AND (", int_index);
      safe_strcat(dbcomm_str, clause_temp, 4095);

      keys_ptr = (char *)interests_dir;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(int_key3) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4095);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4095);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4095);
      }

      safe_strcat(dbcomm_str, "OR ", 4095);
      snprintf(clause_temp, 255, "((int_ind4 = %d) AND (", int_index);
      safe_strcat(dbcomm_str, clause_temp, 4095);

      keys_ptr = (char *)interests_dir;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(int_key4) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4095);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4095);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4095);
      }


      safe_strcat(dbcomm_str, ")", 4095);
      not_implemented = False;
   }


   if ((page_index > 0) && (page_index < 60000))
   {
      snprintf(clause_temp, 255, "AND (((hpage_cat=%d) AND (", page_index);
      safe_strcat(dbcomm_str, clause_temp, 4095);

      keys_ptr = (char *)pagekeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(hpage_txt) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4095);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4095);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4095);
      }

      safe_strcat(dbcomm_str, ")", 4095);
      not_implemented = False;
   }
   
   if (online_only) 
   {
      safe_strcat(dbcomm_str, "AND (uin IN (SELECT uin FROM online_users))", 4095);
   }
  
   if (not_implemented)
   {
       strncpy(finfo.nick,   "this ", 31);  
       strncpy(finfo.first,  "search ", 31);
       strncpy(finfo.email2, "yet", 31);
       strncpy(finfo.email1, "yet", 31);
       strncpy(finfo.last,   "not implemented", 31);
	
       finfo.uin = 0; 
       finfo.auth = 0;
       finfo.e1publ = 0;
       finfo.webaware = 0;

       mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 0, True, 0);
       return;
   }
  
   send_event2ap(papack, ACT_SEARCH, user->uin, user->status,                
                 ipToIcq(user->ip), 55, longToTime(time(NULL)), "v7 wp search");

   /* now query is ready - we can run it */
   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[V7 WP SEARCH]");
      DEBUG(10, ("req: %s\n", dbcomm_str));
      mr_send_wp_found(pack, user, snac, req_seq, finfo, False, 0, True, 0);
      return;
   }

   fsend = PQntuples(res);
    
   DEBUG(10, ("Search found %d users\n", fsend));
    
   if (fsend == 0) 
   {
      mr_send_wp_found(pack, user, snac, req_seq, finfo, False, 0, True, 0);
   }

   for(int i=0; i<fsend; i++)
   {      
      strncpy(finfo.nick,   PQgetvalue(res, i,  1), sizeof(finfo.nick)-1);
      strncpy(finfo.first,  PQgetvalue(res, i,  2), sizeof(finfo.first)-1);
      strncpy(finfo.last,   PQgetvalue(res, i,  3), sizeof(finfo.last)-1);    
      strncpy(finfo.email2, PQgetvalue(res, i,  4), sizeof(finfo.email2)-1);
      strncpy(finfo.email1, PQgetvalue(res, i,  6), sizeof(finfo.email1)-1);

      ITrans.translateToClient(finfo.nick);
      ITrans.translateToClient(finfo.first);
      ITrans.translateToClient(finfo.last);
      ITrans.translateToClient(finfo.email2);
      ITrans.translateToClient(finfo.email1);
	 
      finfo.uin  = atoul(PQgetvalue(res, i, 0));
      finfo.auth  = atol(PQgetvalue(res, i,  5));
      finfo.e1publ  = atol(PQgetvalue(res, i, 7));
      finfo.webaware = atol(PQgetvalue(res, i, 8));
       
      if ((i == (fsend - 1)) | (i == (lp_v7_maxsearch()-1))) 
      {
         if (fsend > lp_v7_maxsearch())
	 {
	    /* limit: last user found, but there is more users */
	    mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 0, True, 1);
	 }
	 else
	 {
	    /* last user found, there is no more users */
	    mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 1, True, 0);
	 }
      }
      else
      {
         /* next user found */
	 mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 1, False, 0);
      }
       
      if (i == (lp_v7_maxsearch()-1)) break;
      if (((i % 100) == 0) & (i != 0)) results_delay(100);

    }
      
    PQclear(res);

}


/**************************************************************************/
/* Search users by details		 			  	  */
/**************************************************************************/
void mr_search_by_details(Packet &pack, struct online_user *user, 
                          struct snac_header &snac, unsigned short req_seq, 
		          struct full_user_info &finfo, class tlv_c &tlv)
{
   cstring dbcomm_str;
   int fsend;
   fstring nick_str, first_str, last_str;
   fstring clause1, clause2, clause3;
   PGresult *res;

   v7_extract_string(first_str, tlv, 32, "v7 frst s_string", *user, pack);
   v7_extract_string(last_str,  tlv, 32, "v7 last s_string", *user, pack);
   v7_extract_string(nick_str,  tlv, 32, "v7 nick s_string", *user, pack);
   
   convert_to_postgres(nick_str, sizeof(nick_str));
   convert_to_postgres(first_str, sizeof(first_str));
   convert_to_postgres(last_str, sizeof(last_str));
   
   /* now we have all data from packet - we should build db query */
   if (strlen(nick_str) != 0) 
   {
      snprintf(clause1, 64, "(upper(nick) like upper('%%%s%%'))", nick_str);
   }
   else
   {
      strncpy(clause1, "", 2);
   }

   if (strlen(first_str) != 0) 
   {
      if (strlen(nick_str) != 0)
      {
         snprintf(clause2, 64, "AND (upper(frst) like upper('%%%s%%'))", 
	                        first_str);
      }
      else
      {
         snprintf(clause2, 64, "(upper(frst) like upper('%%%s%%'))",
			        first_str);
      }
   }
   else
   {
      strncpy(clause2, "", 2);
   }

   if (strlen(last_str) != 0) 
   {
      if ((strlen(nick_str) != 0) | (strlen(first_str) != 0))
      {
         snprintf(clause3, 64, "AND (upper(last) like upper('%%%s%%'))", 
	                        last_str);
      }
      else
      {
         snprintf(clause3, 64, "(upper(last) like upper('%%%s%%'))",
	                        last_str);
      }			
   }
   else
   {
      strncpy(clause3, "", 2);
   }
   

   /* make sql query string */
   snprintf(dbcomm_str, 255, 
           "SELECT uin,nick,frst,last,email2,auth,email1,e1publ,webaware FROM Users_Info_Ext WHERE %s %s %s LIMIT %d", 
	    clause1, clause2, clause3, lp_v7_maxsearch()+1);

   snprintf(clause1, 250, "%s %s %s", nick_str, first_str, last_str);
   send_event2ap(papack, ACT_SEARCH, user->uin, user->status,                
                 ipToIcq(user->ip), 55, longToTime(time(NULL)), clause1);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[V7 SEARCH BY NAME]");
      DEBUG(10, ("req: %s\n", dbcomm_str));
      mr_send_wp_found(pack, user, snac, req_seq, finfo, False, 0, True, 0);
      return;
   }

   fsend = PQntuples(res);
    
   DEBUG(10, ("Search found %d users\n", fsend));
    
   if (fsend == 0) 
   {
      mr_send_wp_found(pack, user, snac, req_seq, finfo, False, 0, True, 0);
   }

   for(int i=0; i<fsend; i++)
   {      
      strncpy(finfo.nick,   PQgetvalue(res, i,  1), sizeof(finfo.nick)-1);
      strncpy(finfo.first,  PQgetvalue(res, i,  2), sizeof(finfo.first)-1);
      strncpy(finfo.last,   PQgetvalue(res, i,  3), sizeof(finfo.last)-1);    
      strncpy(finfo.email2, PQgetvalue(res, i,  4), sizeof(finfo.email2)-1);
      strncpy(finfo.email1, PQgetvalue(res, i,  6), sizeof(finfo.email1)-1);

      ITrans.translateToClient(finfo.nick);
      ITrans.translateToClient(finfo.first);
      ITrans.translateToClient(finfo.last);
      ITrans.translateToClient(finfo.email2);
      ITrans.translateToClient(finfo.email1);
	 
      finfo.uin  = atoul(PQgetvalue(res, i, 0));
      finfo.auth  = atol(PQgetvalue(res, i,  5));
      finfo.e1publ  = atol(PQgetvalue(res, i, 7));
      finfo.webaware = atol(PQgetvalue(res, i, 8));
       
      if ((i == (fsend - 1)) | (i == (lp_v7_maxsearch()-1))) 
      {
         if (fsend > lp_v7_maxsearch())
	 {
	    /* limit: last user found, but there is more users */
	    mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 0, True, 1);
	 }
	 else
	 {
	    /* last user found, there is no more users */
	    mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 0, True, 0);
	 }
      }
      else
      {
         /* next user found */
	 mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 1, False, 0);
      }
       
      if (i == (lp_v7_maxsearch()-1)) break;
      if (((i % 100) == 0) & (i != 0)) results_delay(100);

    }
      
    PQclear(res);

}


/**************************************************************************/
/* Search users by email		 			  	  */
/**************************************************************************/
void mr_search_by_email(Packet &pack, struct online_user *user, 
                        struct snac_header &snac, unsigned short req_seq, 
		        struct full_user_info &finfo, class tlv_c &tlv)
{

   cstring dbcomm_str;
   int fsend;
   fstring email_str;
   PGresult *res;

   v7_extract_string(email_str, tlv, 127, "search email", *user, pack);
   
   DEBUG(10, ("V5 search by email: %s\n", email_str));
   
   send_event2ap(papack, ACT_SEARCH, user->uin, user->status,
                 ipToIcq(user->ip), 54, longToTime(time(NULL)), email_str);   
   
   convert_to_postgres(email_str, sizeof(email_str));   

   /* make sql query string */
   snprintf(dbcomm_str, 255, 
           "SELECT uin,nick,frst,last,email2,auth,email1,webaware,e1publ FROM Users_Info_Ext WHERE (email2 like '%%%s%%') OR (email1 like '%%%s%%') LIMIT %d", 
	    email_str, email_str, lp_v7_maxsearch()+1);
     
   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[V7 SEARCH BY EMAIL]");
      mr_send_wp_found(pack, user, snac, req_seq, finfo, False, 0, True, 0);
      return;
   }

   fsend = PQntuples(res);
    
   DEBUGADD(0, ("Search found %d users\n", fsend));
    
   if (fsend == 0) 
   {
      mr_send_wp_found(pack, user, snac, req_seq, finfo, False, 0, True, 0);
   }

   for(int i=0; i<fsend; i++)
   {      
      strncpy(finfo.nick,   PQgetvalue(res, i,  1), sizeof(finfo.nick)-1);
      strncpy(finfo.first,  PQgetvalue(res, i,  2), sizeof(finfo.first)-1);
      strncpy(finfo.last,   PQgetvalue(res, i,  3), sizeof(finfo.last)-1);    
      strncpy(finfo.email2, PQgetvalue(res, i,  4), sizeof(finfo.email2)-1);
      strncpy(finfo.email1, PQgetvalue(res, i,  6), sizeof(finfo.email1)-1);

      ITrans.translateToClient(finfo.nick);
      ITrans.translateToClient(finfo.first);
      ITrans.translateToClient(finfo.last);
      ITrans.translateToClient(finfo.email2);
      ITrans.translateToClient(finfo.email1);
	 
      finfo.uin  = atoul(PQgetvalue(res, i,  0));
      finfo.auth = atol(PQgetvalue(res, i,  5));
      finfo.e1publ = atol(PQgetvalue(res, i,  8));
      finfo.webaware = atol(PQgetvalue(res, i,  7));

      if ((i == (fsend - 1)) | (i == (lp_v7_maxsearch()-1))) 
      {
         if (fsend > lp_v7_maxsearch())
	 {
	    /* limit: last user found, but there is more users */
	    mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 0, True, 1);
	 }
	 else
	 {
	    /* last user found, there is no more users */
	    mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 0, True, 0);
	 }
      }
      else
      {
          /* next user found */
	  mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 1, False, 0);
      }
       
      if (i == (lp_v7_maxsearch()-1)) break;
      if (((i % 100) == 0) & (i != 0)) results_delay(100);

   }
     
   /* Ok, we finished - free memory */
   PQclear(res);
}


/**************************************************************************/
/* Search users by email (tlv based)	 			  	  */
/**************************************************************************/
void mr_search_by_email2(Packet &pack, struct online_user *user, 
                         struct snac_header &snac, unsigned short req_seq, 
		         struct full_user_info &finfo, class tlv_c &tlv)
{
   class tlv_chain_c tlv_chain;
   class tlv_c *stlv;
   cstring dbcomm_str;
   int fsend;
   fstring email_str;
   PGresult *res;

   tlv_chain.read(tlv);
   tlv_chain.intel_order();
   
   if ((stlv = tlv_chain.get(0x015E)) == NULL)
   {
      if ((stlv = tlv_chain.get(0x0136)) == NULL)
      {
         DEBUG(10, ("ERROR: Mailformed srch_by_mail/uin2 packet from %s (tlv 0x136)\n",
                    inet_ntoa(pack.from_ip)));
		 
         send_snac_error(snac.family, ERR_SNAC_DATA_INVALID, snac.id, pack);
         return;
      }
   }

   
   if (stlv->type == 0x015E)
   {
      v7_extract_string(email_str, *stlv, 127, "search email", *user, pack);
   
      DEBUG(10, ("V7 search by email: %s\n", email_str));
   
      send_event2ap(papack, ACT_SEARCH, user->uin, user->status,
                    ipToIcq(user->ip), 54, longToTime(time(NULL)), email_str);   
   
      convert_to_postgres(email_str, sizeof(email_str));   

      /* make sql query string */
      snprintf(dbcomm_str, 255, "SELECT uin,nick,frst,last,email2,auth,email1,webaware,e1publ FROM Users_Info_Ext WHERE (email2 like '%%%s%%') OR (email1 like '%%%s%%') LIMIT %d", 
   	       email_str, email_str, lp_v7_maxsearch()+1);
     
      res = PQexec(users_dbconn, dbcomm_str);
      if (PQresultStatus(res) != PGRES_TUPLES_OK)
      {
         handle_database_error(res, "[V7 SEARCH BY EMAIL]");
         mr_send_wp_found(pack, user, snac, req_seq, finfo, False, 0, True, 0);
         return;
      }

      fsend = PQntuples(res);
    
      DEBUGADD(0, ("Search found %d users\n", fsend));
    
      if (fsend == 0) 
      {
         mr_send_wp_found(pack, user, snac, req_seq, finfo, False, 0, True, 0);
      }

      for(int i=0; i<fsend; i++)
      {      
         strncpy(finfo.nick,   PQgetvalue(res, i,  1), sizeof(finfo.nick)-1);
         strncpy(finfo.first,  PQgetvalue(res, i,  2), sizeof(finfo.first)-1);
         strncpy(finfo.last,   PQgetvalue(res, i,  3), sizeof(finfo.last)-1);    
         strncpy(finfo.email2, PQgetvalue(res, i,  4), sizeof(finfo.email2)-1);
         strncpy(finfo.email1, PQgetvalue(res, i,  6), sizeof(finfo.email1)-1);

         ITrans.translateToClient(finfo.nick);
         ITrans.translateToClient(finfo.first);
         ITrans.translateToClient(finfo.last);
         ITrans.translateToClient(finfo.email2);
         ITrans.translateToClient(finfo.email1);
 	 
         finfo.uin  = atoul(PQgetvalue(res, i,  0));
         finfo.auth = atol(PQgetvalue(res, i,  5));
         finfo.e1publ = atol(PQgetvalue(res, i,  8));
         finfo.webaware = atol(PQgetvalue(res, i,  7));

         if ((i == (fsend - 1)) | (i == (lp_v7_maxsearch()-1))) 
         {
            if (fsend > lp_v7_maxsearch())
   	    {
	       /* limit: last user found, but there is more users */
	       mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 0, True, 1);
	    }
	    else
	    {
	       /* last user found, there is no more users */
	       mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 0, True, 0);
	    }
         }
         else
         {
             /* next user found */
	     mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 1, False, 0);
         }
       
         if (i == (lp_v7_maxsearch()-1)) break;
	 if (((i % 100) == 0) & (i != 0)) results_delay(100);
      }
     
      /* Ok, we finished - free memory */
      PQclear(res);
   }
   
   if (stlv->type == 0x0136)
   {
      *stlv >> uin;
   
      if (db_users_lookup(uin, finfo) >= 0) 
      {
         mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 0, True, 0);
      }
      else
      {
         mr_send_wp_found(pack, user, snac, req_seq, finfo, False, 0, True, 0);
      }
   }
}


/**************************************************************************/
/* Search users by uin (tlv based)	 			  	  */
/**************************************************************************/
void mr_search_by_uin2(Packet &pack, struct online_user *user, 
                       struct snac_header &snac, unsigned short req_seq, 
		       struct full_user_info &finfo, class tlv_c &tlv)
{
   class tlv_chain_c tlv_chain;
   class tlv_c *stlv;
   unsigned long uin;

   tlv_chain.read(tlv);
   tlv_chain.intel_order();
   
   if ((stlv = tlv_chain.get(0x0136)) == NULL)
   {
      DEBUG(10, ("ERROR: Mailformed srch_by_uin2 packet from %s (tlv 0x136)\n",
                 inet_ntoa(pack.from_ip)));
		 
      send_snac_error(snac.family, ERR_SNAC_DATA_INVALID, snac.id, pack);
      return;
   }
   
   *stlv >> uin;
   
   if (db_users_lookup(uin, finfo) >= 0) 
   {
      mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 0, True, 0);
   }
   else
   {
      mr_send_wp_found(pack, user, snac, req_seq, finfo, False, 0, True, 0);
   }
}


/**************************************************************************/
/* This func process client xml request 			  	  */
/**************************************************************************/
void process_mr_xml_request(Packet &pack, struct online_user *user, 
                            struct snac_header &snac, unsigned short req_seq, 
		            class tlv_c &tlv)
{
   fstring xml_request;
   fstring xml_reply;
   struct variable_record *variable = NULL;
   
   v7_extract_string(xml_request, tlv, 127, "xml request", *user, pack);

   string_sub(xml_request, "<key>", "", sizeof(fstring)-1);
   string_sub(xml_request, "</key>", "", sizeof(fstring)-1);   
   variable = aim_get_variable(xml_request);
   
   
   if (variable == NULL) 
   {   
      mr_send_xml_info(pack, user, snac, req_seq, False, "", 0);
      DEBUGADD(150, ("Sysvar req: %s=[not found]\n", xml_request));
   }
   else
   {
      snprintf(xml_reply, sizeof(fstring)-1, "<value>%s</value>", variable->str_val);
      mr_send_xml_info(pack, user, snac, req_seq, True, xml_reply, 0);
      DEBUGADD(150, ("Sysvar req: %s='%s'\n", xml_request, variable->str_val));
   }
}


/**************************************************************************/
/* This func process client send_sms request 			  	  */
/**************************************************************************/
void process_mr_sms_request(Packet &pack, struct online_user *user, 
                            struct snac_header &snac, unsigned short req_seq, 
		            class tlv_c &tlv)
{
   DEBUG(10, ("Client sent sms message\n"));
   
   /* BUGBUG: STUB */
   /* TODO: pipe this request to external program */
   mr_send_blm_err(pack, user, snac, req_seq, 0x46, 
                   "Sorry, but IServerd SMS messages not implemented yet...");
}


/**************************************************************************/
/* This func parse hpage category info packet			  	  */
/**************************************************************************/
void mr_set_user_hpcat_info(Packet &pack, struct online_user *user, 
                            struct snac_header &snac, unsigned short req_seq, 
		            class tlv_c &tlv)
{
   fstring description;
   char enabled;
   short hpindex;

   tlv >> enabled
       >> hpindex;

   v7_extract_string(description, tlv, 127, "hpage_txt", *user, pack);
   
   /* Now we can save hpcat info */   
   if (db_users_sethpagecat_info(user->uin, enabled, hpindex, description) != -1) 
   {
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETHPCAT_ACK, True, 0);
   } 
   else
   {            
      LOG_USR(0, ("Error saving hpcat user information for %lu\n", user->uin));
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETHPCAT_ACK, False, 0);	
   }
}


/**************************************************************************/
/* This func parse affilations email set info packet		  	  */
/**************************************************************************/
void mr_set_user_affilations_info(Packet &pack, struct online_user *user, 
                                  struct snac_header &snac, unsigned short req_seq, 
		                  class tlv_c &tlv)
{
   struct ext_user_info fuser;
   
   bzero((void *)&fuser, sizeof(fuser));

   tlv >> (char &)fuser.past_num;
  
   if ((fuser.past_num > 4) | (fuser.past_num < 0)) fuser.past_num = 0;

   tlv >> fuser.past_ind1;
   v7_extract_string(fuser.past_key1, tlv, 63, "past key1", *user, pack);
   tlv >> fuser.past_ind2;
   v7_extract_string(fuser.past_key2, tlv, 63, "past key2", *user, pack);
   tlv >> fuser.past_ind3;
   v7_extract_string(fuser.past_key3, tlv, 63, "past key3", *user, pack);

   tlv >> (char &)fuser.aff_num;
  
   if ((fuser.aff_num > 4) | (fuser.aff_num < 0)) fuser.aff_num = 0;

   tlv >> fuser.aff_ind1;
   v7_extract_string(fuser.aff_key1, tlv, 63, "aff key1", *user, pack);
   tlv >> fuser.aff_ind2;
   v7_extract_string(fuser.aff_key2, tlv, 63, "aff key2", *user, pack);
   tlv >> fuser.aff_ind3;
   v7_extract_string(fuser.aff_key3, tlv, 63, "aff key3", *user, pack);

   /* Now we can save interests info */   
   if (db_users_setaffilations_info(user->uin, fuser) != -1) 
   {
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETAFF_ACK, True, 0);
   } 
   else
   {            
      LOG_USR(0, ("Error saving affilations user information for %lu\n", user->uin));
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETAFF_ACK, False, 0);
   }
}


/**************************************************************************/
/* This func parse interests email set info packet		  	  */
/**************************************************************************/
void mr_set_user_interests_info(Packet &pack, struct online_user *user, 
                                struct snac_header &snac, unsigned short req_seq, 
		                class tlv_c &tlv)
{
   struct ext_user_info fuser;
   fuser.int_num = 0;
   
   bzero((void *)&fuser, sizeof(fuser));
   
   tlv >> (char &)fuser.int_num;
  
   if ((fuser.int_num > 4) | (fuser.int_num < 0)) fuser.int_num = 0;

   tlv >> fuser.int_ind1;
   v7_extract_string(fuser.int_key1, tlv, 63, "interests key1", *user, pack);
   tlv >> fuser.int_ind2;
   v7_extract_string(fuser.int_key2, tlv, 63, "interests key2", *user, pack);
   tlv >> fuser.int_ind3;
   v7_extract_string(fuser.int_key3, tlv, 63, "interests key3", *user, pack);
   tlv >> fuser.int_ind4;
   v7_extract_string(fuser.int_key4, tlv, 63, "interests key4", *user, pack);

   /* Now we can save interests info */   
   if (db_users_setinterests_info(user->uin, fuser) != -1) 
   {
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETINT_ACK, True, 0);
   } 
   else
   {            
      LOG_USR(0, ("Error saving interests user information for %lu\n", user->uin));
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETINT_ACK, False, 0);
   }
}


/**************************************************************************/
/* This func parse email set info packet			  	  */
/**************************************************************************/
void mr_set_user_email_info(Packet &pack, struct online_user *user, 
                            struct snac_header &snac, unsigned short req_seq, 
		            class tlv_c &tlv)
{
   BOOL parse_ok = True;
   struct full_user_info finfo;
   char email_num, i;
   tlv >> email_num;

   if (db_users_lookup(user->uin, finfo) < 0)
   {
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETEMAIL_ACK, False, 0);
      return;
   }
   
   strncpy(finfo.email2, "", 63);
   strncpy(finfo.email3, "", 63);
   
   /* our database can handle only 3 email addr - 1 primary + 2 secondary */
   /* so we'll reject set-email packets where email_num > 2               */
   if (email_num > 2)
   {
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETEMAIL_ACK, False, 0);
      return;
   }
   
   if (email_num > 0) 
   {
      tlv >> i;
      if (!v7_extract_string(finfo.email2, tlv, 63, "email2", *user, pack)) parse_ok = False;
   }
       
   if (email_num > 1)
   {
      tlv >> i;
      if (!v7_extract_string(finfo.email3, tlv, 63, "email3", *user, pack)) parse_ok = False;
   }

   /* Now we can save work info */
   if ((parse_ok) &&(db_users_setbasic_info2(user->uin, finfo) != -1)) 
   {
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETEMAIL_ACK, True, 0);
   } 
   else
   {            
      LOG_USR(0, ("Error saving email user information for %lu\n", user->uin));
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETEMAIL_ACK, False, 0);
   }   
}


/**************************************************************************/
/* This func parse work set info packet				  	  */
/**************************************************************************/
void mr_set_user_work_info(Packet &pack, struct online_user *user, 
                           struct snac_header &snac, unsigned short req_seq, 
		           class tlv_c &tlv)
{
   BOOL parse_ok = True;
   struct full_user_info fuser;

   bzero((void *)&fuser, sizeof(fuser));
   
   if (!v7_extract_string(fuser.wcity,    tlv, sizeof(fuser.wcity)-1, "wcity", *user, pack)) parse_ok = False;
   if (!v7_extract_string(fuser.wstate,   tlv, sizeof(fuser.wstate)-1, "wstate", *user, pack)) parse_ok = False;
   if (!v7_extract_string(fuser.wphone,   tlv, sizeof(fuser.wphone)-1, "wphone", *user, pack)) parse_ok = False;
   if (!v7_extract_string(fuser.wfax,     tlv, sizeof(fuser.wfax)-1, "wfax", *user, pack)) parse_ok = False;
   if (!v7_extract_string(fuser.waddr,    tlv, sizeof(fuser.waddr)-1, "waddr", *user, pack)) parse_ok = False;
   if (!v7_extract_string(fuser.wzip2,    tlv, sizeof(fuser.wzip2)-1, "wzip2", *user, pack)) parse_ok = False;   
   tlv >> fuser.wcountry;
   if (!v7_extract_string(fuser.wcompany, tlv, sizeof(fuser.wcompany)-1, "wcompany", *user, pack)) parse_ok = False;
   if (!v7_extract_string(fuser.wdepart2, tlv, sizeof(fuser.wdepart2)-1, "wdepart2", *user, pack)) parse_ok = False;
   if (!v7_extract_string(fuser.wtitle,   tlv, sizeof(fuser.wtitle)-1, "wtitle", *user, pack)) parse_ok = False;
   tlv >> fuser.wocup;
   if (!v7_extract_string(fuser.wpage,    tlv, sizeof(fuser.wpage)-1, "wpage", *user, pack)) parse_ok = False;

   /* Now we can save work info */   
   if ((parse_ok) && (db_users_setwork_info2(user->uin, fuser) != -1))
   {
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETWORK_ACK, True, 0);
   } 
   else
   {            
      LOG_USR(0, ("Error saving work user information for %lu\n", user->uin));
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETWORK_ACK, False, 0);
   }   
}


/**************************************************************************/
/* This func parse perms set info packet			  	  */
/**************************************************************************/
void mr_set_user_perms_info(Packet &pack, struct online_user *user, 
                            struct snac_header &snac, unsigned short req_seq, 
		            class tlv_c &tlv)
{
   char dc_perms, webaware, auth;

   /* dc_perms: 0 - dc with any user, 1- dc with listed in contact, */
   /*           2 - dc with any upon authorization                  */
   
   tlv >> auth
       >> webaware
       >> dc_perms;
	
   if ((dc_perms < 0) || (dc_perms > 2)) dc_perms = 1;
   
   /* Now we can save security info */   
   if (db_users_setsecure_info(user->uin, auth, dc_perms, webaware) != -1) 
   {
      DEBUG(100, ("Saving user permissions: uin=%lu, auth=%d, dc_perms=%d, webaware=%d\n", 
		   user->uin, auth, dc_perms, webaware));
			      
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_PERMS_ACK, True, 0);
   } 
   else
   {
      LOG_USR(0, ("Error saving secure user information for %lu\n", user->uin));
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_PERMS_ACK, False, 0);
   }
}


/**************************************************************************/
/* Set icqphone info stub					  	  */
/**************************************************************************/
void mr_set_user_icqphone_info(Packet &pack, struct online_user *user, 
                               struct snac_header &snac, unsigned short req_seq, 
		               class tlv_c &tlv)
{
   /* TLV contain bunch of unknown parameters   */
   /* we just ignore this stuff because I don't */
   /* want to code icqphone server :)))         */
   mr_send_set_ack(pack, user, snac, req_seq, META_INFO_ICQPHONE_ACK, True, 0);
}


/**************************************************************************/
/* This func parse notes set info packet			  	  */
/**************************************************************************/
void mr_set_user_notes_info(Packet &pack, struct online_user *user, 
                            struct snac_header &snac, unsigned short req_seq, 
		            class tlv_c &tlv)
{
   BOOL parse_ok = True;
   struct notes_user_info notes;

   if (!v7_extract_string(notes.notes, tlv, 1024, "set notes", *user, pack)) 
      parse_ok = False;

   /* Now we can save notes info */             
   if ((parse_ok) && (db_users_setnotes(user->uin, notes) != -1))
   {
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETNOTES_ACK, True, 0);
   } 
   else
   {            
      LOG_USR(0, ("Error saving about info for %lu\n", user->uin));
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETNOTES_ACK, False, 0);
   }
}


/**************************************************************************/
/* This func parse more set info packet				  	  */
/**************************************************************************/
void mr_set_user_more_info(Packet &pack, struct online_user *user, 
                           struct snac_header &snac, unsigned short req_seq, 
		           class tlv_c &tlv)
{
   BOOL parse_ok = True;
   struct full_user_info fuser;
   bzero((void *)&fuser, sizeof(fuser));

   tlv >> fuser.age
       >> fuser.gender;
       
   if (!v7_extract_string(fuser.hpage,  tlv, 128, "hpage", *user, pack)) 
      parse_ok = False;

   tlv >> fuser.byear
       >> fuser.bmonth
       >> fuser.bday
       >> fuser.lang1
       >> fuser.lang2
       >> fuser.lang3;

   /* Now we can save more info */
   if ((parse_ok) &&(db_users_setV5more_info(user->uin, fuser) != -1))
   {
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETMORE_ACK, True, 0);
   } 
   else
   {            
      LOG_USR(0, ("Error saving more user information for %lu\n", user->uin));
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETMORE_ACK, False, 0);
   }
}


/**************************************************************************/
/* This func parse password set info packet			  	  */
/**************************************************************************/
void mr_set_user_pass_info(Packet &pack, struct online_user *user, 
                           struct snac_header &snac, unsigned short req_seq, 
		           class tlv_c &tlv)
{
   BOOL parse_ok = True;
   char password[33];

   if (!v7_extract_string(password, tlv, sizeof(password)-1, "password", *user, pack)) 
      parse_ok = False;

   /* Now we can save password info */             
   if ((parse_ok) && (db_users_setpassword(user->uin, password) != -1))
   {
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_PASS_ACK, True, 0);
   } 
   else
   {            
      LOG_USR(0, ("Error saving password for %lu\n", user->uin));
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_PASS_ACK, False, 0);
   }
}


/**************************************************************************/
/* This func parse home set info packet				  	  */
/**************************************************************************/
void mr_set_user_home_info(Packet &pack, struct online_user *user, 
                           struct snac_header &snac, unsigned short req_seq, 
		           class tlv_c &tlv)
{
   BOOL parse_ok = True;
   struct full_user_info fuser;

   bzero((void *)&fuser, sizeof(fuser));
   
   send_event2ap(papack, ACT_SAVEBASIC, user->uin, user->status, ipToIcq(user->ip), 
                 0, longToTime(time(NULL)), "");                     

   /* first block - personal information clauses */
   if (!v7_extract_string(fuser.nick,   tlv, sizeof(fuser.nick)-1, "nick name", *user, pack)) parse_ok = False;
   if (!v7_extract_string(fuser.first,  tlv, sizeof(fuser.first)-1, "first name", *user, pack)) parse_ok = False;
   if (!v7_extract_string(fuser.last,   tlv, sizeof(fuser.last)-1, "last name", *user, pack)) parse_ok = False;
   if (!v7_extract_string(fuser.email1, tlv, sizeof(fuser.email1)-1, "email1", *user, pack)) parse_ok = False;
   if (!v7_extract_string(fuser.hcity,  tlv, sizeof(fuser.hcity)-1, "hcity", *user, pack)) parse_ok = False;
   if (!v7_extract_string(fuser.hstate, tlv, sizeof(fuser.hstate)-1, "hstate", *user, pack)) parse_ok = False;
   if (!v7_extract_string(fuser.hphone, tlv, sizeof(fuser.hphone)-1, "hphone", *user, pack)) parse_ok = False;
   if (!v7_extract_string(fuser.hfax,   tlv, sizeof(fuser.hfax)-1, "hfax", *user, pack)) parse_ok = False;
   if (!v7_extract_string(fuser.haddr,  tlv, sizeof(fuser.haddr)-1, "haddr", *user, pack)) parse_ok = False;
   if (!v7_extract_string(fuser.hcell,  tlv, sizeof(fuser.hcell)-1, "hcell", *user, pack)) parse_ok = False;
   if (!v7_extract_string(fuser.hzip2,  tlv, sizeof(fuser.hzip2)-1, "hzip2", *user, pack)) parse_ok = False;

   tlv >> fuser.hcountry;
   tlv >> fuser.gmt_offset;
   tlv >> fuser.e1publ;
   
   /* Now we can save basic info */   
   if ((parse_ok) && (db_users_setbasic_info3(user->uin, fuser) != -1))
   {
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETHOME_ACK, True, 0);
   } 
   else
   {            
      LOG_USR(0, ("Error saving basic user information for %lu\n", user->uin));
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SETHOME_ACK, False, 0);
   }   
}


/**************************************************************************/
/* This func send home_info packet				  	  */
/**************************************************************************/
void mr_send_home_info(Packet &pack, struct online_user *user, 
                       struct snac_header &snac, unsigned short req_seq, 
		       struct full_user_info &finfo, unsigned short success,
		       unsigned short flags)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_ICQxMESSxEXT
          << (unsigned short)SN_IME_MULTIxMESSxRESP
	  << (unsigned short)flags
	  << (unsigned  long)snac.id;

   /* I hate ICQ... I think AOL staff love to fuck standing in a hammock */
   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated(); 
   
   reply_pack << (unsigned  long)user->uin
	      << (unsigned short)META_RESP_INFORMATION
	      << (unsigned short)req_seq
	      << (unsigned short)META_INFO_USER_INFO2;

   if (success)
   {

      reply_pack << (char)META_SUCCESS
	         << (unsigned short)(strlen(finfo.nick)+1)
	         << finfo.nick
	         << (unsigned short)(strlen(finfo.first)+1)
	         << finfo.first
	         << (unsigned short)(strlen(finfo.last)+1)
	         << finfo.last;
      
      if ((finfo.e1publ != 1) || (finfo.uin == user->uin))
      {
         reply_pack << (unsigned short)(strlen(finfo.email1)+1)
	            << finfo.email1;
      }
      else
      {
         reply_pack << (unsigned short)(strlen("")+1)
	            << "";
      }
		  
      reply_pack << (unsigned short)(strlen(finfo.hcity)+1)
	         << finfo.hcity
	         << (unsigned short)(strlen(finfo.hstate)+1)
	         << finfo.hstate
	         << (unsigned short)(strlen(finfo.hphone)+1)
	         << finfo.hphone
	         << (unsigned short)(strlen(finfo.hfax)+1)
	         << finfo.hfax
	         << (unsigned short)(strlen(finfo.haddr)+1)
	         << finfo.haddr
	         << (unsigned short)(strlen(finfo.hcell)+1)
	         << finfo.hcell
	         << (unsigned short)(strlen(finfo.hzip2)+1)
		 << finfo.hzip2
	         << (unsigned short)finfo.hcountry
	         << (char)finfo.gmt_offset
	         << (char)finfo.auth
	         << (char)finfo.webaware
		 << (char)finfo.iphide  /* dc_permissions */
	         << (char)finfo.e1publ; /* i'm not sure   */
   }
   else
   {
      reply_pack << (char)META_FAIL;
   }
   	       
   /* now time to put data into arpack */
   arpack << (unsigned short)0x0001                /* tlv 1    */
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));  /* tlv size */
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending v7 home info meta-reply to user %lu\n", user->uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* This func send ack to set_perms packet 			  	  */
/**************************************************************************/
void mr_send_set_ack(Packet &pack, struct online_user *user, 
                     struct snac_header &snac, unsigned short req_seq, 
		     unsigned short sub_cmd, unsigned short success,
		     unsigned short flags)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_ICQxMESSxEXT
          << (unsigned short)SN_IME_MULTIxMESSxRESP
	  << (unsigned short)flags
	  << (unsigned  long)snac.id;

   /* I hate ICQ... I think AOL staff love to fuck standing in a hammock */
   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated(); 
   
   reply_pack << (unsigned  long)user->uin
	      << (unsigned short)META_RESP_INFORMATION
	      << (unsigned short)req_seq
	      << (unsigned short)sub_cmd;

   if (success == 1)
   {
      reply_pack << (char)META_SUCCESS;
   }
   else
   {
      reply_pack << (char)META_FAIL;
   }
   	       
   /* now time to put data into arpack */
   
   arpack << (unsigned short)0x0001     /* tlv 1 */
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending v7 set ack meta-reply to user %lu\n", user->uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* This func send more_info packet				  	  */
/**************************************************************************/
void mr_send_more_info(Packet &pack, struct online_user *user, 
                       struct snac_header &snac, unsigned short req_seq, 
		       struct full_user_info &finfo, unsigned short success,
		       unsigned short flags)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_ICQxMESSxEXT
          << (unsigned short)SN_IME_MULTIxMESSxRESP
	  << (unsigned short)flags
	  << (unsigned  long)snac.id;

   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated(); 
   
   /* incapsulated v5-style packet */
   reply_pack << (unsigned  long)user->uin
	      << (unsigned short)META_RESP_INFORMATION
	      << (unsigned short)req_seq
	      << (unsigned short)META_INFO_MORE;

   if (success)
   {
      reply_pack << (char)META_SUCCESS
	         << (unsigned short)finfo.age
	         << (char)finfo.gender
	         << (unsigned short)(strlen(finfo.hpage)+1)
	         << finfo.hpage
	         << (unsigned short)finfo.byear
	         << (char)finfo.bmonth
		 << (char)finfo.bday
		 << (char)finfo.lang1
		 << (char)finfo.lang2
		 << (char)finfo.lang3
		 
		 /* wtf ? */
		 << (unsigned short)0x0000
		 << (unsigned short)0x0001
		 << (char)0x00
		 << (unsigned short)0x0001
		 << (char)0x00
		 << (unsigned short)0x0000
		 << (char)finfo.martial;
   }
   else
   {
      reply_pack << (char)META_FAIL;
   }
   	       
   /* now time to put data into arpack */
   arpack << (unsigned short)0x0001              /* tlv 1    */
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));  /* tlv size */
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending v7 more info meta-reply to user %lu\n", user->uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* This func send more_email_info packet			  	  */
/**************************************************************************/
void mr_send_email_info(Packet &pack, struct online_user *user, 
                        struct snac_header &snac, unsigned short req_seq, 
		        struct full_user_info &finfo, unsigned short success, 
			unsigned short flags)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_ICQxMESSxEXT
          << (unsigned short)SN_IME_MULTIxMESSxRESP
	  << (unsigned short)flags
	  << (unsigned  long)snac.id;

   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated(); 
   
   /* incapsulated v5-style packet */
   reply_pack << (unsigned  long)user->uin
	      << (unsigned short)META_RESP_INFORMATION
	      << (unsigned short)req_seq
	      << (unsigned short)META_INFO_EMAIL_MORE;

   if (success)
   {
      char email_num = 2;

      /* well, iserverd database supports only 3 email addresses yet */
      if (strlen(finfo.email2) == 0) email_num--;
      if (strlen(finfo.email3) == 0) email_num--;
      
      reply_pack << (char)META_SUCCESS
                 << (char)email_num;

      if (strlen(finfo.email2) > 0)
      {
         reply_pack << (char)0x00 /* publish email = yes */
	            << (unsigned short)(strlen(finfo.email2)+1)
	            << finfo.email2;
      }

      if (strlen(finfo.email3) > 0)
      {
         reply_pack << (char)0x00 /* publish email = yes */
	            << (unsigned short)(strlen(finfo.email3)+1)
	            << finfo.email3;
      }
   }
   else
   {
      reply_pack << (char)META_FAIL;
   }
   	       
   /* now time to put data into arpack */
   arpack << (unsigned short)0x0001                /* tlv 1    */
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));  /* tlv size */
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending v7 extended email info meta-reply to user %lu\n", user->uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* This func send hpage_info packet				  	  */
/**************************************************************************/
void mr_send_hpage_info(Packet &pack, struct online_user *user, 
                        struct snac_header &snac, unsigned short req_seq, 
		        struct full_user_info &finfo, unsigned short success,
			unsigned short flags)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_ICQxMESSxEXT
          << (unsigned short)SN_IME_MULTIxMESSxRESP
	  << (unsigned short)flags
	  << (unsigned  long)snac.id;

   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated(); 
   
   /* incapsulated v5-style packet */
   reply_pack << (unsigned  long)user->uin
	      << (unsigned short)META_RESP_INFORMATION
	      << (unsigned short)req_seq
	      << (unsigned short)META_INFO_HPAGE_CAT;

   if (success)
   {
      /* well, AOL servers doesn't support this, only isd and generic ICQ clients */
      reply_pack << (char)META_SUCCESS
                 << (char)finfo.hpage_cf
		 << (unsigned short)finfo.hpage_cat
	         << (unsigned short)(strlen(finfo.hpage_txt)+1)
	         << finfo.hpage_txt
		 << (char)0x00;
   }
   else
   {
      reply_pack << (char)META_FAIL;
   }
   	       
   /* now time to put data into arpack */
   arpack << (unsigned short)0x0001                /* tlv 1    */
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));  /* tlv size */
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending v7 hpage category info meta-reply to user %lu\n", user->uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* This func send work_info packet				  	  */
/**************************************************************************/
void mr_send_work_info(Packet &pack, struct online_user *user, 
                        struct snac_header &snac, unsigned short req_seq, 
		        struct full_user_info &finfo, unsigned short success,
			unsigned short flags)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_ICQxMESSxEXT
          << (unsigned short)SN_IME_MULTIxMESSxRESP
	  << (unsigned short)flags
	  << (unsigned  long)snac.id;

   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated(); 
   
   /* incapsulated v5-style packet */
   reply_pack << (unsigned  long)user->uin
	      << (unsigned short)META_RESP_INFORMATION
	      << (unsigned short)req_seq
	      << (unsigned short)META_INFO_WORK;

   if (success)
   {

      reply_pack << (char)META_SUCCESS
	         << (unsigned short)(strlen(finfo.wcity)+1)
	         << finfo.wcity
	         << (unsigned short)(strlen(finfo.wstate)+1)
	         << finfo.wstate
		 << (unsigned short)(strlen(finfo.wphone)+1)
	         << finfo.wphone
	         << (unsigned short)(strlen(finfo.wfax)+1)
	         << finfo.wfax
		 << (unsigned short)(strlen(finfo.waddr)+1)
		 << finfo.waddr
		 << (unsigned short)(strlen(finfo.wzip2)+1)
		 << finfo.wzip2
		 << (unsigned short)finfo.wcountry
		 << (unsigned short)(strlen(finfo.wcompany)+1)
		 << finfo.wcompany
		 << (unsigned short)(strlen(finfo.wdepart2)+1)
		 << finfo.wdepart2
		 << (unsigned short)(strlen(finfo.wtitle)+1)
		 << finfo.wtitle
		 << (unsigned short)finfo.wocup 
		 << (unsigned short)(strlen(finfo.wpage)+1)
		 << finfo.wpage;
   }
   else
   {
      reply_pack << (char)META_FAIL;
   }
   	       
   /* now time to put data into arpack */
   
   arpack << (unsigned short)0x0001   /* tlv 1    */
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending v7 work info meta-reply to user %lu\n", user->uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* This func send about_info packet				  	  */
/**************************************************************************/
void mr_send_about_info(Packet &pack, struct online_user *user, 
                        struct snac_header &snac, unsigned short req_seq, 
			unsigned long to_uin, unsigned short flags)
{
   struct notes_user_info notes;
   unsigned short success = False;
 
   if (db_users_notes(to_uin, notes) >= 0) success = True;

   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_ICQxMESSxEXT
          << (unsigned short)SN_IME_MULTIxMESSxRESP
	  << (unsigned short)flags
	  << (unsigned  long)snac.id;

   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated(); 
   
   /* incapsulated v5-style packet */
   reply_pack << (unsigned  long)user->uin
	      << (unsigned short)META_RESP_INFORMATION
	      << (unsigned short)req_seq
	      << (unsigned short)META_INFO_ABOUT;

   if (success)
   {
      reply_pack << (char)META_SUCCESS
	         << (unsigned short)(strlen(notes.notes)+1)
	         << notes.notes;
   }
   else
   {
      reply_pack << (char)META_FAIL;
   }
   	       
   /* now time to put data into arpack */
   
   arpack << (unsigned short)0x0001     /* tlv 1    */
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending v7 about info meta-reply to user %lu\n", user->uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* This func send interests info				  	  */
/**************************************************************************/
void mr_send_interests_info(Packet &pack, struct online_user *user, 
                            struct snac_header &snac, unsigned short req_seq, 
		            struct full_user_info &finfo, unsigned short success, 
			    unsigned short flags)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_ICQxMESSxEXT
          << (unsigned short)SN_IME_MULTIxMESSxRESP
	  << (unsigned short)flags
	  << (unsigned  long)snac.id;

   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated(); 
   
   /* incapsulated v5-style packet */
   reply_pack << (unsigned  long)user->uin
	      << (unsigned short)META_RESP_INFORMATION
	      << (unsigned short)req_seq
	      << (unsigned short)META_INFO_INTERESTS;

   if (success)
   {
      if ((finfo.int_num > 4) | (finfo.int_num < 0)) finfo.int_num = 0;

      reply_pack << (char)META_SUCCESS
                 << (char)finfo.int_num;
		 
      reply_pack << (unsigned short)finfo.int_ind1
	         << (unsigned short)(strlen(finfo.int_key1)+1)
	         << finfo.int_key1;
      reply_pack << (unsigned short)finfo.int_ind2
                 << (unsigned short)(strlen(finfo.int_key2)+1)
                 << finfo.int_key2;
      reply_pack << (unsigned short)finfo.int_ind3
                 << (unsigned short)(strlen(finfo.int_key3)+1)
                 << finfo.int_key3;
      reply_pack << (unsigned short)finfo.int_ind4
                 << (unsigned short)(strlen(finfo.int_key4)+1)
                 << finfo.int_key4;
   }
   else
   {
      reply_pack << (char)META_FAIL;
   }
   	       
   /* now time to put data into arpack */
   
   arpack << (unsigned short)0x0001    /* tlv 1    */
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending v7 interests info meta-reply to user %lu\n", user->uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* This func send affilations info				  	  */
/**************************************************************************/
void mr_send_affilations_info(Packet &pack, struct online_user *user, 
                            struct snac_header &snac, unsigned short req_seq, 
		            struct full_user_info &finfo, unsigned short success, 
			    unsigned short flags)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_ICQxMESSxEXT
          << (unsigned short)SN_IME_MULTIxMESSxRESP
	  << (unsigned short)flags
	  << (unsigned  long)snac.id;

   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated(); 
   
   /* incapsulated v5-style packet */
   reply_pack << (unsigned  long)user->uin
	      << (unsigned short)META_RESP_INFORMATION
	      << (unsigned short)req_seq
	      << (unsigned short)META_INFO_AFFILATIONS;

   if (success)
   {
      reply_pack << (char)META_SUCCESS
		 << (char)0x03
	         << (unsigned short)finfo.past_ind1
	         << (unsigned short)(strlen(finfo.past_key1)+1)
	         << finfo.past_key1
                 << (unsigned short)finfo.past_ind2
	         << (unsigned short)(strlen(finfo.past_key2)+1)
	         << finfo.past_key2
	         << (unsigned short)finfo.past_ind3
	         << (unsigned short)(strlen(finfo.past_key3)+1)
                 << finfo.past_key3
	       
	         << (char)0x03
	         << (unsigned short)finfo.aff_ind1
	         << (unsigned short)(strlen(finfo.aff_key1)+1)
	         << finfo.aff_key1
                 << (unsigned short)finfo.aff_ind2
	         << (unsigned short)(strlen(finfo.aff_key2)+1)
	         << finfo.aff_key2
	         << (unsigned short)finfo.aff_ind3
	         << (unsigned short)(strlen(finfo.aff_key3)+1)
                 << finfo.aff_key3
	         << (unsigned short)0x0000
	         << (unsigned short)0x0001
	         << (char)0x00;
   }
   else
   {
      reply_pack << (char)META_FAIL;
   }
   	       
   /* now time to put data into arpack */
   
   arpack << (unsigned short)0x0001     /* tlv 1 */
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending v7 affilations info meta-reply to user %lu\n", user->uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* This func send short_info packet				  	  */
/**************************************************************************/
void mr_send_short_info(Packet &pack, struct online_user *user, 
                        struct snac_header &snac, unsigned short req_seq, 
		        struct full_user_info &finfo, unsigned short success,
		        unsigned short flags)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_ICQxMESSxEXT
          << (unsigned short)SN_IME_MULTIxMESSxRESP
	  << (unsigned short)flags
	  << (unsigned  long)snac.id;

   /* I hate ICQ... I think AOL staff love to fuck standing in a hammock */
   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated();
   
   reply_pack << (unsigned  long)user->uin
	      << (unsigned short)META_RESP_INFORMATION
	      << (unsigned short)req_seq
	      << (unsigned short)META_INFO_SHORT;

   if (success)
   {
      reply_pack << (char)META_SUCCESS
	         << (unsigned short)(strlen(finfo.nick)+1)
	         << finfo.nick
	         << (unsigned short)(strlen(finfo.first)+1)
	         << finfo.first
	         << (unsigned short)(strlen(finfo.last)+1)
	         << finfo.last;
      
      if ((finfo.e1publ != 1) || (finfo.uin == user->uin))
      {
         reply_pack << (unsigned short)(strlen(finfo.email1)+1)
	            << finfo.email1;
      }
      else
      {
         reply_pack << (unsigned short)(strlen("")+1)
	            << "";
      }
		  
      reply_pack << (char)finfo.auth
	         << (char)0x00
	         << (char)finfo.gender;
   }
   else
   {
      reply_pack << (char)META_FAIL;
   }
   	       
   /* now time to put data into arpack */
   
   arpack << (unsigned short)0x0001     /* tlv 1    */
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));  /* tlv size */
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending v7 short info meta-reply to user %lu\n", user->uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* This func send search_uin					  	  */
/**************************************************************************/
void mr_send_wp_found(Packet &pack, struct online_user *user, 
                      struct snac_header &snac, unsigned short req_seq, 
		      struct full_user_info &finfo, unsigned short success,
		      unsigned short flags, unsigned long last, 
		      unsigned long users_left)
{

   unsigned short pack_len;

   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();

   arpack << (unsigned short)SN_TYP_ICQxMESSxEXT
          << (unsigned short)SN_IME_MULTIxMESSxRESP
	  << (unsigned short)flags
	  << (unsigned  long)snac.id;

   /* I hate ICQ... I think AOL staff love to fuck standing in a hammock */
   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated(); 
   
   reply_pack << (unsigned  long)user->uin
	      << (unsigned short)META_RESP_INFORMATION
	      << (unsigned short)req_seq;
 
   if (last != 1)
   { 
      reply_pack << (unsigned short)META_WHITE_FOUND;
   }
   else
   {
      reply_pack << (unsigned short)META_WHITE_LAST_FOUND;
   }
		   
   if  (success) 
   {
      reply_pack << (char)META_SUCCESS;
   }
   else
   {
      reply_pack << (char)META_EMPTY;
   }      

   if (success)
   {
      /* I REALLY hate Mirabilis for this */
      pack_len = 8 + 4 + 3 + 3 + 2;
      if (last) pack_len += 4;
      pack_len = pack_len + strlen(finfo.nick) + 
                            strlen(finfo.first) + 
			    strlen(finfo.last);      
			     
      if ((finfo.e1publ != 1) || (finfo.uin == user->uin))
      {
         pack_len += strlen(finfo.email1);
      }
      else
      {
         pack_len += strlen(finfo.email2);
      }
      
      /* Ok now we cant put data into packet */
      reply_pack  << (unsigned short)pack_len
                  << (unsigned  long)finfo.uin
   	          << (unsigned short)(strlen(finfo.nick)+1)
	          << finfo.nick
	          << (unsigned short)(strlen(finfo.first)+1)
	          << finfo.first
	          << (unsigned short)(strlen(finfo.last)+1)
	          << finfo.last;
		  
      /* Well here we should check if user wants to publish   */
      /* his email1 address, if not - push email2 into packet */
      if ((finfo.e1publ != 1) || (finfo.uin == user->uin))
      {
         reply_pack << (unsigned short)(strlen(finfo.email1)+1)
	            << finfo.email1;
      }
      else
      {
         reply_pack << (unsigned short)(strlen(finfo.email2)+1)
	            << finfo.email2;      
      }
	
      reply_pack  << (char)finfo.auth;
      
      /* Ok, we should show status only if webaware = 0 */
      if (finfo.webaware == 0)
      {
         /* user is webaware */
         reply_pack << (char)0x02;
      
      }
      else
      {
         if (shm_user_exist(finfo.uin))
         {
	    /* user online*/
	    reply_pack << (char)0x01;
         }
	 else
	 {
	    /* user offline */
	    reply_pack << (char)0x00;
	 }
      }

      reply_pack << (char)0x00
    		 << (char)finfo.gender
		 << (char)finfo.age
		 << (char)0x00;

      if (last == 1) 
      {
         reply_pack << users_left;
      }
      
      reply_pack << (unsigned short)0x0000; /* Lite5 want this */
   }
	       
   /* now time to put data into arpack */
   
   arpack << (unsigned short)0x0001
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending v7 wp user found info meta-reply to user %lu\n", user->uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);

}


/**************************************************************************/
/* This func send end_of_messages packet			  	  */
/**************************************************************************/
void mr_send_end_of_messages(Packet &pack, struct online_user *user, 
                             struct snac_header &snac, unsigned short req_seq,
		             unsigned short flags)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_ICQxMESSxEXT
          << (unsigned short)SN_IME_MULTIxMESSxRESP
	  << (unsigned short)flags
	  << (unsigned  long)snac.id;

   /* prepare incapsulated packet */
   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated(); 
   
   reply_pack << (unsigned  long)user->uin
	      << (unsigned short)META_INFO_MESSAGES_EOF
	      << (unsigned short)req_seq
	      << (char)0x00;

   /* now time to put data into arpack */   
   arpack << (unsigned short)0x0001
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending v7 end of messages meta-reply to user %lu\n", user->uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);

}


/**************************************************************************/
/* This func send xml response					  	  */
/**************************************************************************/
void mr_send_xml_info(Packet &pack, struct online_user *user, 
                      struct snac_header &snac, unsigned short req_seq, 
		      unsigned short success, char *xml_str,
		      unsigned short flags)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_ICQxMESSxEXT
          << (unsigned short)SN_IME_MULTIxMESSxRESP
	  << (unsigned short)flags
	  << (unsigned  long)snac.id;

   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated(); 
   
   reply_pack << (unsigned  long)user->uin
	      << (unsigned short)META_RESP_INFORMATION
	      << (unsigned short)req_seq
	      << (unsigned short)META_XML_DATA;

   if (success)
   {
      reply_pack << (char)META_SUCCESS
	         << (unsigned short)(strlen(xml_str)+1)
	         << xml_str;
      
   }
   else
   {
      reply_pack << (char)META_FAIL;
   }
   	       
   /* now time to put data into arpack */   
   arpack << (unsigned short)0x0001
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(200, ("Sending v7 xml reply to user %lu\n", user->uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* White pages search (icq2002a modification) 			  	  */
/**************************************************************************/
void mr_search_white2(Packet &pack, struct online_user *user, 
                      struct snac_header &snac, unsigned short req_seq, 
		      struct full_user_info &finfo, class tlv_c &tlv)
{
   class tlv_chain_c tlv_chain;  
   class tlv_c *stlv;
   char dbcomm_str[4096];
   int fsend, keys_cnt;
   char *keys_ptr;
   fstring  clause_temp, pkeys, interests_dir, affkeys, pagekeys, token;
   ffstring nick_str, first_str, last_str, email, city, state, company;
   ffstring position;
   unsigned short min_age, max_age, country, pcode;
   unsigned short int_index, aff_index, page_index;
   unsigned long uin;
   char gender, language, work_code = 0, online_only, martial;
   PGresult *res;
   BOOL is_first_clause, not_implemented, at_least_one_key;

   send_event2ap(papack, ACT_SEARCH, user->uin, user->status,
                 ipToIcq(user->ip), 52, longToTime(time(NULL)), "");

   /* :) Mirabilis staff is crazy enought to put tlv chain in single tlv ! */
   tlv_chain.read(tlv);
   tlv_chain.intel_order();
   
   /* first query part - constant */
   not_implemented = True;
   snprintf(dbcomm_str, 4095, "SELECT uin,nick,frst,last,email2,auth,email1,e1publ,webaware,sex,age FROM Users_Info_Ext WHERE (1=1) ");
   
   /* uin stlv = 0x0136 */
   if ((stlv = tlv_chain.get(0x0136)) != NULL)
   {
      *stlv >> uin;

      snprintf(clause_temp, 255, "AND (uin=%lu) ", uin);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
      is_first_clause = False;
   }
   
   /* nick name stlv = 0x0154 */
   if ((stlv = tlv_chain.get(0x0154)) != NULL)
   {
      v7_extract_string(nick_str, *stlv, 32, "v7 wp nick", *user, pack);
      convert_to_postgres(nick_str, sizeof(nick_str));
      
      snprintf(clause_temp, 255, "AND (upper(nick) like upper('%%%s%%')) ", nick_str);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   /* first name stlv = 0x0140 */
   if ((stlv = tlv_chain.get(0x0140)) != NULL)
   {
      v7_extract_string(first_str, *stlv, 32, "v7 wp frst", *user, pack);
      convert_to_postgres(first_str, sizeof(first_str));
	 
      snprintf(clause_temp, 255, "AND (upper(frst) like upper('%%%s%%')) ", first_str);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   /* last name stlv = 0x014A */
   if ((stlv = tlv_chain.get(0x014A)) != NULL)
   {
      v7_extract_string(last_str,  *stlv, 32, "v7 wp last", *user, pack);
      convert_to_postgres(last_str, sizeof(last_str));
      
      snprintf(clause_temp, 255, "AND (upper(last) like upper('%%%s%%')) ", last_str);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   /* email2 stlv = 0x015E */
   if ((stlv = tlv_chain.get(0x015E)) != NULL)
   {
      v7_extract_string(email,  *stlv, 63, "v7 wp email", *user, pack);
      convert_to_postgres(email, sizeof(email));
      
      snprintf(clause_temp, 255, "AND (((upper(email1) like upper('%%%s%%')) AND (e1publ=0)) ", email);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      snprintf(clause_temp, 255, "OR (upper(email2) like upper('%%%s%%')) ", email);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      snprintf(clause_temp, 255, "OR (upper(email3) like upper('%%%s%%'))) ", email);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   /* Age range to search: stlv = 0x0168 */
   if ((stlv = tlv_chain.get(0x0168)) != NULL)
   {
      *stlv >> min_age >> max_age;
      
      /* well, I belive in immortals like highlander */
      if ((min_age != 0) && (max_age != 0) && (min_age < 5000) && (max_age < 5000)) 
      {
         snprintf(clause_temp, 255, "AND ((age >= %d) AND (age <= %d)) ", min_age, max_age);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }
   
   
   /* Gender. stlv = 0x017C */
   if ((stlv = tlv_chain.get(0x017C)) != NULL)
   {
      *stlv >> gender;      
   
      /* who knows ? :) */
      if ((gender < 16) && (gender > 0))
      {
         snprintf(clause_temp, 255, "AND (sex = %d) ", gender);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }
   
   /* Language code stlv = 0x0186 */
   if ((stlv = tlv_chain.get(0x0186)) != NULL)
   {
      *stlv >> language;      
   
      if ((language > 0) && (language < 127))
      {
         snprintf(clause_temp, 255, "AND (%d in (lang1, lang2, lang3)) ", 
                  language);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }
   
   /* City stlv = 0x0190 */
   if ((stlv = tlv_chain.get(0x0190)) != NULL)
   {
      v7_extract_string(city,  *stlv, 32, "v7 wp city", *user, pack);
      convert_to_postgres(city, sizeof(city));
      
      if (strlen(city) != 0) 
      {
         snprintf(clause_temp, 255, "AND (upper(hcity) like upper('%%%s%%')) ", city);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }
   
   
   /* state stlv = 0x019A */
   if ((stlv = tlv_chain.get(0x019A)) != NULL)
   {
      v7_extract_string(state, *stlv, 32, "v7 wp state", *user, pack);
      convert_to_postgres(state, sizeof(state));
      
      if (strlen(state) != 0) 
      {
         snprintf(clause_temp, 255, "AND ((upper(hstate) like upper('%%%s%%')) OR (upper(wstate) like upper('%%%s%%'))) ", state, state);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }
   
   /* country code stlv = 0x01A4 */
   if ((stlv = tlv_chain.get(0x01A4)) != NULL)
   {
      *stlv >> country;
   
      if ((country > 0) && (country < 20000)) 
      {
         snprintf(clause_temp, 255, "AND ((hcountry=%d) OR (wcountry=%d)) ", country, country);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }
   
   /* wcompany stlv =  0x01AE */
   if ((stlv = tlv_chain.get(0x01AE)) != NULL)
   {
      v7_extract_string(company, *stlv, 32, "v7 wp company", *user, pack);
      convert_to_postgres(company, sizeof(company));
   
      if (strlen(company) != 0) 
      {
         snprintf(clause_temp, 255, "AND (upper(wcompany) like upper('%%%s%%')) ", company);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }

   /* wposition stlv =  0x01C2 */
   if ((stlv = tlv_chain.get(0x01C2)) != NULL)
   {
      v7_extract_string(position, *stlv, 32, "v7 wp position", *user, pack);
      convert_to_postgres(position, sizeof(position));
      
      if (strlen(position) != 0) 
      {
         snprintf(clause_temp, 255, "AND (upper(wtitle) like upper('%%%s%%')) ", position);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }
   
   /* ocupation code stlv = 0x01CC */
   if ((stlv = tlv_chain.get(0x01CC)) != NULL)
   {
      if ((work_code > 0) && (work_code < 127))
      {
         snprintf(clause_temp, 255, "AND (wocup = %d) ", work_code);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }
   
   /* past info stlv = 0x01FE */
   if ((stlv = tlv_chain.get(0x01FE)) != NULL)
   {
      *stlv >> pcode;
      v7_extract_string(pkeys, *stlv, 63, "v7 wp past", *user, pack);
      convert_to_postgres(pkeys, sizeof(pkeys));
   
      if ((pcode > 0) && (pcode < 60000))
      {
         snprintf(clause_temp, 255, "AND (((past_ind1 = %d) AND (", pcode);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)pkeys;
         at_least_one_key = False;
         keys_cnt = 0;
         while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
   	    {  
	        snprintf(clause_temp, 255, 
	        "(upper(past_key1) like upper('%%%s%%')) OR ", token);
	        safe_strcat(dbcomm_str, clause_temp, 4095);
	        at_least_one_key = True;
	        keys_cnt++;
	        if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, "OR ", 4095);
         snprintf(clause_temp, 255, "((past_ind2 = %d) AND (", pcode);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)pkeys;
         at_least_one_key = False;
         keys_cnt = 0;

         while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
   	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(past_key2) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, "OR ", 4095);
         snprintf(clause_temp, 255, "((past_ind3 = %d) AND (", pcode);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)pkeys;
         at_least_one_key = False;
         keys_cnt = 0;
      
         while (next_token(&keys_ptr,token,",",sizeof(token))) 
	 {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(past_key3) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False ))", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True ))", 4095);
         }

         safe_strcat(dbcomm_str, ")", 4095);
         not_implemented = False;
      }
   }
   
   /* affilations info stlv = 0x01D6 */
   if ((stlv = tlv_chain.get(0x01D6)) != NULL)
   {
      *stlv >> aff_index;
      v7_extract_string(affkeys, *stlv, 127, "v7 wp affilations", *user, pack);
      convert_to_postgres(affkeys, sizeof(affkeys));
   
      if ((aff_index > 0) && (aff_index < 60000))
      {
         snprintf(clause_temp, 255, "AND (((aff_ind1 = %d) AND (", aff_index);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)affkeys;
         at_least_one_key = False;
         keys_cnt = 0;
         
	 while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(aff_key1) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, "OR ", 4095);
         snprintf(clause_temp, 255, "((aff_ind2 = %d) AND (", aff_index);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)affkeys;
         at_least_one_key = False;
         keys_cnt = 0;
      
         while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(aff_key2) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
  	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, "OR ", 4095);
         snprintf(clause_temp, 255, "((aff_ind3 = %d) AND (", aff_index);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)affkeys;
         at_least_one_key = False;
         keys_cnt = 0;
      
         while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(aff_key3) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
  	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, ")", 4095);
         not_implemented = False;
      }
   }

   /* interests info stlv = 0x01EA */
   if ((stlv = tlv_chain.get(0x01EA)) != NULL)
   {
      *stlv >> int_index;
      v7_extract_string(interests_dir, *stlv, 127, "v7 wp intdir", *user, pack);
      convert_to_postgres(interests_dir, sizeof(interests_dir));
   
      if ((int_index > 0) && (int_index < 60000))
      {
         snprintf(clause_temp, 255, "AND (((int_ind1 = %d) AND (", int_index);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)interests_dir;
         at_least_one_key = False;
         keys_cnt = 0;
      
         while (next_token(&keys_ptr,token,",",sizeof(token))) 
	 {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(int_key1) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, "OR ", 4095);
         snprintf(clause_temp, 255, "((int_ind2 = %d) AND (", int_index);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)interests_dir;
         at_least_one_key = False;
         keys_cnt = 0;
         
	 while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(int_key2) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, "OR ", 4095);
         snprintf(clause_temp, 255, "((int_ind3 = %d) AND (", int_index);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)interests_dir;
         at_least_one_key = False;
         keys_cnt = 0;
      
         while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(int_key3) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, "OR ", 4095);
         snprintf(clause_temp, 255, "((int_ind4 = %d) AND (", int_index);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)interests_dir;
         at_least_one_key = False;
         keys_cnt = 0;
      
         while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(int_key4) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         } 
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }


         safe_strcat(dbcomm_str, ")", 4095);
         not_implemented = False;
      }
   }
 
   /* home page category info stlv = 0x0212 */
   if ((stlv = tlv_chain.get(0x0212)) != NULL)
   {
      *stlv >> page_index;
      v7_extract_string(pagekeys, *stlv, 127, "v7 wp page", *user, pack);
      convert_to_postgres(pagekeys, sizeof(pagekeys));
   
      if ((page_index > 0) && (page_index < 60000))
      {
         snprintf(clause_temp, 255, "AND (((hpage_cat=%d) AND (", page_index);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)pagekeys;
         at_least_one_key = False;
         keys_cnt = 0;
      
         while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(hpage_txt) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, ")", 4095);
         not_implemented = False;
      }
   }
   
   /* search only online information stlv = 0x0230 */
   if ((stlv = tlv_chain.get(0x0230)) != NULL)
   {
      *stlv >> online_only;

      if (online_only) 
      {      
         safe_strcat(dbcomm_str, "AND (uin IN (SELECT uin FROM online_users)) ", 4095);
      }
   }

   /* search keyword stlv = 0x0226 */
   if ((stlv = tlv_chain.get(0x0226)) != NULL)
   {
      if (not_implemented)
      {
         strncpy(finfo.nick,   "this ", 31);  
         strncpy(finfo.first,  "search ", 31);
         strncpy(finfo.email2, "yet", 31);
         strncpy(finfo.email1, "yet", 31);
         strncpy(finfo.last,   "not implemented", 31);
	
         finfo.uin = 0; 
         finfo.auth = 1;
         finfo.e1publ = 0;
         finfo.webaware = 0;

         mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 0, True, 0);
         return;
      }
   }
  
   /* Martial. stlv = 0x033E */
   if ((stlv = tlv_chain.get(0x033E)) != NULL)
   {
      *stlv >> martial;      
   
      if ((martial < 41) && (martial > 0))
      {
         snprintf(clause_temp, 255, "AND (martial = %d) ", martial);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }

   send_event2ap(papack, ACT_SEARCH, user->uin, user->status,                
                 ipToIcq(user->ip), 55, longToTime(time(NULL)), "v7 wp search");

   /* now query is ready - we can run it */
   res = PQexec(users_dbconn, dbcomm_str);
   DEBUG(10, ("query: %s\n", dbcomm_str));
   
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[V7 WP SEARCH]");
      DEBUG(10, ("req: %s\n", dbcomm_str));
      mr_send_wp_found(pack, user, snac, req_seq, finfo, False, 0, True, 0);
      return;
   }

   fsend = PQntuples(res);
    
   DEBUG(10, ("Search found %d users\n", fsend));
    
   if (fsend == 0) 
   {
      mr_send_wp_found(pack, user, snac, req_seq, finfo, False, 0, True, 0);
   }

   for(int i=0; i<fsend; i++)
   {      
      strncpy(finfo.nick,   PQgetvalue(res, i,  1), sizeof(finfo.nick)-1);
      strncpy(finfo.first,  PQgetvalue(res, i,  2), sizeof(finfo.first)-1);
      strncpy(finfo.last,   PQgetvalue(res, i,  3), sizeof(finfo.last)-1);    
      strncpy(finfo.email2, PQgetvalue(res, i,  4), sizeof(finfo.email2)-1);
      strncpy(finfo.email1, PQgetvalue(res, i,  6), sizeof(finfo.email1)-1);

      ITrans.translateToClient(finfo.nick);
      ITrans.translateToClient(finfo.first);
      ITrans.translateToClient(finfo.last);
      ITrans.translateToClient(finfo.email2);
      ITrans.translateToClient(finfo.email1);
	 
      finfo.uin  = atoul(PQgetvalue(res, i, 0));
      finfo.auth  = atol(PQgetvalue(res, i,  5));
      finfo.e1publ  = atol(PQgetvalue(res, i, 7));
      finfo.webaware = atol(PQgetvalue(res, i, 8));
      finfo.gender = atol(PQgetvalue(res, i, 9));
      finfo.age = atol(PQgetvalue(res, i, 10));
       
      if ((i == (fsend - 1)) | (i == (lp_v7_maxsearch()-1))) 
      {
         if (fsend > lp_v7_maxsearch())
	 {
	    /* limit: last user found, but there is more users */
	    mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 0, True, 1);
	 }
	 else
	 {
	    /* last user found, there is no more users */
	    mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 1, True, 0);
	 }
      }
      else
      {
         /* next user found */
	 mr_send_wp_found(pack, user, snac, req_seq, finfo, True, 1, False, 0);
      }
       
      if (i == (lp_v7_maxsearch()-1)) break;
      if (((i % 100) == 0) & (i != 0)) results_delay(100);

    }
      
    PQclear(res);
}


/**************************************************************************/
/* Send family error to client					  	  */
/**************************************************************************/
void mr_send_blm_err(Packet &pack, struct online_user *user, 
                     struct snac_header &snac, unsigned short req_seq, 
		     char errcode, char *errdesc)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_ICQxMESSxEXT
          << (unsigned short)SN_IME_MULTIxMESSxRESP
	  << (unsigned short)0x0000
	  << (unsigned  long)snac.id;

   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated(); 
   
   reply_pack << (unsigned  long)user->uin
	      << (unsigned short)META_RESP_INFORMATION
	      << (unsigned short)req_seq
	      << (unsigned short)META_PROCESS_ERROR
	      << (char)errcode
	      << errdesc;
   	       
   /* now time to put data into arpack */
   arpack << (unsigned short)0x0001
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending meta processing error to user %lu\n", user->uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* This func send offline message to user			  	  */
/**************************************************************************/
void mr_send_offline_message(Packet &pack, struct online_user *user, 
                             struct snac_header &snac, unsigned short req_seq,
		             unsigned short flags, struct msg_header &msg_hdr,
			     char *message)
{
   int salloc = 1;
   char *nmessage = NULL;
   unsigned short i,j,s=0,cnt;
   time_t rmess_time;
   struct tm *sp_time;
   unsigned short mtype = 0;

   /* some messages should be sent thru SSI */
   if ((msg_hdr.mtype == MSG_TYPE_AUTH_GRANTED) &&
       (user->enable_ssi))
   {
      ssi_send_auth_granted(msg_hdr.fromuin, user);
      return;
   }

   if ((msg_hdr.mtype == MSG_TYPE_ADDED) &&
       (user->enable_ssi))
   {
      ssi_send_you_added(msg_hdr.fromuin, user);
      return;
   }

   if ((msg_hdr.mtype == MSG_TYPE_AUTH_REQ) &&
       (user->enable_ssi))
   {
      /* I should strip "nick\xfefirst\xfe\last\xfeemail\xfe\1\xfe" from  */
      /* message because ssi doesn't support this :( I know this is a bad */
      /* solution, so i'll fix this later */
      j = strlen(message);
      for (i=0, cnt=0; i<j; i++)
      {
         s = message[i];
         s = s & 0x00FF;
         if (s == 0xFE) cnt++;
         if (cnt == 5) 
         {
            i++; break;
         }
      }
      ssi_send_auth_req(msg_hdr.fromuin, user, message+i);
      return;
   }

   if ((msg_hdr.mtype == MSG_TYPE_AUTH_DENIED) &&
       (user->enable_ssi))
   {
      ssi_send_auth_denied(msg_hdr.fromuin, user, message);
      return;
   }
      
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_ICQxMESSxEXT
          << (unsigned short)SN_IME_MULTIxMESSxRESP
	  << (unsigned short)flags
	  << (unsigned  long)snac.id;

   /* prepare incapsulated packet */
   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated(); 
   
   mtype = convert_message_type(msg_hdr.mtype);
   nmessage = convert_message_text(msg_hdr.fromuin, msg_hdr.mtype, message);
   if (nmessage == NULL) { nmessage = message; salloc = 0; }

   rmess_time = msg_hdr.mtime - serverzone;
   sp_time = gmtime(&rmess_time);
   
   reply_pack << (unsigned  long)user->uin
	      << (unsigned short)OFFLINE_MSG_RESPONSE
	      << (unsigned short)req_seq
	      << (unsigned  long)msg_hdr.fromuin
	      << (unsigned short)((sp_time->tm_year)+1900)
              << (char)((sp_time->tm_mon)+1)
              << (char)(sp_time->tm_mday)
              << (char)sp_time->tm_hour
              << (char)sp_time->tm_min
	      << (unsigned short)mtype
	      << (unsigned short)(strlen(nmessage)+1)
	      << nmessage;

   /* now time to put data into arpack */
   arpack << (unsigned short)0x0001
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending v7 offline message to user %lu\n", user->uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
   
   if (salloc == 1) free(nmessage);
}

/**************************************************************************/
/* New UTF8 info/search requst		 			  	  */
/**************************************************************************/
void mr_search_info_req(Packet &pack, struct online_user *user, 
                      struct snac_header &snac, unsigned short req_seq, 
		      struct full_user_info &finfo, class tlv_c &tlv)
{
   DEBUG(400, ("Have new utf8 request from user %lu\n", user->uin));
   class tlv_chain_c tlv_chain;  
   class tlv_c *stlv;
   char dbcomm_str[4096];
   int fsend;
   fstring  clause_temp;
   ffstring nick_str, first_str, last_str, email, city;
   unsigned short min_age, max_age;
   char gender;
   PGresult *res;
   BOOL is_first_clause, not_implemented;
   ffstring uin;
   unsigned short online_only, language, martial, req_type;
   unsigned long country;
   char status;

   send_event2ap(papack, ACT_SEARCH, user->uin, user->status,
                 ipToIcq(user->ip), 52, longToTime(time(NULL)), "");

   /* :) Mirabilis staff is crazy enought to put tlv chain in single tlv ! */
   tlv.network_order();
   tlv_chain.readUTF(tlv, req_type);
   
   /* first query part - constant */
   not_implemented = True;
   snprintf(dbcomm_str, 4095, "SELECT users_info_ext.uin,nick,frst,last,email2,auth,email1,e1publ,webaware,sex,age,wtitle,wcompany,wdepart2,wweb,waddr,wcity,wstate,wzip2,wcountry,bcountry,bstate,bcity,haddr,hcity,hstate,hzip2,hcountry,hphon,hfax,hcell,wphon,wfax,martial,hweb,lang1,lang2,lang3,int_ind1,int_ind2,int_ind3,int_ind4,int_key1,int_key2,int_key3,int_key4,notes,tmp.active FROM users_info_ext LEFT JOIN (SELECT active,uin FROM online_users) AS tmp ON tmp.uin=users_info_ext.uin WHERE (1=1) ");

   /* uin stlv = 0x0032 */
   if ((stlv = tlv_chain.get(0x0032)) != NULL)
   {
      v7_extract_string(uin, *stlv, 32);
      convert_from_unicode(uin, sizeof(uin));
      convert_to_postgres(uin, sizeof(uin));

      snprintf(clause_temp, 255, "AND (users_info_ext.uin=%s) ", uin);

      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
      is_first_clause = False;
   }
   
   /* nick name stlv = 0x0078 */
   if ((stlv = tlv_chain.get(0x0078)) != NULL)
   {
      v7_extract_string(nick_str, *stlv, 32);
      convert_from_unicode(nick_str, sizeof(nick_str));      
      convert_to_postgres(nick_str, sizeof(nick_str));
      
      snprintf(clause_temp, 255, "AND (upper(nick) like upper('%%%s%%')) ", nick_str);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   /* first name stlv = 0x0064 */
   if ((stlv = tlv_chain.get(0x0064)) != NULL)
   {
      v7_extract_string(first_str, *stlv, 32);
      convert_from_unicode(first_str, sizeof(first_str));      
      convert_to_postgres(first_str, sizeof(first_str));
	 
      snprintf(clause_temp, 255, "AND (upper(frst) like upper('%%%s%%')) ", first_str);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   /* last name stlv = 0x006E */
   if ((stlv = tlv_chain.get(0x006E)) != NULL)
   {
      v7_extract_string(last_str,  *stlv, 32);
      convert_from_unicode(last_str, sizeof(last_str));      
      convert_to_postgres(last_str, sizeof(last_str));
      
      snprintf(clause_temp, 255, "AND (upper(last) like upper('%%%s%%')) ", last_str);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   /* email2 stlv = 0x0050 */
   if ((stlv = tlv_chain.get(0x0050)) != NULL)
   {
      v7_extract_string(email,  *stlv, 63);
      convert_from_unicode(email, sizeof(email));
      convert_to_postgres(email, sizeof(email));
      
      snprintf(clause_temp, 255, "AND (((upper(email1) like upper('%%%s%%')) AND (e1publ=0)) ", email);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      snprintf(clause_temp, 255, "OR (upper(email2) like upper('%%%s%%')) ", email);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      snprintf(clause_temp, 255, "OR (upper(email3) like upper('%%%s%%'))) ", email);
      safe_strcat(dbcomm_str, clause_temp, 4095);
      not_implemented = False;
   }

   /* Age range to search: stlv = 0x0154 */
   if ((stlv = tlv_chain.get(0x0154)) != NULL)
   {
      *stlv >> max_age >> min_age;
      
      max_age = ntohs(max_age);
      min_age = ntohs(min_age);
      
      if ((min_age != 0) && (max_age != 0) && (min_age < 61) && (max_age < 121)) 
      {
         snprintf(clause_temp, 255, "AND ((age >= %d) AND (age <= %d)) ", min_age, max_age);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }
   
   
   /* Gender. stlv = 0x0082 */
   if ((stlv = tlv_chain.get(0x0082)) != NULL)
   {
      *stlv >> gender;      
   
      if ((gender < 3) && (gender > 0))
      {
         snprintf(clause_temp, 255, "AND (sex = %d) ", gender);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }
   
   /* Language code stlv = 0x00FA */
   if ((stlv = tlv_chain.get(0x00FA)) != NULL)
   {
      *stlv >> language;      
      
      language = ntohs(language);
   
      if ((language > 0) && (language < 256))
      {
         snprintf(clause_temp, 255, "AND (%d in (lang1, lang2, lang3)) ", 
                  language);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }
   
   /* City stlv = 0x00A0 */
   if ((stlv = tlv_chain.get(0x00A0)) != NULL)
   {
      v7_extract_string(city,  *stlv, 32);
      convert_from_unicode(city, sizeof(city));      
      convert_to_postgres(city, sizeof(city));
      
      if (strlen(city) != 0) 
      {
         snprintf(clause_temp, 255, "AND (upper(hcity) like upper('%%%s%%')) ", city);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }
   
   
   /* state stlv = 0x019A */
/*   if ((stlv = tlv_chain.get(0x019A)) != NULL)
   {
      v7_extract_string(state, *stlv, 32, "v7 wp state", *user, pack);
      convert_to_postgres(state, sizeof(state));
      
      if (strlen(state) != 0) 
      {
         snprintf(clause_temp, 255, "AND ((upper(hstate) like upper('%%%s%%')) OR (upper(wstate) like upper('%%%s%%'))) ", state, state);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }
*/
   
   /* country code stlv = 0x00BE */
   if ((stlv = tlv_chain.get(0x00BE)) != NULL)
   {
      *stlv >> country;
      
      country = ntohl(country);
   
      if ((country > 0) && (country < 20000)) 
      {
         snprintf(clause_temp, 255, "AND ((hcountry=%lu) OR (wcountry=%lu)) ", country, country);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }
   
   /* wcompany stlv =  0x01AE */
/*   if ((stlv = tlv_chain.get(0x01AE)) != NULL)
   {
      v7_extract_string(company, *stlv, 32, "v7 wp company", *user, pack);
      convert_to_postgres(company, sizeof(company));
   
      if (strlen(company) != 0) 
      {
         snprintf(clause_temp, 255, "AND (upper(wcompany) like upper('%%%s%%')) ", company);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }
*/
   /* wposition stlv =  0x01C2 */
/*   if ((stlv = tlv_chain.get(0x01C2)) != NULL)
   {
      v7_extract_string(position, *stlv, 32, "v7 wp position", *user, pack);
      convert_to_postgres(position, sizeof(position));
      
      if (strlen(position) != 0) 
      {
         snprintf(clause_temp, 255, "AND (upper(wtitle) like upper('%%%s%%')) ", position);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }
*/   
   /* ocupation code stlv = 0x01CC */
/*   if ((stlv = tlv_chain.get(0x01CC)) != NULL)
   {
      if ((work_code > 0) && (work_code < 127))
      {
         snprintf(clause_temp, 255, "AND (wocup = %d) ", work_code);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }
*/   
   /* past info stlv = 0x01FE */
/*   if ((stlv = tlv_chain.get(0x01FE)) != NULL)
   {
      *stlv >> pcode;
      v7_extract_string(pkeys, *stlv, 63, "v7 wp past", *user, pack);
      convert_to_postgres(pkeys, sizeof(pkeys));
   
      if ((pcode > 0) && (pcode < 60000))
      {
         snprintf(clause_temp, 255, "AND (((past_ind1 = %d) AND (", pcode);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)pkeys;
         at_least_one_key = False;
         keys_cnt = 0;
         while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
   	    {  
	        snprintf(clause_temp, 255, 
	        "(upper(past_key1) like upper('%%%s%%')) OR ", token);
	        safe_strcat(dbcomm_str, clause_temp, 4095);
	        at_least_one_key = True;
	        keys_cnt++;
	        if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, "OR ", 4095);
         snprintf(clause_temp, 255, "((past_ind2 = %d) AND (", pcode);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)pkeys;
         at_least_one_key = False;
         keys_cnt = 0;

         while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
   	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(past_key2) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, "OR ", 4095);
         snprintf(clause_temp, 255, "((past_ind3 = %d) AND (", pcode);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)pkeys;
         at_least_one_key = False;
         keys_cnt = 0;
      
         while (next_token(&keys_ptr,token,",",sizeof(token))) 
	 {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(past_key3) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False ))", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True ))", 4095);
         }

         safe_strcat(dbcomm_str, ")", 4095);
         not_implemented = False;
      }
   }
*/   
   /* affilations info stlv = 0x01D6 */
/*   if ((stlv = tlv_chain.get(0x01D6)) != NULL)
   {
      *stlv >> aff_index;
      v7_extract_string(affkeys, *stlv, 127, "v7 wp affilations", *user, pack);
      convert_to_postgres(affkeys, sizeof(affkeys));
   
      if ((aff_index > 0) && (aff_index < 60000))
      {
         snprintf(clause_temp, 255, "AND (((aff_ind1 = %d) AND (", aff_index);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)affkeys;
         at_least_one_key = False;
         keys_cnt = 0;
         
	 while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(aff_key1) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, "OR ", 4095);
         snprintf(clause_temp, 255, "((aff_ind2 = %d) AND (", aff_index);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)affkeys;
         at_least_one_key = False;
         keys_cnt = 0;
      
         while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(aff_key2) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
  	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, "OR ", 4095);
         snprintf(clause_temp, 255, "((aff_ind3 = %d) AND (", aff_index);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)affkeys;
         at_least_one_key = False;
         keys_cnt = 0;
      
         while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(aff_key3) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
  	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, ")", 4095);
         not_implemented = False;
      }
   }
*/
   /* interests info stlv = 0x01EA */
/*   if ((stlv = tlv_chain.get(0x01EA)) != NULL)
   {
      *stlv >> int_index;
      v7_extract_string(interests_dir, *stlv, 127, "v7 wp intdir", *user, pack);
      convert_to_postgres(interests_dir, sizeof(interests_dir));
   
      if ((int_index > 0) && (int_index < 60000))
      {
         snprintf(clause_temp, 255, "AND (((int_ind1 = %d) AND (", int_index);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)interests_dir;
         at_least_one_key = False;
         keys_cnt = 0;
      
         while (next_token(&keys_ptr,token,",",sizeof(token))) 
	 {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(int_key1) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, "OR ", 4095);
         snprintf(clause_temp, 255, "((int_ind2 = %d) AND (", int_index);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)interests_dir;
         at_least_one_key = False;
         keys_cnt = 0;
         
	 while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(int_key2) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, "OR ", 4095);
         snprintf(clause_temp, 255, "((int_ind3 = %d) AND (", int_index);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)interests_dir;
         at_least_one_key = False;
         keys_cnt = 0;
      
         while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(int_key3) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, "OR ", 4095);
         snprintf(clause_temp, 255, "((int_ind4 = %d) AND (", int_index);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)interests_dir;
         at_least_one_key = False;
         keys_cnt = 0;
      
         while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(int_key4) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         } 
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }


         safe_strcat(dbcomm_str, ")", 4095);
         not_implemented = False;
      }
   }
*/ 
   /* home page category info stlv = 0x0212 */
/*   if ((stlv = tlv_chain.get(0x0212)) != NULL)
   {
      *stlv >> page_index;
      v7_extract_string(pagekeys, *stlv, 127, "v7 wp page", *user, pack);
      convert_to_postgres(pagekeys, sizeof(pagekeys));
   
      if ((page_index > 0) && (page_index < 60000))
      {
         snprintf(clause_temp, 255, "AND (((hpage_cat=%d) AND (", page_index);
         safe_strcat(dbcomm_str, clause_temp, 4095);

         keys_ptr = (char *)pagekeys;
         at_least_one_key = False;
         keys_cnt = 0;
      
         while (next_token(&keys_ptr,token,",",sizeof(token))) 
         {
            if (strlen(token) > 1) 
	    {  
	       snprintf(clause_temp, 255, 
	       "(upper(hpage_txt) like upper('%%%s%%')) OR ", token);
	       safe_strcat(dbcomm_str, clause_temp, 4095);
	       at_least_one_key = True;
	       keys_cnt++;
	       if (keys_cnt > 5) break;
	    }
         }

         if (at_least_one_key) 
         {
            safe_strcat(dbcomm_str, "False )) ", 4095);
         }
         else
         {
            safe_strcat(dbcomm_str, "True )) ", 4095);
         }

         safe_strcat(dbcomm_str, ")", 4095);
         not_implemented = False;
      }
   }
*/   
   /* search only online information stlv = 0x0136 */
   if ((stlv = tlv_chain.get(0x0136)) != NULL)
   {
      *stlv >> online_only;

      if (online_only) 
      {      
         safe_strcat(dbcomm_str, "AND (uin IN (SELECT uin FROM online_users)) ", 4095);
      }
   }

   /* search keyword stlv = 0x017C */
   if ((stlv = tlv_chain.get(0x017C)) != NULL)
   {
      if (not_implemented)
      {
         strncpy(finfo.nick,   "this ", 31);  
         strncpy(finfo.first,  "search ", 31);
         strncpy(finfo.email2, "yet", 31);
         strncpy(finfo.email1, "yet", 31);
         strncpy(finfo.last,   "not implemented", 31);
	
         strncpy(finfo.nuin,   "0", 31);
         finfo.auth = 1;
         finfo.e1publ = 0;
         finfo.webaware = 0;

         mr_send_utf8_found(pack, user, snac, req_seq, finfo, True, 0, True, 0, 0, req_type);
         return;
      }
   }
  
   /* Martial. stlv = 0x012C */
   if ((stlv = tlv_chain.get(0x012C)) != NULL)
   {
      *stlv >> martial;      
      
      martial = ntohs(martial);
   
      if ((martial < 256) && (martial > 0))
      {
         snprintf(clause_temp, 255, "AND (martial = %d) ", martial);
         safe_strcat(dbcomm_str, clause_temp, 4095);
         not_implemented = False;
      }
   }

   send_event2ap(papack, ACT_SEARCH, user->uin, user->status,                
                 ipToIcq(user->ip), 55, longToTime(time(NULL)), "UTF-8 search");

   /* now query is ready - we can run it */
   res = PQexec(users_dbconn, dbcomm_str);
   DEBUG(10, ("query: %s\n", dbcomm_str));
   
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[UTF-8 SEARCH]");
      DEBUG(10, ("req: %s\n", dbcomm_str));
      mr_send_utf8_found(pack, user, snac, req_seq, finfo, False, 0, True, 0, 0, req_type);
      return;
   }
   
   fsend = PQntuples(res);
    
   DEBUG(10, ("Search found %d users\n", fsend));
    
   if (fsend == 0) 
   {
      mr_send_utf8_found(pack, user, snac, req_seq, finfo, False, 0, True, 0, 0, req_type);
   }
      
   for(int i=0; i<fsend; i++)
   {      
      strncpy(finfo.nuin,   PQgetvalue(res, i,  0), sizeof(finfo.nuin)-1);
      strncpy(finfo.nick,   PQgetvalue(res, i,  1), sizeof(finfo.nick)-1);
      strncpy(finfo.first,  PQgetvalue(res, i,  2), sizeof(finfo.first)-1);
      strncpy(finfo.last,   PQgetvalue(res, i,  3), sizeof(finfo.last)-1);    
      strncpy(finfo.email2, PQgetvalue(res, i,  4), sizeof(finfo.email2)-1);
      strncpy(finfo.email1, PQgetvalue(res, i,  6), sizeof(finfo.email1)-1);
      strncpy(finfo.wtitle, PQgetvalue(res, i, 11), sizeof(finfo.wtitle)-1);
      strncpy(finfo.wcompany, PQgetvalue(res, i, 12), sizeof(finfo.wcompany)-1);
      strncpy(finfo.wdepart2, PQgetvalue(res, i, 13), sizeof(finfo.wdepart2)-1);
      strncpy(finfo.wpage, PQgetvalue(res, i, 14), sizeof(finfo.wpage)-1);
      strncpy(finfo.waddr, PQgetvalue(res, i, 15), sizeof(finfo.waddr)-1);
      strncpy(finfo.wcity, PQgetvalue(res, i, 16), sizeof(finfo.wcity)-1);
      strncpy(finfo.wstate, PQgetvalue(res, i, 17), sizeof(finfo.wstate)-1);
      strncpy(finfo.wzip2, PQgetvalue(res, i, 18), sizeof(finfo.wzip2)-1);
      strncpy(finfo.bstate, PQgetvalue(res, i, 21), sizeof(finfo.wstate)-1);
      strncpy(finfo.bcity, PQgetvalue(res, i, 22), sizeof(finfo.wcity)-1);
      strncpy(finfo.haddr, PQgetvalue(res, i, 23), sizeof(finfo.haddr)-1);
      strncpy(finfo.hcity, PQgetvalue(res, i, 24), sizeof(finfo.hcity)-1);
      strncpy(finfo.hstate, PQgetvalue(res, i, 25), sizeof(finfo.hstate)-1);
      strncpy(finfo.hzip2, PQgetvalue(res, i, 26), sizeof(finfo.hzip2)-1);
      strncpy(finfo.hphone, PQgetvalue(res, i, 28), sizeof(finfo.hphone)-1);
      strncpy(finfo.hfax, PQgetvalue(res, i, 29), sizeof(finfo.hfax)-1);
      strncpy(finfo.hcell, PQgetvalue(res, i, 30), sizeof(finfo.hcell)-1);
      strncpy(finfo.wphone, PQgetvalue(res, i, 31), sizeof(finfo.wphone)-1);
      strncpy(finfo.wfax, PQgetvalue(res, i, 32), sizeof(finfo.wfax)-1);
      strncpy(finfo.hpage, PQgetvalue(res, i, 34), sizeof(finfo.hpage)-1);
      strncpy(finfo.int_key1, PQgetvalue(res, i, 42), sizeof(finfo.int_key1)-1);
      strncpy(finfo.int_key2, PQgetvalue(res, i, 43), sizeof(finfo.int_key2)-1);
      strncpy(finfo.int_key3, PQgetvalue(res, i, 44), sizeof(finfo.int_key3)-1);
      strncpy(finfo.int_key4, PQgetvalue(res, i, 45), sizeof(finfo.int_key4)-1);
      strncpy(finfo.notes, PQgetvalue(res, i, 46), sizeof(finfo.notes)-1);
      

      ITrans.translateToClient(finfo.nuin);
      ITrans.translateToClient(finfo.nick);
      ITrans.translateToClient(finfo.first);
      ITrans.translateToClient(finfo.last);
      ITrans.translateToClient(finfo.email2);
      ITrans.translateToClient(finfo.email1);
      ITrans.translateToClient(finfo.wtitle);
      ITrans.translateToClient(finfo.wcompany);
      ITrans.translateToClient(finfo.wdepart2);
      ITrans.translateToClient(finfo.wpage);
      ITrans.translateToClient(finfo.waddr);
      ITrans.translateToClient(finfo.wcity);
      ITrans.translateToClient(finfo.wstate);
      ITrans.translateToClient(finfo.bcity);
      ITrans.translateToClient(finfo.bstate);
      ITrans.translateToClient(finfo.haddr);
      ITrans.translateToClient(finfo.hcity);
      ITrans.translateToClient(finfo.hstate);
      ITrans.translateToClient(finfo.hzip2);
      ITrans.translateToClient(finfo.hphone);
      ITrans.translateToClient(finfo.hfax);
      ITrans.translateToClient(finfo.hcell);
      ITrans.translateToClient(finfo.wphone);
      ITrans.translateToClient(finfo.wfax);
      ITrans.translateToClient(finfo.hpage);
      ITrans.translateToClient(finfo.int_key1);
      ITrans.translateToClient(finfo.int_key2);
      ITrans.translateToClient(finfo.int_key3);
      ITrans.translateToClient(finfo.int_key4);
      ITrans.translateToClient(finfo.notes);
      
      finfo.auth  = atol(PQgetvalue(res, i,  5));
      finfo.e1publ  = atol(PQgetvalue(res, i, 7));
      finfo.webaware = atol(PQgetvalue(res, i, 8));
      finfo.gender = atol(PQgetvalue(res, i, 9));
      finfo.age = atol(PQgetvalue(res, i, 10));
      finfo.wcountry = atol(PQgetvalue(res, i, 19));
      finfo.bcountry = atol(PQgetvalue(res, i, 20));
      finfo.hcountry = atol(PQgetvalue(res, i, 27));
      finfo.martial = atol(PQgetvalue(res, i, 33));
      finfo.lang1 = atol(PQgetvalue(res, i, 35));
      finfo.lang2 = atol(PQgetvalue(res, i, 36));
      finfo.lang3 = atol(PQgetvalue(res, i, 37));
      finfo.int_ind1 = atol(PQgetvalue(res, i, 38));
      finfo.int_ind2 = atol(PQgetvalue(res, i, 39));
      finfo.int_ind3 = atol(PQgetvalue(res, i, 40));
      finfo.int_ind4 = atol(PQgetvalue(res, i, 41));
      

      status = atol(PQgetvalue(res, i, 47));
       
      if ((i == (fsend - 1)) | (i == (lp_v7_maxsearch()-1))) 
      {
         if (fsend > lp_v7_maxsearch())
	 {
	    /* limit: last user found, but there is more users */
	    mr_send_utf8_found(pack, user, snac, req_seq, finfo, True, 0, True, 1, status, req_type);
	 }
	 else
	 {
	    /* last user found, there is no more users */
	    mr_send_utf8_found(pack, user, snac, req_seq, finfo, True, 1, True, 0, status, req_type);
	 }
      }
      else
      {
         /* next user found */
	 mr_send_utf8_found(pack, user, snac, req_seq, finfo, True, 1, False, 0, status, req_type);
      }
       
      if (i == (lp_v7_maxsearch()-1)) break;
      if (((i % 100) == 0) & (i != 0)) results_delay(100);

    }


    PQclear(res);
}

/**************************************************************************/
/* This func send search_uin					  	  */
/**************************************************************************/
void mr_send_utf8_found(Packet &pack, struct online_user *user, 
                      struct snac_header &snac, unsigned short req_seq, 
		      struct full_user_info &finfo, unsigned short success,
		      unsigned short flags, unsigned long last, 
		      unsigned long users_left, char status, unsigned short req_type)
{

   unsigned short cnt;
   char str64[64];
   char str128[128];   
   char str256[256];
   char str512[512];

   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();

   arpack << (unsigned short)SN_TYP_ICQxMESSxEXT
          << (unsigned short)SN_IME_MULTIxMESSxRESP
	  << (unsigned short)flags
	  << (unsigned  long)snac.id;

   /* I hate ICQ... I think AOL staff love to fuck standing in a hammock */
   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated(); 
   
   reply_pack << (unsigned  long)user->uin
	      << (unsigned short)META_RESP_INFORMATION
	      << (unsigned short)req_seq;
 
   reply_pack << (unsigned short)META_INFO_SEARCH_RESP;
   
		   
   reply_pack << (char)0x0a;

   if  (success) 
   {
        if (req_type==3)
	    reply_pack << (char)0x16;
	else 
	    reply_pack << (char)0x27;

	reply_pack << (char)0x02;      
   }
   else
   {
      reply_pack << (char)0x1d
    		 << (char)0x00;      
   }      
   
   reply_pack << (char)0x05
	      << (unsigned short)0x00b9
	      << (unsigned short)0x0004
	      << (unsigned long)0x00000000
	      << (unsigned short)0x0100
	      << (unsigned long)0x00000000
	      << (unsigned long)0x00000000
	      << (unsigned long)0x00000000;

   if (success)
   {
      reply_pack << (unsigned short)0x0100
    		 << (unsigned short)0x0100
		 << (unsigned short)0x0100;
		 
      reply_pack2.clearPacket();
      reply_pack2.network_order();
      reply_pack2.no_null_terminated();
      
      reply_pack2 << (unsigned short)0x0032
    		  << (unsigned short)strlen(finfo.nuin)
		  << finfo.nuin;

      if ((finfo.e1publ != 1) || (atoul(finfo.nuin) == user->uin))
      {
	     snprintf(str128, 127, finfo.email1);
      }
      else
      {
	     snprintf(str128, 127, finfo.email2);
      }

      convert_to_unicode(str128, sizeof(str128));
      
      reply_pack2 << (unsigned short)0x0050
    		  << (unsigned short)strlen(str128)
		  << str128;

      snprintf(str64, 63, finfo.first);
      convert_to_unicode(str64, sizeof(str64));
      
      reply_pack2 << (unsigned short)0x0064
    		  << (unsigned short)strlen(str64)
		  << str64;

      snprintf(str64, 63, finfo.last);
      convert_to_unicode(str64, sizeof(str64));
      
      reply_pack2 << (unsigned short)0x006E
    		  << (unsigned short)strlen(str64)
		  << str64;

      snprintf(str64, 63, finfo.nick);
      convert_to_unicode(str64, sizeof(str64));
      
      reply_pack2 << (unsigned short)0x0078
    		  << (unsigned short)strlen(str64)
		  << str64;

      reply_pack2 << (unsigned short)0x0082
    		  << (unsigned short)0x0001
		  << finfo.gender;

      // Home block
      reply_pack2 << (unsigned short)0x0096;
      
      reply_pack3.clearPacket();
      reply_pack3.network_order();
      reply_pack3.no_null_terminated();

      snprintf(str128, 127, finfo.haddr);
      convert_to_unicode(str128, sizeof(str128));
      
      reply_pack3 << (unsigned short)0x0064
    		  << (unsigned short)strlen(str128)
		  << str128;

      snprintf(str64, 63, finfo.hcity);
      convert_to_unicode(str64, sizeof(str64));
      
      reply_pack3 << (unsigned short)0x006e
    		  << (unsigned short)strlen(str64)
		  << str64;

      snprintf(str64, 63, finfo.hstate);
      convert_to_unicode(str64, sizeof(str64));
      
      reply_pack3 << (unsigned short)0x0078
    		  << (unsigned short)strlen(str64)
		  << str64;

      snprintf(str64, 63, finfo.hzip2);
      convert_to_unicode(str64, sizeof(str64));
      
      reply_pack3 << (unsigned short)0x0082
    		  << (unsigned short)strlen(str64)
		  << str64;

      reply_pack3 << (unsigned short)0x008c
    		  << (unsigned short)0x0004
		  << (unsigned long)finfo.hcountry;

      reply_pack2 << (unsigned short)(reply_pack3.size()+4);
      reply_pack2 << (unsigned short)0x0001;      
      reply_pack2 << (unsigned short)reply_pack3.size();      

      memcpy(reply_pack2.nextData, reply_pack3.buff, reply_pack3.size());
      reply_pack2.sizeVal += reply_pack3.size();
      
      reply_pack2.append();

      reply_pack2.network_order();
      reply_pack2.no_null_terminated();
      
      // Born block
      reply_pack2 << (unsigned short)0x00a0;
      
      reply_pack3.clearPacket();
      reply_pack3.network_order();
      reply_pack3.no_null_terminated();

      snprintf(str64, 63, finfo.bcity);
      convert_to_unicode(str64, sizeof(str64));
      
      reply_pack3 << (unsigned short)0x006e
    		  << (unsigned short)strlen(str64)
		  << str64;

      snprintf(str64, 63, finfo.bstate);
      convert_to_unicode(str64, sizeof(str64));
      
      reply_pack3 << (unsigned short)0x0078
    		  << (unsigned short)strlen(str64)
		  << str64;

      reply_pack3 << (unsigned short)0x008c
    		  << (unsigned short)0x0004
		  << (unsigned long)finfo.bcountry;

      reply_pack2 << (unsigned short)(reply_pack3.size()+4);
      reply_pack2 << (unsigned short)0x0001;      
      reply_pack2 << (unsigned short)reply_pack3.size();      

      memcpy(reply_pack2.nextData, reply_pack3.buff, reply_pack3.size());
      reply_pack2.sizeVal += reply_pack3.size();
      
      reply_pack2.append();

      reply_pack2.network_order();
      reply_pack2.no_null_terminated();      
 
      // Language block
      reply_pack2 << (unsigned short)0x00aa
    		  << (unsigned short)0x0002
		  << (unsigned short)finfo.lang1
		  << (unsigned short)0x00b4
    		  << (unsigned short)0x0002
		  << (unsigned short)finfo.lang2
		  << (unsigned short)0x00be
    		  << (unsigned short)0x0002
		  << (unsigned short)finfo.lang3;


      // Phone, fax, mobile block
      reply_pack2 << (unsigned short)0x00c8;
      
      reply_pack3.clearPacket();
      reply_pack3.network_order();
      reply_pack3.no_null_terminated();

      cnt = 0;
      
      if (finfo.hphone)
      {
	  reply_pack3 << (unsigned short)0x0012;
	  
	  snprintf(str128, 127, finfo.hphone);
	  convert_to_unicode(str128, sizeof(str128));
	  
	  reply_pack3 << (unsigned short)0x0064
		      << (unsigned short)strlen(str128)
		      << str128;
	  reply_pack3 << (unsigned short)0x006e
		      << (unsigned short)0x0002
		      << (unsigned short)0x0001;
	  cnt++;
      }

      if (finfo.hfax)
      {
	  reply_pack3 << (unsigned short)0x0012;
	  
	  snprintf(str128, 127, finfo.hfax);
	  convert_to_unicode(str128, sizeof(str128));
	  
	  reply_pack3 << (unsigned short)0x0064
		      << (unsigned short)strlen(str128)
		      << str128;
	  reply_pack3 << (unsigned short)0x006e
		      << (unsigned short)0x0002
		      << (unsigned short)0x0004;
	  cnt++;
      }

      if (finfo.hcell)
      {
	  reply_pack3 << (unsigned short)0x0012;
	  
	  snprintf(str128, 127, finfo.hcell);
	  convert_to_unicode(str128, sizeof(str128));
	  
	  reply_pack3 << (unsigned short)0x0064
		      << (unsigned short)strlen(str128)
		      << str128;
	  reply_pack3 << (unsigned short)0x006e
		      << (unsigned short)0x0002
		      << (unsigned short)0x0003;
	  cnt++;
      }

      if (finfo.wphone)
      {
	  reply_pack3 << (unsigned short)0x0012;
	  
	  snprintf(str128, 127, finfo.wphone);
	  convert_to_unicode(str128, sizeof(str128));
	  
	  reply_pack3 << (unsigned short)0x0064
		      << (unsigned short)strlen(str128)
		      << str128;
	  reply_pack3 << (unsigned short)0x006e
		      << (unsigned short)0x0002
		      << (unsigned short)0x0002;
	  cnt++;
      }

      if (finfo.wfax)
      {
	  reply_pack3 << (unsigned short)0x0012;
	  
	  snprintf(str128, 127, finfo.wfax);
	  convert_to_unicode(str128, sizeof(str128));
	  
	  reply_pack3 << (unsigned short)0x0064
		      << (unsigned short)strlen(str128)
		      << str128;
	  reply_pack3 << (unsigned short)0x006e
		      << (unsigned short)0x0002
		      << (unsigned short)0x0005;
	  cnt++;
      }

      reply_pack2 << (unsigned short)(reply_pack3.size()+2);
      
      reply_pack2 << (unsigned short)cnt;
      
      memcpy(reply_pack2.nextData, reply_pack3.buff, reply_pack3.size());
      reply_pack2.sizeVal += reply_pack3.size();
      
      reply_pack2.append();

      reply_pack2.network_order();
      reply_pack2.no_null_terminated();

      // home page
      snprintf(str256, 255, finfo.hpage);
      convert_to_unicode(str256, sizeof(str256));

      reply_pack2 << (unsigned short)0x00fa
    		  << (unsigned short)strlen(str256)
		  << str256;

      // Work block
      reply_pack2 << (unsigned short)0x0118;

      reply_pack3.clearPacket();
      reply_pack3.network_order();
      reply_pack3.no_null_terminated();

      snprintf(str128, 127, finfo.wtitle);
      convert_to_unicode(str128, sizeof(str128));

      reply_pack3 << (unsigned short)0x0064
    		  << (unsigned short)strlen(str128)
		  << str128;

      snprintf(str128, 127, finfo.wcompany);
      convert_to_unicode(str128, sizeof(str128));

      reply_pack3 << (unsigned short)0x006e
    		  << (unsigned short)strlen(str128)
		  << str128;

      snprintf(str128, 127, finfo.wdepart2);
      convert_to_unicode(str128, sizeof(str128));

      reply_pack3 << (unsigned short)0x007d
    		  << (unsigned short)strlen(str128)
		  << str128;

      snprintf(str256, 255, finfo.wpage);
      convert_to_unicode(str256, sizeof(str256));

      reply_pack3 << (unsigned short)0x0078
    		  << (unsigned short)strlen(str256)
		  << str256;

      snprintf(str128, 127, finfo.waddr);
      convert_to_unicode(str128, sizeof(str128));

      reply_pack3 << (unsigned short)0x00aa
    		  << (unsigned short)strlen(str128)
		  << str128;

      snprintf(str64, 63, finfo.wcity);
      convert_to_unicode(str64, sizeof(str64));
      
      reply_pack3 << (unsigned short)0x00b4
    		  << (unsigned short)strlen(str64)
		  << str64;

      snprintf(str64, 63, finfo.wstate);
      convert_to_unicode(str64, sizeof(str64));
      
      reply_pack3 << (unsigned short)0x00be
    		  << (unsigned short)strlen(str64)
		  << str64;

      snprintf(str64, 63, finfo.wzip2);
      convert_to_unicode(str64, sizeof(str64));
      
      reply_pack3 << (unsigned short)0x00c8
    		  << (unsigned short)strlen(str64)
		  << str64;

      reply_pack3 << (unsigned short)0x00d2
    		  << (unsigned short)0x0004
		  << (unsigned long)finfo.wcountry;

      reply_pack2 << (unsigned short)(reply_pack3.size()+4);
      reply_pack2 << (unsigned short)0x0001;      
      reply_pack2 << (unsigned short)reply_pack3.size();      
      
      memcpy(reply_pack2.nextData, reply_pack3.buff, reply_pack3.size());
      reply_pack2.sizeVal += reply_pack3.size();
      
      reply_pack2.append();

      reply_pack2.network_order();
      reply_pack2.no_null_terminated();      
      

      // Interest block
      reply_pack3.clearPacket();
      reply_pack3.network_order();
      reply_pack3.no_null_terminated();

      cnt = 0;

      if (finfo.int_ind1)
      {
	  snprintf(str128, 127, finfo.int_key1);
	  convert_to_unicode(str128, sizeof(str128));
	  
	  reply_pack3 << (unsigned short)(strlen(str128)+10)
		      << (unsigned short)0x0064
		      << (unsigned short)strlen(str128)
		      << str128
		      << (unsigned short)0x006e
		      << (unsigned short)0x0002
		      << (unsigned short)finfo.int_ind1;
	  cnt++;
      }

      if (finfo.int_ind2)
      {
	  snprintf(str128, 127, finfo.int_key2);
	  convert_to_unicode(str128, sizeof(str128));
	  
	  reply_pack3 << (unsigned short)(strlen(str128)+10)
		      << (unsigned short)0x0064
		      << (unsigned short)strlen(str128)
		      << str128
		      << (unsigned short)0x006e
		      << (unsigned short)0x0002
		      << (unsigned short)finfo.int_ind2;
	  cnt++;
      }

      if (finfo.int_ind3)
      {
	  snprintf(str128, 127, finfo.int_key3);
	  convert_to_unicode(str128, sizeof(str128));
	  
	  reply_pack3 << (unsigned short)(strlen(str128)+10)
		      << (unsigned short)0x0064
		      << (unsigned short)strlen(str128)
		      << str128
		      << (unsigned short)0x006e
		      << (unsigned short)0x0002
		      << (unsigned short)finfo.int_ind3;
	  cnt++;
      }

      if (finfo.int_ind4)
      {
	  snprintf(str128, 127, finfo.int_key4);
	  convert_to_unicode(str128, sizeof(str128));
	  
	  reply_pack3 << (unsigned short)(strlen(str128)+10)
		      << (unsigned short)0x0064
		      << (unsigned short)strlen(str128)
		      << str128
		      << (unsigned short)0x006e
		      << (unsigned short)0x0002
		      << (unsigned short)finfo.int_ind4;
	  cnt++;
      }

      if(cnt)
      {
    	  reply_pack2 << (unsigned short)0x0122;
	  
	  reply_pack2 << (unsigned short)(reply_pack3.size()+2);
	  
	  reply_pack2 << (unsigned short)cnt;
	  
	  memcpy(reply_pack2.nextData, reply_pack3.buff, reply_pack3.size());
	  reply_pack2.sizeVal += reply_pack3.size();
	  
	  reply_pack2.append();
	  reply_pack2.network_order();
	  reply_pack2.no_null_terminated();
      }


      // marital status
      reply_pack2 << (unsigned short)0x012c
    		  << (unsigned short)0x0002
		  << (unsigned short)finfo.martial;

      // About notes
      snprintf(str512, 511, finfo.notes);
      convert_to_unicode(str512, sizeof(str512));

      reply_pack2 << (unsigned short)0x0186
    		  << (unsigned short)strlen(str512)
		  << str512;

      // Online status (webaware depend)
      reply_pack2 << (unsigned short)0x0190
    		  << (unsigned short)0x0002
		  << (char)0x00;

      if (finfo.webaware)
      {		  
        reply_pack2 << (char)status;
      }
      else
      {
        reply_pack2 << (char)0x02;      
      }


      reply_pack2 << (unsigned short)0x019a
    		  << (unsigned short)0x0002
		  << (unsigned short)finfo.auth;

      reply_pack << (unsigned short)ntohs(reply_pack2.size());
      
      memcpy(reply_pack.nextData, reply_pack2.buff, reply_pack2.size());
      reply_pack.sizeVal += reply_pack2.size();
   }
   else
   {
      reply_pack << (unsigned long)0x00000000
    		 << (unsigned short)0x0000;   
   }
	       
   /* now time to put data into arpack */
   
   arpack << (unsigned short)0x0001
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending v7 utf8 user found info meta-reply to user %lu\n", user->uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);

}

/**************************************************************************/
/* Update utf8 user information command processor.		  	  */
/**************************************************************************/
void mr_utf8_set_req(Packet &pack, struct online_user *user, 
                      struct snac_header &snac, unsigned short req_seq, 
		      struct full_user_info &finfo, class tlv_c &tlv)
{
   PGresult *res;
   char ctemp;
   unsigned short stemp;
   unsigned long ltemp;
   BOOL is_first_clause = True;
   BOOL save_error = False;
   class tlv_chain_c tlv_chain;
   struct notes_user_info notes;
   class tlv_c *stlv;
   char clause_temp[512];
   char str64[64], str128[128], str256[256], str2050[2050];
   unsigned short req;
   class tlv_chain_c tlv_chain2;
   class tlv_c *stlv2;
   
   tlv.network_order();
   tlv.no_null_terminated();
   tlv_chain.readUTF(tlv, req);
   
   DEBUG(50, ("UTF-8 SaveInfo tlv-chain tlvs num=%d\n", tlv_chain.num()));

   /* first we should take user notice field (max - 500 chars) */
   if ((stlv = tlv_chain.get(0x0186)) != NULL)
   {
       if (v7_extract_string(str2050, *stlv, sizeof(str2050)-1))
       {
	  convert_from_unicode(str2050, sizeof(str2050)-1);      

          snprintf(notes.notes, sizeof(notes.notes)-1, "%s", str2050);

          if (db_users_setnotes(user->uin, notes) == -1) save_error = True;
          tlv_chain.remove(0x0186);
       }
       else save_error = True;
   }

   /* First part of the SQL update command */   
   strncpy(tempst, "UPDATE Users_Info_Ext Set ", 128);
   
   /* nick name stlv = 0x0078 */
   if (((stlv = tlv_chain.get(0x0078)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(str64, *stlv, sizeof(str64)-1))
      {
        convert_from_unicode(str64, sizeof(str64)-1);      
        convert_to_postgres(str64, sizeof(str64)-1);
      
        snprintf(clause_temp, sizeof(clause_temp)-1, "nick='%s'", str64);
        if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
        safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
        is_first_clause = False;
	tlv_chain.remove(0x0078);
      }
      else save_error = True;
   }

   /* first name stlv = 0x0064 */
   if (((stlv = tlv_chain.get(0x0064)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(str64, *stlv, sizeof(str64)-1))
      {
        convert_from_unicode(str64, sizeof(str64)-1);      
        convert_to_postgres(str64, sizeof(str64)-1);
      
        snprintf(clause_temp, sizeof(clause_temp)-1, "frst='%s'", str64);
        if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
        safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
        is_first_clause = False;
	tlv_chain.remove(0x0064);
      }
      else save_error = True;
   }

   /* last name stlv = 0x006E */
   if (((stlv = tlv_chain.get(0x006E)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(str64, *stlv, sizeof(str64)-1))
      {
        convert_from_unicode(str64, sizeof(str64)-1);      
        convert_to_postgres(str64, sizeof(str64)-1);
      
        snprintf(clause_temp, sizeof(clause_temp)-1, "last='%s'", str64);
        if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
        safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
        is_first_clause = False;
	tlv_chain.remove(0x006E);
      }
      else save_error = True;
   }

   // Homepage stlv = 0x00FA 
   if (((stlv = tlv_chain.get(0x00FA)) != NULL) && 
        (save_error != True))
   {
      if (v7_extract_string(str256, *stlv, sizeof(str256)-1))
      {
        convert_from_unicode(str256, sizeof(str256)-1);      
        convert_to_postgres(str256, sizeof(str256)-1);
      
        snprintf(clause_temp, sizeof(clause_temp)-1, "hweb='%s'", str256);
        if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
        safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
        is_first_clause = False;
	tlv_chain.remove(0x00FA);
      }
      else save_error = True;
   }


   /* Gender stlv = 0x0082 */
   if (((stlv = tlv_chain.get(0x0082)) != NULL) && 
       (save_error != True))
   {
	*stlv >> ctemp;

	snprintf(clause_temp, 255, "sex=%d", ctemp);
	if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
	safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
	is_first_clause = False;
	tlv_chain.remove(0x0082);
   }

   /* Lang1 stlv = 0x00AA */
   if (((stlv = tlv_chain.get(0x00AA)) != NULL) && 
       (save_error != True))
   {
	stlv->no_null_terminated();
	stlv->network_order();

	*stlv >> stemp;
	
	snprintf(clause_temp, 255, "lang1=%d", stemp);
	if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
	safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
	is_first_clause = False;
	tlv_chain.remove(0x00AA);
   }

   /* Lang2 stlv = 0x00B4 */
   if (((stlv = tlv_chain.get(0x00B4)) != NULL) && 
       (save_error != True))
   {
	stlv->no_null_terminated();
	stlv->network_order();

	*stlv >> stemp;
	
	snprintf(clause_temp, 255, "lang2=%d", stemp);
	if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
	safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
	is_first_clause = False;
	tlv_chain.remove(0x00B4);
   }
   
   /* Lang3 stlv = 0x00BE */
   if (((stlv = tlv_chain.get(0x00BE)) != NULL) && 
       (save_error != True))
   {
	stlv->no_null_terminated();
	stlv->network_order();

	*stlv >> stemp;

	snprintf(clause_temp, 255, "lang3=%d", stemp);
	if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
	safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
	is_first_clause = False;
	tlv_chain.remove(0x00BE);
   }

   /* Marital stlv = 0x012C */
   if (((stlv = tlv_chain.get(0x012C)) != NULL) && 
       (save_error != True))
   {
	stlv->no_null_terminated();
	stlv->network_order();

	*stlv >> stemp;
	
	snprintf(clause_temp, 255, "martial=%d", stemp);
	if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
	safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
	is_first_clause = False;
	tlv_chain.remove(0x012C);
   }

   // Home info block
   if (((stlv = tlv_chain.get(0x0096)) != NULL) && 
        (save_error != True))
   {
	stlv->network_order();
	tlv_chain2.readSub(*stlv);

	/* Home address stlv = 0x0064 */
	if (((stlv2 = tlv_chain2.get(0x0064)) != NULL) && 
    	    (save_error != True))
	{
	    if (v7_extract_string(str128, *stlv2, sizeof(str128)-1))
	    {
		convert_from_unicode(str128, sizeof(str128)-1);
		convert_to_postgres(str128, sizeof(str128)-1);
		
		snprintf(clause_temp, sizeof(clause_temp)-1, "haddr='%s'", str128);
		if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
		safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
		is_first_clause = False;
		tlv_chain2.remove(0x0064);
	    }
	    else save_error = True;
	}

	/* Home city stlv = 0x006E */
	if (((stlv2 = tlv_chain2.get(0x006E)) != NULL) && 
    	    (save_error != True))
	{
	    if (v7_extract_string(str64, *stlv2, sizeof(str64)-1))
	    {
		convert_from_unicode(str64, sizeof(str64)-1);
		convert_to_postgres(str64, sizeof(str64)-1);
		
		snprintf(clause_temp, sizeof(clause_temp)-1, "hcity='%s'", str64);
		if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
		safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
		is_first_clause = False;
		tlv_chain2.remove(0x006E);
	    }
	    else save_error = True;
	}

	/* Home state stlv = 0x0078 */
	if (((stlv2 = tlv_chain2.get(0x0078)) != NULL) && 
    	    (save_error != True))
	{
	    if (v7_extract_string(str64, *stlv2, sizeof(str64)-1))
	    {
		convert_from_unicode(str64, sizeof(str64)-1);
		convert_to_postgres(str64, sizeof(str64)-1);
		
		snprintf(clause_temp, sizeof(clause_temp)-1, "hstate='%s'", str64);
		if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
		safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
		is_first_clause = False;
		tlv_chain2.remove(0x0078);
	    }
	    else save_error = True;
	}

	/* Home zip2 stlv = 0x0082 */
	if (((stlv2 = tlv_chain2.get(0x0082)) != NULL) && 
    	    (save_error != True))
	{
	    if (v7_extract_string(str64, *stlv2, sizeof(str64)-1))
	    {
		convert_from_unicode(str64, sizeof(str64)-1);
		convert_to_postgres(str64, sizeof(str64)-1);
		
		snprintf(clause_temp, sizeof(clause_temp)-1, "hzip2='%s'", str64);
		if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
		safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
		is_first_clause = False;
		tlv_chain2.remove(0x0082);
	    }
	    else save_error = True;
	}

	/* Home country stlv = 0x008C */
	if (((stlv2 = tlv_chain2.get(0x008C)) != NULL) && 
    	    (save_error != True))
	{
	    stlv2->no_null_terminated();
	    stlv2->network_order();

	    *stlv2 >> ltemp;
	    
	    snprintf(clause_temp, 255, "hcountry=%d", (short)ltemp);
    	    if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
    	    safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
    	    is_first_clause = False;
    	    tlv_chain2.remove(0x008C);
	}
	tlv_chain.remove(0x0096);
   }


   // Work info block
   if (((stlv = tlv_chain.get(0x0118)) != NULL) && 
        (save_error != True))
   {
	stlv->network_order();
	tlv_chain2.readSub(*stlv);

	/* Position stlv = 0x0064 */
	if (((stlv2 = tlv_chain2.get(0x0064)) != NULL) && 
    	    (save_error != True))
	{
	    if (v7_extract_string(str128, *stlv2, sizeof(str128)-1))
	    {
		convert_from_unicode(str128, sizeof(str128)-1);
		convert_to_postgres(str128, sizeof(str128)-1);
		
		snprintf(clause_temp, sizeof(clause_temp)-1, "wtitle='%s'", str128);
		if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
		safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
		is_first_clause = False;
		tlv_chain2.remove(0x0064);
	    }
	    else save_error = True;
	}

	/* Company stlv = 0x006E */
	if (((stlv2 = tlv_chain2.get(0x006E)) != NULL) && 
    	    (save_error != True))
	{
	    if (v7_extract_string(str128, *stlv2, sizeof(str128)-1))
	    {
		convert_from_unicode(str128, sizeof(str128)-1);
		convert_to_postgres(str128, sizeof(str128)-1);
		
		snprintf(clause_temp, sizeof(clause_temp)-1, "wcompany='%s'", str128);
		if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
		safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
		is_first_clause = False;
		tlv_chain2.remove(0x006E);
	    }
	    else save_error = True;
	}

	/* Depart stlv = 0x007D */
	if (((stlv2 = tlv_chain2.get(0x007D)) != NULL) && 
    	    (save_error != True))
	{
	    if (v7_extract_string(str128, *stlv2, sizeof(str128)-1))
	    {
		convert_from_unicode(str128, sizeof(str128)-1);
		convert_to_postgres(str128, sizeof(str128)-1);
		
		snprintf(clause_temp, sizeof(clause_temp)-1, "wdepart2='%s'", str128);
		if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
		safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
		is_first_clause = False;
		tlv_chain2.remove(0x007D);
	    }
	    else save_error = True;
	}

	/* Website stlv = 0x0078 */
	if (((stlv2 = tlv_chain2.get(0x0078)) != NULL) && 
    	    (save_error != True))
	{
	    if (v7_extract_string(str256, *stlv2, sizeof(str256)-1))
	    {
		convert_from_unicode(str256, sizeof(str256)-1);
		convert_to_postgres(str256, sizeof(str256)-1);
		
		snprintf(clause_temp, sizeof(clause_temp)-1, "wweb='%s'", str256);
		if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
		safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
		is_first_clause = False;
		tlv_chain2.remove(0x0078);
	    }
	    else save_error = True;
	}

	/* Address stlv = 0x00AA */
	if (((stlv2 = tlv_chain2.get(0x00AA)) != NULL) && 
    	    (save_error != True))
	{
	    if (v7_extract_string(str128, *stlv2, sizeof(str128)-1))
	    {
		convert_from_unicode(str128, sizeof(str128)-1);
		convert_to_postgres(str128, sizeof(str128)-1);
		
		snprintf(clause_temp, sizeof(clause_temp)-1, "waddr='%s'", str128);
		if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
		safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
		is_first_clause = False;
		tlv_chain2.remove(0x00AA);
	    }
	    else save_error = True;
	}

	/* City stlv = 0x00B4 */
	if (((stlv2 = tlv_chain2.get(0x00B4)) != NULL) && 
    	    (save_error != True))
	{
	    if (v7_extract_string(str64, *stlv2, sizeof(str64)-1))
	    {
		convert_from_unicode(str64, sizeof(str64)-1);
		convert_to_postgres(str64, sizeof(str64)-1);
		
		snprintf(clause_temp, sizeof(clause_temp)-1, "wcity='%s'", str64);
		if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
		safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
		is_first_clause = False;
		tlv_chain2.remove(0x00B4);
	    }
	    else save_error = True;
	}

	/* State stlv = 0x00BE */
	if (((stlv2 = tlv_chain2.get(0x00BE)) != NULL) && 
    	    (save_error != True))
	{
	    if (v7_extract_string(str64, *stlv2, sizeof(str64)-1))
	    {
		convert_from_unicode(str64, sizeof(str64)-1);
		convert_to_postgres(str64, sizeof(str64)-1);
		
		snprintf(clause_temp, sizeof(clause_temp)-1, "wstate='%s'", str64);
		if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
		safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
		is_first_clause = False;
		tlv_chain2.remove(0x00BE);
	    }
	    else save_error = True;
	}

	/* Zip2 stlv = 0x00C8 */
	if (((stlv2 = tlv_chain2.get(0x00C8)) != NULL) && 
    	    (save_error != True))
	{
	    if (v7_extract_string(str64, *stlv2, sizeof(str64)-1))
	    {
		convert_from_unicode(str64, sizeof(str64)-1);
		convert_to_postgres(str64, sizeof(str64)-1);
		
		snprintf(clause_temp, sizeof(clause_temp)-1, "wzip2='%s'", str64);
		if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
		safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
		is_first_clause = False;
		tlv_chain2.remove(0x00C8);
	    }
	    else save_error = True;
	}

	/* Country stlv = 0x00D2 */
	if (((stlv2 = tlv_chain2.get(0x00D2)) != NULL) && 
    	    (save_error != True))
	{
	    stlv2->no_null_terminated();
	    stlv2->network_order();
	    *stlv2 >> ltemp;
	    
	    snprintf(clause_temp, 255, "wcountry=%d", (short)ltemp);
    	    if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
    	    safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
    	    is_first_clause = False;
    	    tlv_chain2.remove(0x00D2);
	}
	tlv_chain.remove(0x0118);	
   }


   // Born info block
   if (((stlv = tlv_chain.get(0x00A0)) != NULL) && 
        (save_error != True))
   {
	stlv->network_order();
	tlv_chain2.readSub(*stlv);

	/* City stlv = 0x006E */
	if (((stlv2 = tlv_chain2.get(0x006E)) != NULL) && 
    	    (save_error != True))
	{
	    if (v7_extract_string(str64, *stlv2, sizeof(str64)-1))
	    {
		convert_from_unicode(str64, sizeof(str64)-1);
		convert_to_postgres(str64, sizeof(str64)-1);
		
		snprintf(clause_temp, sizeof(clause_temp)-1, "bcity='%s'", str64);
		if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
		safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
		is_first_clause = False;
		tlv_chain2.remove(0x006E);
	    }
	    else save_error = True;
	}

	/* State stlv = 0x0078 */
	if (((stlv2 = tlv_chain2.get(0x0078)) != NULL) && 
    	    (save_error != True))
	{
	    if (v7_extract_string(str64, *stlv2, sizeof(str64)-1))
	    {
		convert_from_unicode(str64, sizeof(str64)-1);
		convert_to_postgres(str64, sizeof(str64)-1);
		
		snprintf(clause_temp, sizeof(clause_temp)-1, "bstate='%s'", str64);
		if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
		safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
		is_first_clause = False;
		tlv_chain2.remove(0x0078);
	    }
	    else save_error = True;
	}

	/* Country stlv = 0x008C */
	if (((stlv2 = tlv_chain2.get(0x008C)) != NULL) && 
    	    (save_error != True))
	{
	    *stlv2 >> ltemp;
	    
	    snprintf(clause_temp, 255, "bcountry=%d", (short)ltemp);
    	    if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
    	    safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
    	    is_first_clause = False;
    	    tlv_chain2.remove(0x008C);
	}
	tlv_chain.remove(0x00A0);
   }


   // Interest block
   if (((stlv = tlv_chain.get(0x0122)) != NULL) && 
        (save_error != True))
   {
	stlv->network_order();
	stlv->no_null_terminated();
	tlv_chain2.readSub2(*stlv);
	
	unsigned short num_ind = 0;

	for(unsigned short i=0;i<4;i++)
	{

	    /* Int_ind stlv = 0x(0123)06E */

	    if (((stlv2 = tlv_chain2.get(i*0x1000+0x006E)) != NULL) && 
		(save_error != True))
	    {
		stlv2->no_null_terminated();
		stlv2->network_order();

		*stlv2 >> stemp;
		
		num_ind++;    
		
		snprintf(clause_temp, 255, "int_ind%d=%d", num_ind, stemp);
		if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
		safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
		is_first_clause = False;
		tlv_chain2.remove(i*0x1000+0x006E);
		
		// Int_key stlv = 0x(0123)064
		if (((stlv2 = tlv_chain2.get(i*0x1000+0x0064)) != NULL) && 
		    (save_error != True))
		{
		    if (v7_extract_string(str128, *stlv2, sizeof(str128)-1))
		    {
			if (stemp)
			{
			convert_from_unicode(str128, sizeof(str128)-1);
			convert_to_postgres(str128, sizeof(str128)-1);
			}
			else memset(&str128,0,sizeof(str128));
			
			snprintf(clause_temp, sizeof(clause_temp)-1, "int_key%d='%s'", num_ind, str128);
			if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
			safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
			is_first_clause = False;
		    }
		    else save_error = True;
		    tlv_chain2.remove(i*0x1000+0x0064);

		}
	    }

	}

	// Write nums of interests
	snprintf(clause_temp, 255, "int_num=%d", num_ind);
	if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
	safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
	is_first_clause = False;
	
	tlv_chain.remove(0x0122);
   }

   // Phanes, faxes, cells block
   if (((stlv = tlv_chain.get(0x00C8)) != NULL) && 
        (save_error != True))
   {
	stlv->network_order();
	stlv->no_null_terminated();
	tlv_chain2.readSub2(*stlv);
	
	char name_field[32];
	
	bool have_error = false;
	
	for(unsigned short i=0;i<5;i++)
	{

	    /* Phone type stlv = 0x(01234)06E */

	    if (((stlv2 = tlv_chain2.get(i*0x1000+0x006E)) != NULL) && 
		(save_error != True))
	    {
		stlv2->no_null_terminated();
		stlv2->network_order();

		*stlv2 >> stemp;
		
		switch(stemp)
		{
		    case 0x0001:
			snprintf(name_field, 31, "hphon");
			break;
		    case 0x0002:
			snprintf(name_field, 31, "wphon");
			break;
		    case 0x0003:
			snprintf(name_field, 31, "hcell");
			break;
		    case 0x0004:
			snprintf(name_field, 31, "hfax");
			break;
		    case 0x0005:
			snprintf(name_field, 31, "wfax");
			break;
		    default:
			have_error = true;
			break;
		}
		
		tlv_chain2.remove(i*0x1000+0x006E);
		
		if(!have_error)
		{
		    // Phone number stlv = 0x(01234)064
		    if (((stlv2 = tlv_chain2.get(i*0x1000+0x0064)) != NULL) && 
			(save_error != True))
		    {
			if (v7_extract_string(str128, *stlv2, sizeof(str128)-1))
			{
			    convert_from_unicode(str128, sizeof(str128)-1);
			    convert_to_postgres(str128, sizeof(str128)-1);
			
			    snprintf(clause_temp, sizeof(clause_temp)-1, "%s='%s'", name_field, str128);
			    if (!is_first_clause) safe_strcat(tempst, ",", sizeof(tempst)-1);
			    safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
			    is_first_clause = False;
			    
			}
			else save_error = True;
			tlv_chain2.remove(i*0x1000+0x0064);
		    }
		}
	    }

	}

	tlv_chain.remove(0x00C8);
   }


   /* Last part of the query - WHERE clause */
   snprintf(clause_temp, 255, " WHERE uin=%lu", user->uin);
   safe_strcat(tempst, clause_temp, sizeof(tempst)-1);
   
   DEBUG(10, ("SQLQ: %s\n", tempst));
   
   /* Currently skipped fields */
   tlv_chain.remove(0x01f9);
   tlv_chain.remove(0x01a4);
   
   if (is_first_clause) save_error = True;

   /* Check if we have unhandled TLVs in chain */
   if ((save_error != True) && (tlv_chain.num() != 0))
   {
      DEBUG(10, ("SI (for %lu) changed, added %d tlvs, please report ICQ version\n", user->uin, tlv_chain.num()));
      LOG_SYS(0, ("SI (for %lu) changed, added %d tlvs, please report ICQ version\n", user->uin, tlv_chain.num()));
   }

   /* execute database SQL query */
   if (save_error != True)
   {
      res = PQexec(users_dbconn, tempst);
      if (PQresultStatus(res) != PGRES_COMMAND_OK)
      {
         handle_database_error(res, "[SET INFO FULL]");
         save_error = True;
      }
      else
      {
         if (atoul(PQcmdTuples(res)) == 0) save_error = True;
         PQclear(res);  
      }
   }

   /* Finish him!!! (send saveinfo result to client) */   
   if (save_error)
   {
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SET_RESULT, False, 0);
   }
   else
   {
      mr_send_set_ack(pack, user, snac, req_seq, META_INFO_SET_RESULT, True, 0);
   }
   // test_for_dummies();
}
