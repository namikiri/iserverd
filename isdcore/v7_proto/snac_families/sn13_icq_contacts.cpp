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
/* This module contain SSI service processing functions                   */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

unsigned short type_limits[SSI_TYPES_NUM];

/**************************************************************************/
/* Server-Side Information (SSI) service SNAC family packets handler	  */
/**************************************************************************/
void process_snac_ssi(struct snac_header &snac, Packet &pack)
{
   struct online_user *user = NULL;

   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip)));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   switch (snac.subfamily)
   {
      case SN_SSI_PARAMxREQUEST:   process_ssi_get_rights(snac, pack, user); break;
      case SN_SSI_ROASTERxREQUEST: process_ssi_request(snac, pack, user);    break;
      case SN_SSI_CHECKOUT:        process_ssi_checkout(snac, pack, user);   break;
      case SN_SSI_ACTIVATE:        process_ssi_activate(snac, pack, user);   break;
      case SN_SSI_ITEMxADD:        change_ssi(SSI_ADD, snac, pack, user);    break;
      case SN_SSI_ITEMxUPDATE:     change_ssi(SSI_UPDATE, snac, pack, user); break;
      case SN_SSI_ITEMxREMOVE:     change_ssi(SSI_REMOVE, snac, pack, user); break;
      case SN_SSI_EDITxBEGIN:      process_ssi_ch_begin(snac, pack, user);   break;
      case SN_SSI_EDITxEND:        process_ssi_ch_end(snac, pack, user);     break;
      case SN_SSI_AUTHxGRANT:      process_ssi_auth_grant(snac, pack, user); break;
      case SN_SSI_AUTHxSENDxREQ:   process_ssi_auth_req(snac, pack, user);   break;
      case SN_SSI_AUTHxSENDxREPLY: process_ssi_auth_rep(snac, pack, user);   break;
      
      default: DEBUG(10, ("Unknown ssi SNAC(0x13, %04X)\n", snac.subfamily));
               log_alarm_packet(0, pack);
   }
}


/**************************************************************************/
/* Request server limitations handler					  */
/**************************************************************************/
void process_ssi_get_rights(struct snac_header &snac, Packet &pack, 
                            struct online_user *user)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();

   arpack << (unsigned short)SN_TYP_SSI
          << (unsigned short)SN_SSI_PARAMxREPLY
	  << (unsigned short)0x0000
	  << (unsigned  long)snac.id

          << (unsigned short)0x0004 /* TLV(0x04) */
          << (unsigned short)(sizeof(unsigned short)*SSI_TYPES_NUM);
	  
	  for (int i=0; i<SSI_TYPES_NUM; i++)
	  {
	     arpack << (unsigned short)type_limits[i];
	  }
	  
   arpack << (unsigned short)0x0002 /* TLV(0x02) */
	  << (unsigned short)sizeof(unsigned short)
	  << (unsigned short)0x00FE /* unknown */

	  << (unsigned short)0x0003 /* TLV(0x03) */
	  << (unsigned short)sizeof(unsigned short)
	  << (unsigned short)0x01FC /* unknown */

	  << (unsigned short)0x0005 /* TLV(0x05) */
	  << (unsigned short)sizeof(unsigned short)
	  << (unsigned short)0x0064 /* unknown */

	  << (unsigned short)0x0006 /* TLV(0x06) */
	  << (unsigned short)sizeof(unsigned short)
	  << (unsigned short)0x0061 /* unknown */

	  << (unsigned short)0x0007 /* TLV(0x07) */
	  << (unsigned short)sizeof(unsigned short)
	  << (unsigned short)0x0000 /* unknown */

	  << (unsigned short)0x0008 /* TLV(0x08) */
	  << (unsigned short)sizeof(unsigned short)
	  << (unsigned short)0x0000 /* unknown */

	  << (unsigned short)0x0009 /* TLV(0x09) */
	  << (unsigned short)sizeof(unsigned long)
	  << (unsigned  long)0x00069780 /* unknown - 432000 */
   
	  << (unsigned short)0x000A /* TLV(0x0A) */
	  << (unsigned short)sizeof(unsigned long)
	  << (unsigned  long)0x0000000E; /* unknown */
   
   arpack.flap_channel = 0x02;
   arpack.sock_evt     = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* Request SSI from server handler					  */
/**************************************************************************/
void process_ssi_request(struct snac_header &snac, Packet &pack,
                         struct online_user *user)
{
   if (lp_v7_create_groups()) db_ssi_check_dgroup(user);
   user_send_ssi_data(snac, user);
}


/**************************************************************************/
/* SSI checkout handler							  */
/**************************************************************************/
void process_ssi_checkout(struct snac_header &snac, Packet &pack,
                          struct online_user *user)
{
   unsigned short item_num, citem_num;
   unsigned  long mod_date, cmod_date;
   
   pack >> cmod_date >> citem_num;

   if (lp_v7_create_groups()) db_ssi_check_dgroup(user);
   user_ssi_get_mod_info(user, mod_date, item_num);

   DEBUG(10, ("User SSI checkout: udate=%08X, inum=%d (db_udate=%08X, db_inum=%d)\n", 
             (unsigned int)cmod_date, citem_num, (unsigned int)mod_date, item_num));
   
   if ((mod_date == cmod_date) &&
       (item_num == citem_num))
   {
      /* user local copy up-to-date */
      pcopy(arpack, pack);
      arpack.clearPacket();
      arpack.setup_aim();

      arpack << (unsigned short)SN_TYP_SSI
             << (unsigned short)SN_SSI_UPxTOxDATE
   	     << (unsigned short)0x0000
	     << (unsigned  long)snac.id
	     
	     << (unsigned  long)mod_date
	     << (unsigned short)item_num;

      arpack.flap_channel = 0x02;
      arpack.sock_evt     = SOCK_DATA;
      tcp_writeback_packet(arpack);
   }
   else
   {
      /* user local copy not same */
      user_send_ssi_data(snac, user);
   }
}


/**************************************************************************/
/* Activate SSI handler							  */
/**************************************************************************/
void process_ssi_activate(struct snac_header &snac, Packet &pack,
                          struct online_user *user)
{
   /* activate SSI data (copy it into online_contacts table) */
   user_ssi_activate(user);
   user->cloaded = 1;
   
   if (user->active)
   {
      user_send_presense_full(user);
   }
}


/**************************************************************************/
/* SSI grant authorization handler					  */
/**************************************************************************/
void process_ssi_auth_grant(struct snac_header &snac, Packet &pack,
                            struct online_user *user)
{
   unsigned long to_uin;
   fstring reason;
   
   to_uin = read_buin(pack);
   if (!v7_extract_string(reason, pack, 254, "SSI grant reason")) return;

   /* add this authorization to ssi table :) for future use */
   grant_ssi_authorization(user->uin, to_uin);
}


/**************************************************************************/
/* SSI authorization request handler					  */
/**************************************************************************/
void process_ssi_auth_req(struct snac_header &snac, Packet &pack,
                          struct online_user *user)
{
   unsigned long to_uin;
   unsigned short chs_flg, unk_flg;
   unsigned short fbt=0, fbt2=0, i=0;
   struct online_user *to_user;
   struct msg_header msg_hdr;
   fstring reason;
   fstring charset;
   char buin[32];
   
   bzero((void *)&reason, sizeof(reason));
   bzero((void *)&charset, sizeof(charset));
   bzero((void *)&msg_hdr, sizeof(msg_hdr));
   
   to_uin = read_buin(pack);
   if (!v7_extract_string(reason, pack, 254, "SSI auth req reason")) return;
   pack >> chs_flg;
   
   if (chs_flg)
   {
      pack >> unk_flg;
      if (!v7_extract_string(charset, pack, 32, "SSI auth charset")) return;
   }

   /* find largest char code (put into fbt) */
   while (reason[i] != 0)
   {
      fbt2 = reason[i];
      if (fbt < fbt2) fbt = fbt2;
      i++;
   }
   
   /* I'm not sure this is correct */
   DEBUG(50, ("Extract charset: fbt=%d, charset='%s', charset_len=%d\n", 
               fbt, charset, strlen(charset)));
	       
   if ((fbt > 127) && (strlen(charset) == 0))
   {
      /* For stupid ICQ2003 client */
      strncpy(charset, "utf-8", 31);
   }

   if (((to_user = shm_get_user(to_uin)) != NULL) &&
       (to_user->enable_ssi))
   {
      arpack.clearPacket();
      arpack.setup_aim();
   
      snprintf(buin, 31, "%lu", user->uin);

      arpack << (unsigned short)SN_TYP_SSI
             << (unsigned short)SN_SSI_AUTHxREQ
   	     << (unsigned short)0x8000
	     << (unsigned  long)to_user->servseq2++
	     
	     /* For compatibility with badly written clients */
	     << (unsigned short)0x0006
	     << (unsigned short)0x0001
	     << (unsigned short)0x0002
	     << (unsigned short)0x0002
	     
	     << (char)strlen(buin)
	     << buin
	     << (unsigned short)strlen(reason)
	     << reason;
	     
      if (strlen(charset) != 0)
      {
         arpack << (unsigned short)0x0001
	        << (unsigned short)0x0001
		<< (unsigned short)strlen(charset)
		<< charset;
      }
      else
      {
         arpack << (unsigned short)0x0000;
      }	     

      arpack.from_ip      = to_user->ip;
      arpack.from_port    = 0;
      arpack.sock_hdl     = to_user->sock_hdl;
      arpack.sock_rnd     = to_user->sock_rnd;
      arpack.sock_evt     = SOCK_DATA;
      arpack.sock_type    = SAIM;
      arpack.flap_channel = 2;
   
      DEBUG(10, ("I'm going to send an auth request ssi packet to %lu\n", to_user->uin));
      tcp_writeback_packet(arpack);
   }
   else
   {
      /* notification via message because user offline or non-SSI capable */
      bzero((void *)&msg_hdr, sizeof(msg_hdr));
      msg_hdr.mtype     = MSG_TYPE_AUTH_REQ;
      msg_hdr.touin     = to_uin;
      msg_hdr.fromuin   = user->uin;
      msg_hdr.seq2      = 0;
      msg_hdr.from_ver  = V7_PROTO;
      msg_hdr.mtime     = timeToLong(time(NULL));
      msg_hdr.fromindex = user->shm_index;

      if (strlen(charset) != 0)
      {
         /* I can't fix this for now, i'll do this later via iconv */
	 snprintf(msg_buff, sizeof(msg_buff)-1, "-%s-%s-%s-%s1%s%s", 
	          CLUE_CHAR, CLUE_CHAR, CLUE_CHAR, CLUE_CHAR, CLUE_CHAR, 
		  "Message dropped by server because of charset");
      }
      else
      {
	 snprintf(msg_buff, sizeof(msg_buff)-1, "-%s-%s-%s-%s1%s%s", 
	          CLUE_CHAR, CLUE_CHAR, CLUE_CHAR, CLUE_CHAR, CLUE_CHAR, 
		  reason);
      }
      
      msg_hdr.msglen = strlen(msg_buff);
      process_message(msg_hdr, msg_buff);
   }
}


/**************************************************************************/
/* SSI authorization reply handler					  */
/**************************************************************************/
void process_ssi_auth_rep(struct snac_header &snac, Packet &pack,
                          struct online_user *user)
{
   unsigned long to_uin;
   unsigned short chs_flg, unk_flg;
   unsigned short fbt=0, fbt2=0, i=0;
   struct online_user *to_user;
   struct msg_header msg_hdr;
   fstring reason;
   fstring charset;
   char buin[32];
   char auth_state = 0;

   bzero((void *)&reason, sizeof(reason));
   bzero((void *)&charset, sizeof(charset));
   bzero((void *)&msg_hdr, sizeof(msg_hdr));
   
   to_uin = read_buin(pack);

   /* 1 - authorization accepted, 0-authorization declined */
   pack >> auth_state;
   if ((auth_state != 0) && (auth_state != 1)) auth_state=1;

   if (!v7_extract_string(reason, pack, 254, "SSI auth reply reason")) return;
   pack >> chs_flg;
   
   if (chs_flg)
   {
      pack >> unk_flg;
      if (!v7_extract_string(charset, pack, 32, "SSI auth charset")) return;
   }
   
   while (reason[i] != 0)
   {
      fbt2 = reason[i];
      if (fbt < fbt2) fbt = fbt2;
      i++;
   }

   /* I'm not sure this is correct */
   DEBUG(50, ("Extract charset: fbt=%d, charset='%s', charset_len=%d\n", 
               fbt, charset, strlen(charset)));
	       
   if ((fbt > 127) && (strlen(charset) == 0))
   {
      strncpy(charset, "utf-8", 31);
   }

   if (auth_state == 1)
   {
      grant_ssi_authorization(user->uin, to_uin);
   }
   else 
   {
      if (((to_user = shm_get_user(to_uin)) != NULL) &&
           (to_user->enable_ssi))
      {
         arpack.clearPacket();
         arpack.setup_aim();
   
         snprintf(buin, 31, "%lu", user->uin);

         arpack << (unsigned short)SN_TYP_SSI
                << (unsigned short)SN_SSI_AUTHxREPLY
   	        << (unsigned short)0x8000
	        << (unsigned  long)to_user->servseq2++
	     
	        /* For compatibility with badly written clients */
	        << (unsigned short)0x0006
	        << (unsigned short)0x0001
	        << (unsigned short)0x0002
	        << (unsigned short)0x0002
	        
	        << (char)strlen(buin)
	        << buin
	        << auth_state
	        << (unsigned short)strlen(reason)
	        << reason;

         if (strlen(charset) != 0)
         {
            arpack    << (unsigned short)0x0001
	              << (unsigned short)0x0001
	  	      << (unsigned short)strlen(charset)
	   	      << charset;
         }
         else
         {
            arpack << (unsigned short)0x0000;
         }
      
         arpack.from_ip      = to_user->ip;
         arpack.from_port    = 0;
         arpack.sock_hdl     = to_user->sock_hdl;
         arpack.sock_rnd     = to_user->sock_rnd;
	 arpack.sock_evt     = SOCK_DATA;
         arpack.sock_type    = SAIM;
         arpack.flap_channel = 2;
   
         DEBUG(10, ("I'm going to send an auth reply ssi packet to %lu\n", to_user->uin));
         tcp_writeback_packet(arpack);
      }
      else
      {
         /* notification via message because user offline or non-SSI capable */
         bzero((void *)&msg_hdr, sizeof(msg_hdr));
         msg_hdr.touin     = to_uin;
         msg_hdr.fromuin   = user->uin;
         msg_hdr.from_ver  = V7_PROTO;
         msg_hdr.mtime     = timeToLong(time(NULL));
         msg_hdr.fromindex = user->shm_index;
         msg_hdr.mtype     = MSG_TYPE_AUTH_DENIED;
      
         if (strlen(charset) != 0)
         {
            /* I can't fix this for now, i'll do this later via iconv */
            strncpy(msg_buff, "Msg text dropped by server because of charset", 128);
         }
         else
         {
            strncpy(msg_buff, reason, sizeof(msg_buff)-1);
         }
         
	 msg_hdr.msglen = strlen(msg_buff);
         process_message(msg_hdr, msg_buff);
      }
   }
}


/**************************************************************************/
/* SSI modification begin handler					  */
/**************************************************************************/
void process_ssi_ch_begin(struct snac_header &snac, Packet &pack,
                          struct online_user *user)
{
   unsigned long mode_flag = 0;

   /* SSI transaction begins */
   pack >> mode_flag;
   user->ssi_trans = 1;
   
   if ((lp_v7_enable_ssi_import()) && (mode_flag == 0x00010000))
   {
      DEBUG(0, ("SSI Import mode started (User %lu, ip=%s)\n", 
                 user->uin, inet_ntoa(pack.from_ip)));

      user->import_mode = 1;
   }
}


/**************************************************************************/
/* SSI modification end handler						  */
/**************************************************************************/
void process_ssi_ch_end(struct snac_header &snac, Packet &pack,
                        struct online_user *user)
{
   BOOL import_mode = False;

   if (user->import_mode == 1)
   {
      DEBUG(10, ("Import mode transaction closed...\n"));
      import_mode = True;
      user->import_mode = 0;
   }
   
   if (user->ssi_trans == 1)
   {
      /* we can close transaction and set mod_date */
      user_ssi_update_mod_info(user, import_mode);
      user->ssi_trans = 0;
   }  
}


/**************************************************************************/
/* SSI change command handler						  */
/**************************************************************************/
void change_ssi(unsigned short action_code, struct snac_header &snac, 
                Packet &pack, struct online_user *user)
{
   fstring item_name;
   int qresult = 0;
   BOOL need_presense = False;
   BOOL auth_needed = False;
   unsigned short err   = SSI_UPDATE_SUCCESS;
   unsigned short gid   = 0;
   unsigned short iid   = 0;
   unsigned short itype = 0;
   unsigned short size  = 0;
   unsigned long contact = 0;
   unsigned long rid = 0;
   class tlv_c *tlv     = NULL;
   class tlv_chain_c tlv_chain;
   struct login_user_info userinfo;

   bzero((void *)&userinfo, sizeof(userinfo));
   /* PQclear(PQexec(users_dbconn, "BEGIN")); */
   
   while ((pack.nextData < (pack.buff + pack.sizeVal)) && (err == SSI_UPDATE_SUCCESS))
   {
      rid = lrandom_num();
      auth_needed = False;
      
      /* === STAGE I: Extract item from packet */
      if (!v7_extract_string(item_name, pack, 128, "ssi item name"))
      {
         DEBUGADD(10, ("SSI_ERROR: Item name size too big...\n"));
         err = SSI_UPDATE_ERROR;
	 break;
      }
      
      pack >> gid >> iid >> itype >> size;
      
      /* copy additional data into allocated tlv, and read chain */
      if (pack.nextData + size > (pack.buff + pack.sizeVal))
      {
         DEBUGADD(10, ("SSI_ERROR: Item size exceeds packet length\n"));
         err = SSI_UPDATE_ERROR;
	 break;
      }
      else
      {
         tlv = new class tlv_c(0x0001, size);
	 tlv->setup_aim();
         memcpy((void *)tlv->value,(const void *)pack.nextData, size);
         pack.nextData += size;
         tlv_chain.read(*tlv);
	 tlv_chain.network_order();
	
	 DEBUGADD(250, ("SSI_CHG: additional blob size=%d, blob contain %d TLVs\n", 
	           size, tlv_chain.num()));
		   
         /* free unused structures */
         delete tlv; 
         tlv = NULL;
      }
            
      /* === Stage II: Validate extracted data */
      if (itype > SSI_TYPES_NUM)
      {
         DEBUGADD(10, ("SSI_ERROR: Can't add unknown item type (itype=%d)\n", itype));
         err = SSI_UPDATE_ERROR;
	 break;
      }
      
      if ((itype == ITEM_GROUP) && (iid != 0))
      {
         DEBUGADD(10, ("SSI_ERROR: Client sent group item with iid!=0\n"));
         err = SSI_UPDATE_ERROR;
	 break;      
      }

/* COMMENTED OUT: I'm not sure this is neccessary because */
/* ICQLite may add item and after that add its group .... */
#if 0
      /* Now we should check if BUDDY item has correct gid (group present) */
      if (itype == ITEM_BUDDY)
      {
         if (db_ssi_check_gid(user, gid) == False)
	 {
            DEBUGADD(10, ("SSI_ERROR: Item has invalid gid (group %d doesn't exist)\n", gid));
	    err = SSI_UPDATE_ERROR;
	    break;
	 }
      }
#endif

      /* Now time to check if we should ask authorization for contact */
      if ((itype == ITEM_BUDDY) && 
          ((tlv = tlv_chain.get(SSI_TLV_AUTH)) == NULL) && 
	  (action_code == SSI_ADD) &&
	  (user->import_mode != 1))
      {
         db_users_lookup(atoul(item_name), userinfo);
	 if (userinfo.auth != 1)
	 {
            /* check for authorization in ssi auth table */
	    if (!db_ssi_get_auth(userinfo.uin, user->uin))
	    {
               DEBUGADD(10, ("SSI_ERROR: Client not authorized to add a contact\n"));
               err = SSI_UPDATE_AUTH_REQUIRED;
            }
	 }
      }
      else
      {
         if (tlv_chain.get(SSI_TLV_AUTH) != NULL) auth_needed = True;
      }
      
      if (err == SSI_UPDATE_SUCCESS)
      {
         switch (action_code)
         {
            case SSI_ADD:

                 qresult = ssi_db_add_item(user, item_name, gid, iid, itype, tlv_chain);
		 user->cloaded = 1;
		 
                 if (qresult != 0)
                 {
		    if (qresult == -2)
		    {
                       DEBUGADD(10, ("SSI_ERROR: Can't add item because of limits\n"));
                       err = SSI_UPDATE_LIMIT;
                       break;		    
		    }
		 
                    DEBUGADD(10, ("SSI_ERROR: Can't add item because of db error\n"));
                    err = SSI_UPDATE_ERROR;
                    break;
                 }
		 else
		 {
		    if (auth_needed != True)
		    {
		       contact = atoul(item_name);
		    
		       /* Well, I'm not sure... :( */
		       if ((itype == ITEM_BUDDY) && (contact != 0) &&
		           (user->import_mode != 1))
		       {
		          send_ssi_added(contact, user->uin);
		       }
		    
		       /* Now if user activated i'll send presense to client */
		       if ((itype == ITEM_BUDDY) && (user->active))
		       {
		          if (contact != 0)
		          {
		             db_contact_insert(user->uin, 1, &contact, NORMAL_CONTACT, rid);
			     need_presense = True;
		          }
		       }
		    }
		 }
	         break;
	     
            case SSI_REMOVE:

                 if (ssi_db_del_item(user, item_name, gid, iid, itype, tlv_chain) != 0)
                 {
                    DEBUGADD(10, ("SSI_ERROR: Can't del item because of db error\n"));
                    err = SSI_UPDATE_NOT_FOUND;
                    break;
                 }
		 else
		 {
		    /* Now if user activated i'll send presense to client */
		    if ((itype == ITEM_BUDDY) && (user->active))
		    {
		       if ((contact = atoul(item_name)) != 0)
		       {
		          db_contact_delete(user->uin, contact, NORMAL_CONTACT);
		       }
		    }
		 }
	         break;
	      
            case SSI_UPDATE:
	 
                 qresult = ssi_db_update_item(user, item_name, gid, iid, itype, tlv_chain);
	         if (qresult != 0)
                 {
		    if (qresult == 1)
		    {
                       DEBUGADD(10, ("SSI_ERROR: Can't update item because it doesn't exist\n"));
                       err = SSI_UPDATE_NOT_FOUND;
                       break;		    
		    }
		    
                    DEBUGADD(10, ("SSI_ERROR: Can't update item - db error (code=%d)\n", qresult));
                    err = SSI_UPDATE_ERROR;
                    break;
                 }
	         break;
         }
      }
      
      tlv_chain.free();
   }
   
   if (err == SSI_UPDATE_SUCCESS)
   {
      /* Item was added sucessfully, send ack packet  */
      /* PQclear(PQexec(users_dbconn, "END")); */
      send_ssi_update_ack(snac, user, SSI_UPDATE_SUCCESS);
      if (need_presense) user_send_presense_part(user, rid);
      
   }
   else
   {  
      /* There was an error - abort transaction, send error */
      /* PQclear(PQexec(users_dbconn, "ROLLBACK")); */
      send_ssi_update_ack(snac, user, err);
   }
}


/**************************************************************************/
/* Send SSI data to user 						  */
/**************************************************************************/
void user_send_ssi_data(struct snac_header &snac, struct online_user *user)
{
   PGresult *res;
   fstring dbcomm_str;
   fstring iname;
   fstring nickname;
   class tlv_chain_c tlv_chain;
   
   unsigned short item_num    = 0;
   unsigned  long udate      = 0;
   unsigned short items_num  = 0;
   unsigned short items_sent = 0;
   unsigned short pack_items = 0;
   unsigned short snac_flags = 0;
   
   unsigned short gid    = 0;
   unsigned short iid    = 0;
   unsigned short type   = 0;
   unsigned short auth   = 0;
   unsigned  long iperm  = 0;
   unsigned  long vclass = 0;
   unsigned  long perms  = 0;
   unsigned  long utime  = 0;
   char privacy = 0;
   
   db_ssi_get_modinfo(user, udate, item_num);

   /* OK... first we should get data from database */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, 
           "SELECT * FROM Users_SSI_Data WHERE ouin=%lu ORDER BY gid,iid,type ASC", user->uin);
	    
   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[GET SSI DATA]");
      return;
   }

   if (PQnfields(res) != USSI_TBL_FIELDS) 
   {
      LOG_SYS(0, ("Corrypted ssi table structure in db: \"%s\"\n", lp_db_users()));
      exit(EXIT_ERROR_DB_STRUCTURE);
   }
   
   items_num = PQntuples(res);
   
   DEBUG(10, ("SSI_REPLY: database query returned %d records\n", items_num));
   
   if (items_num > 0) 
   {
      /* Here we should loop thry records and put them into     */
      /* SNAC(13,6). We should split roaster into several snacs */
      /* if it too large (pack size > 2,5K) 			*/
      while (items_sent < items_num)
      {
         arpack.clearPacket();
         arpack.setup_aim();
	 reply_pack.clearPacket();
	 reply_pack.setup_aim();
	 pack_items = 0;
	 
	 while (items_sent < items_num)
	 {  
	    reply_pack2.clearPacket();
	    reply_pack2.setup_aim();

	    type = atol(PQgetvalue(res, items_sent, 4));

	    if (type < 0x15)
	    {
	       strncpy(iname, PQgetvalue(res, items_sent, 9), sizeof(iname)-1);
	       strncpy(tempst, PQgetvalue(res, items_sent, 11), sizeof(tempst)-1);
	       strncpy(nickname, PQgetvalue(res, items_sent, 10), sizeof(nickname)-1);
	       
	       gid     = atol(PQgetvalue(res, items_sent, 2));
	       iid     = atol(PQgetvalue(res, items_sent, 3));
	       auth    = atol(PQgetvalue(res, items_sent, 6));
	       utime   = atoul(PQgetvalue(res, items_sent, 7));
	       iperm   = atoul(PQgetvalue(res, items_sent, 12));
	       vclass  = atoul(PQgetvalue(res, items_sent, 14));
	       perms   = atoul(PQgetvalue(res, items_sent, 15));
	       privacy = (char)atoi(PQgetvalue(res, items_sent, 13));
	    
	       /* Add nickname as additional data */
	       if (strlen(nickname) > 0)
	       {
	          reply_pack2 << (unsigned short)SSI_TLV_NICKNAME
		              << (unsigned short)strlen(nickname)
			      << nickname;
	       }
	       
	       /* Add authorization flag as additional data */
	       if (auth != 0)
	       {
	          reply_pack2 << (unsigned short)SSI_TLV_AUTH
		              << (unsigned short)0x0000;
	       }

	       /* Add iperms dword as additional data */
	       if (iperm != 0)
	       {
	          reply_pack2 << (unsigned short)SSI_TLV_IDLEPERMS
		              << (unsigned short)sizeof(iperm)
			      << (unsigned  long)iperm;
	       }

	       /* Add vclass dword as additional data */
	       if (vclass != 0)
	       {
	          reply_pack2 << (unsigned short)SSI_TLV_RIGHTS
		              << (unsigned short)sizeof(vclass)
			      << (unsigned  long)vclass;
	       }

	       /* Add perms dword as additional data */
	       if (perms != 0)
	       {
	          reply_pack2 << (unsigned short)SSI_TLV_PRESENSE
		              << (unsigned short)sizeof(perms)
			      << (unsigned  long)perms;
	       }

	       /* Add privacy flag char as additional data */
	       if (privacy != 0)
	       {
	          reply_pack2 << (unsigned short)SSI_TLV_PRIVACY
		              << (unsigned short)sizeof(privacy)
			      << (char)privacy;
	       }

	       /* Add import time as additional data */
	       if (type == ITEM_IMPORT)
	       {
	          reply_pack2 << (unsigned short)SSI_TLV_IMPORTTIME
		              << (unsigned short)sizeof(utime)
			      << (unsigned  long)utime;
	       }

	       /* Additional data should be decoded and appended here */
	       tlv_chain.decode(tempst);
	       tlv_chain.addToPacket(reply_pack2);
	       tlv_chain.free();
	       
	       /* New check packet size (need less than MTU+some_bytes)*/
	       if ((reply_pack.size() + reply_pack2.size()) <= 1400)
	       {
		    reply_pack  << (unsigned short)strlen(iname)
	        		<< iname
	   	        	<< (unsigned short)gid
		        	<< (unsigned short)iid
		        	<< (unsigned short)type
		        	<< (unsigned short)reply_pack2.sizeVal;
			   
		    memcpy(reply_pack.nextData, reply_pack2.buff, reply_pack2.size());
            	    reply_pack.sizeVal += reply_pack2.size();
	    	    reply_pack.append();
	    	    reply_pack.setup_aim();
	       
  	    	    pack_items++;
	       }
	       else
	       {
	    	    break;
	       }
	    }
	    
	    items_sent++;
	 }

	 /* If this is not last packet (roster was splitted) we */
	 /* should set SNAC flags bit0=1 */
	 snac_flags = 0;
	 if (items_sent != items_num) snac_flags = snac_flags | 0x0001;
//	 if (user->ssi_version > 0) snac_flags = snac_flags | 0x8000;
	 
	 /* now we can create reply packet */
         arpack << (unsigned short)SN_TYP_SSI
                << (unsigned short)SN_SSI_ROASTERxREPLY
                << (unsigned short)snac_flags
	        << (unsigned  long)snac.id;

//         if (user->ssi_version > 0)
//	 {
//	    DEBUG(10, ("SSI_REPLY: Sent compatibility info in snac header\n"));
//	    
//	    arpack << (unsigned short)0x0006 /* extinfo size */
//	           << (unsigned short)0x0001 /* TLV(1)   */
//		   << (unsigned short)0x0002 /* tlv size */
//		   << (unsigned short)user->ssi_version;
//	 }
	 
         arpack << (char)0x00 /* ssi protocol version */
                << (unsigned short)pack_items;

         reply_pack << (unsigned long)udate;
    	 memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
         arpack.sizeVal += reply_pack.size();
	 
         arpack.from_ip      = user->ip;
         arpack.from_port    = 0;
         arpack.sock_hdl     = user->sock_hdl;
         arpack.sock_rnd     = user->sock_rnd;
	 arpack.sock_evt     = SOCK_DATA;
         arpack.sock_type    = SAIM;
         arpack.flap_channel = 2;
         
         tcp_writeback_packet(arpack);
      }
   
      PQclear(res);
      return;
   }
   else 
   {
      arpack.clearPacket();
      arpack.setup_aim();

      DEBUG(10, ("User SSI data is empty... I'll send empty roaster\n"));

      snac_flags = 0;
//      if (user->ssi_version > 0) snac_flags = snac_flags | 0x8000;

       /* user ssi list is empty */
      arpack << (unsigned short)SN_TYP_SSI
             << (unsigned short)SN_SSI_ROASTERxREPLY
             << (unsigned short)snac_flags
             << (unsigned  long)snac.id;
	     
//      if (user->ssi_version > 0)
//      {
//         DEBUG(10, ("SSI_REPLY: Sent compatibility info in snac header\n"));
//    
//         arpack << (unsigned short)0x0006
//	        << (unsigned short)0x0001 /* TLV(1) */
//		<< (unsigned short)0x0002
//		<< (unsigned short)user->ssi_version;
//      }

      arpack << (char)0x00
             << (unsigned short)0x0000
	     << (unsigned  long)udate;

      arpack.from_ip      = user->ip;
      arpack.from_port    = 0;
      arpack.sock_hdl     = user->sock_hdl;
      arpack.sock_rnd     = user->sock_rnd;
      arpack.sock_evt     = SOCK_DATA;
      arpack.sock_type    = SAIM;
      arpack.flap_channel = 2;

      tcp_writeback_packet(arpack);	 
   
      PQclear(res);
      return;
   }
}


/**************************************************************************/
/* Activate SSI data: copy buddy records to online_contacts table 	  */
/**************************************************************************/
void user_ssi_activate(struct online_user *user)
{
   PGresult *res;

#define SSI_ACT425 \
"INSERT INTO Online_Contacts \
    SELECT ouin,uin,itype FROM Users_SSI_Data \
    WHERE (ouin=%lu) AND (itype<%d) AND (auth=0)"

   fstring dbcomm_str;
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, SSI_ACT425, user->uin, 50);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SSI ACTIVATE]");
   }
}


/**************************************************************************/
/* Get SSI data last modification time and SSI items number	 	  */
/**************************************************************************/
void user_ssi_get_mod_info(struct online_user *user, unsigned long &mod_date, 
                           unsigned short &item_num)
{
   db_ssi_get_modinfo(user, mod_date, item_num);
}


/**************************************************************************/
/* Update SSI data last modification time and SSI			  */
/**************************************************************************/
void user_ssi_update_mod_info(struct online_user *user, BOOL import)
{
   db_ssi_set_modinfo(user, import);
}


/**************************************************************************/
/* Send SSI change ack							  */
/**************************************************************************/
void send_ssi_update_ack(struct snac_header &snac, struct online_user *user,
                         unsigned short result_code)
{
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_SSI
          << (unsigned short)SN_SSI_CHANGExACK
          << (unsigned short)0x0000
          << (unsigned  long)snac.id
          << (unsigned short)result_code;

   arpack.from_ip      = user->ip;
   arpack.from_port    = 0;
   arpack.sock_hdl     = user->sock_hdl;
   arpack.sock_rnd     = user->sock_rnd;
   arpack.sock_evt     = SOCK_DATA;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 2;

   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* Authorize SSI user, add it to online_contacts, send presense 	  */
/**************************************************************************/
void grant_ssi_authorization(unsigned long from_uin, unsigned long to_uin)
{
   struct online_user *tuser = NULL;
   struct msg_header msg_hdr;
   unsigned long contact = from_uin;
   unsigned long rid = 0;

   DEBUG(10, ("Authorization granted from %lu to %lu\n", from_uin, to_uin));
   
   bzero((void *)&msg_hdr, sizeof(msg_hdr));

   rid = lrandom_num();
   db_ssi_auth_add(from_uin, to_uin, 0);
   tuser = shm_get_user(to_uin);
   
   if (db_ssi_auth_grant(from_uin, to_uin))
   {
      /* User authorized succesfully, now we should add record to  */
      /* online_contacts and send its presense to auth requester   */
      
      if ((tuser != NULL) && 
          (tuser->enable_ssi))
      {
         send_item_auth_update(from_uin, tuser);
	 ssi_send_auth_granted(from_uin, tuser);
         db_contact_insert(tuser->uin, 1, &contact, NORMAL_CONTACT, rid);
         user_send_presense_part(tuser, rid);
      }
      else
      {
         /* notification via message because user offline */
         bzero((void *)&msg_hdr, sizeof(msg_hdr));
         msg_hdr.touin     = to_uin;
         msg_hdr.fromuin   = from_uin;
         msg_hdr.from_ver  = V7_PROTO;
         msg_hdr.mtime     = timeToLong(time(NULL));
         msg_hdr.mtype     = MSG_TYPE_AUTH_GRANTED;
         msg_buff[0]       = 0;
	 
	 msg_hdr.msglen = strlen(msg_buff);
         process_message(msg_hdr, msg_buff);         
      }
   }
   else
   {
      /* Add authorization for future use */
      db_ssi_auth_add(from_uin, to_uin, 0);

      if ((tuser != NULL) && 
          (tuser->enable_ssi))
      {
	 /* skip this for now because i'm not sure */
      }
      else
      {
         /* notification via message because user offline */
         bzero((void *)&msg_hdr, sizeof(msg_hdr));
         msg_hdr.touin     = to_uin;
         msg_hdr.fromuin   = from_uin;
         msg_hdr.from_ver  = V7_PROTO;
         msg_hdr.mtime     = timeToLong(time(NULL));
         msg_hdr.mtype     = MSG_TYPE_AUTH_GRANTED;
         msg_buff[0]       = 0;
	 
         process_message(msg_hdr, msg_buff);         
      }      
   }
}


/**************************************************************************/
/* Send SSI "You were added" message					  */
/**************************************************************************/
void send_ssi_added(unsigned long uin, unsigned long by_uin)
{
   struct online_user *user;
   struct msg_header msg_hdr;
   char buin[32];

   bzero((void *)&msg_hdr, sizeof(msg_hdr));
   
   /* First we should check if we should alert this user */
   if (db_ssi_get_added(uin, by_uin)) return;
   db_ssi_added_add(uin, by_uin, 0);
   
   if (((user = shm_get_user(uin)) != NULL) &&
        (user->enable_ssi))
   {
      arpack.clearPacket();
      arpack.setup_aim();
   
      snprintf(buin, 31, "%lu", by_uin);

      DEBUG(10, ("SSI 'You were added' notification from %lu to %lu\n", by_uin, uin));

      arpack << (unsigned short)SN_TYP_SSI
             << (unsigned short)SN_SSI_YOUxWERExADDED
             << (unsigned short)0x0000
             << (unsigned  long)user->servseq2++
             << (char)strlen(buin)
	     << buin;

      arpack.from_ip      = user->ip;
      arpack.from_port    = 0;
      arpack.sock_hdl     = user->sock_hdl;
      arpack.sock_rnd     = user->sock_rnd;
      arpack.sock_evt     = SOCK_DATA;
      arpack.sock_type    = SAIM;
      arpack.flap_channel = 2;

      tcp_writeback_packet(arpack);
   }
   else
   {
      DEBUG(10, ("Non-SSI 'You were added' notification from %lu to %lu\n", by_uin, uin));

      /* notification via message because user offline or non-SSI capable */
      bzero((void *)&msg_hdr, sizeof(msg_hdr));
      msg_hdr.mtype     = MSG_TYPE_ADDED;
      msg_hdr.touin     = uin;
      msg_hdr.fromuin   = by_uin;
      msg_hdr.seq2      = 0;
      msg_hdr.from_ver  = V7_PROTO;
      msg_hdr.mtime     = timeToLong(time(NULL));

      struct full_user_info userinfo;
      bzero((void *)&userinfo, sizeof(userinfo));
      if (db_users_lookup_short(msg_hdr.fromuin, userinfo) == 0)
      {
         snprintf(msg_buff, 255, "%s%s%s%s%s%s%s%s1", userinfo.nick, CLUE_CHAR,
	          userinfo.first, CLUE_CHAR, userinfo.last, CLUE_CHAR, 
		  userinfo.email1, CLUE_CHAR);
      }
      else
      {
         DEBUG(50, ("Well, db_new_lookup(%lu) returned error. Empty msg sent\n", 
	             msg_hdr.fromuin));
		     
         snprintf(msg_buff, 255, "-%s-%s-%s-%s1", 
                  CLUE_CHAR, CLUE_CHAR, CLUE_CHAR, CLUE_CHAR);
      }

      msg_hdr.msglen    = strlen(msg_buff);
      process_message(msg_hdr, msg_buff);
   }
}


/**************************************************************************/
/* Send auth update packets						  */
/**************************************************************************/
void send_item_auth_update(unsigned long from_uin, struct online_user *to_user)
{
   PGresult *res;
   fstring dbcomm_str;
   fstring iname;
   fstring nickname;
   class tlv_chain_c tlv_chain;
   
   unsigned short gid    = 0;
   unsigned short iid    = 0;
   unsigned short type   = 0;
   unsigned short auth   = 0;

   /* OK... first we should get data from database */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, 
           "SELECT * FROM Users_SSI_Data WHERE (ouin=%lu) AND (uin=%lu) AND (type=%d)", 
	    to_user->uin, from_uin, ITEM_BUDDY);
	    
   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[GET SSI AUTH DATA]");
      return;
   }

   if (PQnfields(res) != USSI_TBL_FIELDS) 
   {
      LOG_SYS(0, ("Corrypted ssi table structure in db: \"%s\"\n", lp_db_users()));
      exit(EXIT_ERROR_DB_STRUCTURE);
   }
   
   if (PQntuples(res) > 0) 
   {
       arpack.clearPacket();
       arpack.setup_aim();
       reply_pack2.clearPacket();
       reply_pack2.setup_aim();

       arpack.from_ip      = to_user->ip;
       arpack.from_port    = 0;
       arpack.sock_hdl     = to_user->sock_hdl;
       arpack.sock_rnd     = to_user->sock_rnd;
       arpack.sock_evt     = SOCK_DATA;
       arpack.sock_type    = SAIM;
       arpack.flap_channel = 2;
       
       /* start transaction packet */
       arpack  << (unsigned short)SN_TYP_SSI
               << (unsigned short)SN_SSI_EDITxBEGIN
	       << (unsigned short)0x0000
	       << (unsigned  long)to_user->servseq2++;
       
       tcp_writeback_packet(arpack);
       arpack.clearPacket();
       arpack.setup_aim();
       
       strncpy(iname, PQgetvalue(res, 0, 9), sizeof(iname)-1);
       strncpy(tempst, PQgetvalue(res, 0, 11), sizeof(tempst)-1);
       strncpy(nickname, PQgetvalue(res, 0, 10), sizeof(nickname)-1);	       
       gid     = atol(PQgetvalue(res, 0, 2));
       iid     = atol(PQgetvalue(res, 0, 3));
       auth    = 0;
       
       PQclear(res);

       /* Add nickname as additional data */
       if (strlen(nickname) > 0)
       {
          reply_pack2 << (unsigned short)SSI_TLV_NICKNAME
	              << (unsigned short)strlen(nickname)
		      << nickname;
       }
	       
       /* Add authorization flag as additional data */
       if (auth != 0)
       {
          reply_pack2 << (unsigned short)SSI_TLV_AUTH
	              << (unsigned short)0x0000;
       }

       /* Additional data should be decoded and appended here */
       tlv_chain.decode(tempst);
       tlv_chain.addToPacket(reply_pack2);
       tlv_chain.free();
	       
       arpack  << (unsigned short)SN_TYP_SSI
               << (unsigned short)SN_SSI_ITEMxUPDATE
	       << (unsigned short)0x8000
	       << (unsigned  long)to_user->servseq2++
	       
	       << (unsigned short)0x0006
	       << (unsigned short)0x0001
	       << (unsigned short)0x0002
	       << (unsigned short)0x0004
       
               << (unsigned short)strlen(iname)
               << iname
   	       << (unsigned short)gid
	       << (unsigned short)iid
	       << (unsigned short)type
	       << (unsigned short)reply_pack2.sizeVal;
			   
       memcpy(arpack.nextData, reply_pack2.buff, reply_pack2.size());
       arpack.sizeVal += reply_pack2.size();
       tcp_writeback_packet(arpack);
       arpack.clearPacket();
       arpack.setup_aim();       
       
       /* end of transaction packet */
       arpack  << (unsigned short)SN_TYP_SSI
               << (unsigned short)SN_SSI_EDITxEND
	       << (unsigned short)0x0000
	       << (unsigned  long)to_user->servseq2++;
       
       tcp_writeback_packet(arpack);
    }
}


/**************************************************************************/
/* Send auth granted ssi packet						  */
/**************************************************************************/
void ssi_send_auth_granted(unsigned long from_uin, struct online_user *to_user)
{
   arpack.clearPacket();
   arpack.setup_aim();
   char buin[32];

   snprintf(buin, 31, "%lu", from_uin);

   arpack << (unsigned short)SN_TYP_SSI
          << (unsigned short)SN_SSI_AUTHxREPLY
          << (unsigned short)0x8000
          << (unsigned  long)to_user->servseq2++
	  
	  << (unsigned short)0x0006
	  << (unsigned short)0x0001
	  << (unsigned short)0x0002
	  << (unsigned short)0x0004
	  
	  << (char)strlen(buin)
          << buin
	  << (char)0x01
	  << (unsigned short)0x0000
	  << (unsigned short)0x0000;
       
   arpack.from_ip      = to_user->ip;
   arpack.from_port    = 0;
   arpack.sock_hdl     = to_user->sock_hdl;
   arpack.sock_rnd     = to_user->sock_rnd;
   arpack.sock_evt     = SOCK_DATA;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 2;

   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* Send SSI 'You were added' notification				  */
/**************************************************************************/
void ssi_send_you_added(unsigned long from_uin, struct online_user *to_user)
{
   arpack.clearPacket();
   arpack.setup_aim();
   char buin[32];

   snprintf(buin, 31, "%lu", from_uin);

   arpack << (unsigned short)SN_TYP_SSI
          << (unsigned short)SN_SSI_YOUxWERExADDED
          << (unsigned short)0x8000
          << (unsigned  long)to_user->servseq2++
	  
	  << (unsigned short)0x0006
	  << (unsigned short)0x0001
	  << (unsigned short)0x0002
	  << (unsigned short)0x0004
	  
	  << (char)strlen(buin)
          << buin;
       
   arpack.from_ip      = to_user->ip;
   arpack.from_port    = 0;
   arpack.sock_hdl     = to_user->sock_hdl;
   arpack.sock_rnd     = to_user->sock_rnd;
   arpack.sock_evt     = SOCK_DATA;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 2;

   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* Send SSI authorization request 					  */
/**************************************************************************/
void ssi_send_auth_req(unsigned long from_uin, struct online_user *to_user, 
                       char *message)
{
   arpack.clearPacket();
   arpack.setup_aim();
   char buin[32];

   snprintf(buin, 31, "%lu", from_uin);

   arpack << (unsigned short)SN_TYP_SSI
          << (unsigned short)SN_SSI_AUTHxREQ
          << (unsigned short)0x8000
          << (unsigned  long)to_user->servseq2++
	  
	  << (unsigned short)0x0006
	  << (unsigned short)0x0001
	  << (unsigned short)0x0002
	  << (unsigned short)0x0004
	  
	  << (char)strlen(buin)
          << buin
          << (unsigned short)strlen(message)
          << message
	  << (unsigned short)0x0000;
       
   arpack.from_ip      = to_user->ip;
   arpack.from_port    = 0;
   arpack.sock_hdl     = to_user->sock_hdl;
   arpack.sock_rnd     = to_user->sock_rnd;
   arpack.sock_evt     = SOCK_DATA;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 2;

   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* Send SSI authorization request 					  */
/**************************************************************************/
void ssi_send_auth_denied(unsigned long from_uin, struct online_user *to_user, 
                          char *message)
{
   arpack.clearPacket();
   arpack.setup_aim();
   char buin[32];

   snprintf(buin, 31, "%lu", from_uin);

   arpack << (unsigned short)SN_TYP_SSI
          << (unsigned short)SN_SSI_AUTHxREPLY
          << (unsigned short)0x8000
          << (unsigned  long)to_user->servseq2++
	  
	  << (unsigned short)0x0006
	  << (unsigned short)0x0001
	  << (unsigned short)0x0002
	  << (unsigned short)0x0004
	  
	  << (char)strlen(buin)
          << buin
	  << (char)0x00 /* auth denied flag */
          << (unsigned short)strlen(message)
          << message
	  << (unsigned short)0x0000;
       
   arpack.from_ip      = to_user->ip;
   arpack.from_port    = 0;
   arpack.sock_hdl     = to_user->sock_hdl;
   arpack.sock_rnd     = to_user->sock_rnd;
   arpack.sock_evt     = SOCK_DATA;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 2;

   tcp_writeback_packet(arpack);
}


