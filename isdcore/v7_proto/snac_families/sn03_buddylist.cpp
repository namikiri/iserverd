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

/**************************************************************************/
/* Buddy list services SNAC family packets handler			  */
/**************************************************************************/
void process_snac_buddylist(struct snac_header &snac, Packet &pack)
{
   switch (snac.subfamily)
   {
      case SN_BLM_RIGHTSxREQUEST:   process_blm_req_rights(snac, pack);  break;
      case SN_BLM_ADDxCONTACT:	    process_blm_add_contact(snac, pack); break;
      case SN_BLM_DELxCONTACT:      process_blm_del_contact(snac, pack); break;
      
      default: DEBUG(10, ("Unknown location SNAC(0x3, %04X)\n", snac.subfamily));
   }

}


/**************************************************************************/
/* Client wants to know blm limits					  */
/**************************************************************************/
void process_blm_req_rights(struct snac_header &snac, Packet &pack)
{
   struct online_user *user;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip)));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   /* Prepare reply packet */
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();

   arpack << (unsigned short)SN_TYP_BUDDYLIST
          << (unsigned short)SN_BLM_RIGHTSxRESPONSE
	  << (unsigned short)0x0000
	  << (unsigned  long)snac.id

          << (unsigned short)0x0001
          << (unsigned short)sizeof(unsigned short)
	  << (unsigned short)lp_v7_max_contact_size()
	  << (unsigned short)0x0002
	  << (unsigned short)sizeof(unsigned short)
	  << (unsigned short)lp_v7_max_watchers_size()
	  << (unsigned short)0x0003
	  << (unsigned short)sizeof(unsigned short)
	  << (unsigned short)0x0200; /* wtf ? */
   
   arpack.flap_channel = 0x02;
   arpack.sock_evt     = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* Server-initiated user_offline packet 				  */
/**************************************************************************/
void v7_send_user_offline(struct online_user &to_user, unsigned long uin)
{
   char buin[32];

   arpack.clearPacket();
   arpack.setup_aim();
   
   snprintf(buin, 31, "%lu", uin);

   arpack << (unsigned short)SN_TYP_BUDDYLIST
          << (unsigned short)SN_BLM_OFFGOINGxBUDDY
	  << (unsigned short)0x0000
	  << (unsigned  long)to_user.servseq2++ 
	  
	  << (char)strlen(buin)
	  << buin
	  
	  << (unsigned long)0x00000001   /* wtf ? */
	  << (unsigned short)0x0001
	  << (unsigned short)sizeof(unsigned short)
	  << (unsigned short)0x0000;

   arpack.from_ip      = to_user.ip;
   arpack.from_port    = 0;
   arpack.sock_hdl     = to_user.sock_hdl;
   arpack.sock_rnd     = to_user.sock_rnd;
   arpack.sock_evt     = SOCK_DATA;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 2;
   
   DEBUG(450, ("Online: Sending alert about %lu to user %lu\n", 
                uin, to_user.uin));

   /* send packet to client */
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* Client wants to add new record to contact				  */
/**************************************************************************/
void process_blm_add_contact(struct snac_header &snac, Packet &pack)
{
   struct online_user *user, sauser;
   unsigned short contacts_num = 0;
   unsigned long  rid = 0;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip)));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   /* get the data from packet */
   while (pack.nextData < (pack.buff + pack.sizeVal))
   {
      contacts[contacts_num] = read_buin(pack);
      if (contacts[contacts_num] != 0) contacts_num++;
      
      /* check for single packet contacts limit */
      if (contacts_num > MAX_CONTACTS) break;
   }

   rid = lrandom_num();

   /* insert all extracted contacts into database */
   db_contact_insert(user->uin, contacts_num, contacts, NORMAL_CONTACT, rid);
   DEBUG(100, ("Client have sent packet with %d normal contacts (rid=%lu)...\n", 
             contacts_num, rid));

   user->cloaded = 1; /* contact list was loaded */
   
   /* We should send presense only if user was activated  */
   if (!user->active) return;
   
   /* Here we should check if we have only one contact to */
   /* decrease server load on user added single contacts  */
   if (contacts_num == 1)
   {
      DEBUG(100, ("Optimized single contact search (%lu)...\n", contacts[0]));
   
      if ((db_add_online_lookup(user->uin, contacts[0], sauser) == 0) &&
          (sauser.active == 1))
      {
         v7_send_user_online(*user, sauser);
      }
      
      return;
   }
   
   /* send presense only for just added contacts */
   user_send_presense_part(user, rid);
}


/**************************************************************************/
/* Client wants to del record from contact				  */
/**************************************************************************/
void process_blm_del_contact(struct snac_header &snac, Packet &pack)
{
   struct online_user *user;
   unsigned short contact_num = 0;
   unsigned long uin;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip)));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   /* get the data from packet */
   while (pack.nextData < (pack.buff + pack.sizeVal))
   {
      uin = read_buin(pack);
      db_contact_delete(user->uin, uin, NORMAL_CONTACT);
      contact_num++;
   }
   
   DEBUG(10, ("Client want to delete %d normal contacts...\n", contact_num));
}


/**************************************************************************/
/* Server-initiated user_online packet	 				  */
/**************************************************************************/
void v7_send_user_online(struct online_user &to_user, struct online_user &user)
{
   char buin[32];
   unsigned short tlvs_num;
   unsigned long uptime;
   unsigned long dc_bcook;
   
   arpack.clearPacket();
   arpack.setup_aim();
   
   snprintf(buin, 31, "%lu", user.uin);
   uptime = timeToLong(time(NULL)) - user.uptime;

   arpack << (unsigned short)SN_TYP_BUDDYLIST
          << (unsigned short)SN_BLM_ONCOMINGxBUDDY
	  << (unsigned short)0x0000
	  << (unsigned  long)to_user.servseq2++ 
	  
	  << (char)strlen(buin)
	  << buin;
	  
   reply_pack.clearPacket();
   reply_pack.setup_aim();
   
   tlvs_num = 4;
   reply_pack << (unsigned short)0x0001 /* user class */
	      << (unsigned short)(sizeof(unsigned short))
	      << (unsigned short)user.uclass;
	  
   dc_bcook = user.dc_cookie + to_user.dc_cookie;

   if ((user.protocol == V3_PROTO) && (!lp_v7_direct_v3_connect()) ||
      ((user.protocol == V5_PROTO) && (!lp_v7_direct_v5_connect())))
   {
      /* disable dc with v3 clients */
      reply_pack << (unsigned short)0x000c /* dc information */
  	         << (unsigned short)0x0025
	         << (unsigned  long)0x00000000
  	         << (unsigned  long)0x00000000
	         << (char)0x00
	         << (unsigned short)user.tcpver
	         << (unsigned  long)dc_bcook
  	         << (unsigned  long)user.web_port
	         << (unsigned  long)user.cli_futures
	         << (unsigned  long)user.info_utime
	         << (unsigned  long)user.more_utime
	         << (unsigned  long)user.stat_utime
	         << (unsigned short)0x0000;
   }
   else
   {
      reply_pack << (unsigned short)0x000c /* dc information */
  	         << (unsigned short)0x0025;

      if (user.dc_perms != 2)
      {
         reply_pack << (unsigned  long)ipToIcq2(user.int_ip)
  	            << (unsigned  long)user.tcp_port
	            << (char)user.dc_type
	            << (unsigned short)user.tcpver
	            << (unsigned  long)dc_bcook;
      }
      else
      {
         reply_pack << (unsigned  long)0x00000000
		    << (unsigned  long)0x00000000
		    << (char)user.dc_type
		    << (unsigned short)user.tcpver
		    << (unsigned  long)0x00000000;
      }

      reply_pack << (unsigned  long)user.web_port
	         << (unsigned  long)user.cli_futures		 
	         << (unsigned  long)user.info_utime
	         << (unsigned  long)user.more_utime
	         << (unsigned  long)user.stat_utime
	         << (unsigned short)0x0000;
   }
	  
   reply_pack << (unsigned short)0x000a /* real ip address */
	      << (unsigned short)sizeof(user.ip);
	      
   if (user.dc_perms != 2)
   {   
      reply_pack << (unsigned  long)ipToIcq2(user.ip);
   }
   else
   {
      reply_pack << (unsigned  long)0x00;
   }
    
   /* ICQLite doesn't accept estat = 0x0000 :( So we change it to 1- webaware */
   if (user.estat == 0)	user.estat = 0x0001;
  
   reply_pack << (unsigned short)0x0006 /* user ext status */
	      << (unsigned short)(sizeof(user.estat)+sizeof(user.status))
	      << (unsigned short)user.estat
	      << (unsigned short)user.status;
	  
   /* let's dump user CLSIDs to packet */
   if (user.caps_num)
   {
      tlvs_num += 1;
      reply_pack << (unsigned short)0x000D
	         << (unsigned short)(user.caps_num*16);

      for (int i=0;i<user.caps_num;i++)
      {
         for (int j=0;j<16;j++)
	 {
	    reply_pack << (char)user.caps[i][j];
	 }
      }	      
   }

   tlvs_num += 2;
   reply_pack << (unsigned short)0x000f /* user idle time TLV value */
	      << (unsigned short)(sizeof(unsigned long));
 
   if (user.idle_perms)
   {
      reply_pack << (unsigned  long)(time(NULL) - user.idle_time);
   }
   else
   {
      reply_pack << (unsigned  long)0x00000000;
   }

   reply_pack << (unsigned short)0x0003 /* account create time */
	      << (unsigned short)(sizeof(user.crtime))
	      << (unsigned  long)user.crtime;

   /* copy TLV chain from reply_pack to arpack */
   arpack << (unsigned short)0x0000
	  << (unsigned short)tlvs_num;
   
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   arpack.from_ip      = to_user.ip;
   arpack.from_port    = 0;
   arpack.sock_hdl     = to_user.sock_hdl;
   arpack.sock_rnd     = to_user.sock_rnd;
   arpack.sock_evt     = SOCK_DATA;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 2;
   
   DEBUG(450, ("Online: Sending online alert about %lu to user %lu\n", 
                user.uin, to_user.uin));

   /* send packet to client */
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* Server-initiated user_new_status packet				  */
/**************************************************************************/
void v7_send_user_status(struct online_user &to_user, struct online_user &user)
{
   char buin[32];
   unsigned short tlvs_num;
   unsigned long uptime;
   unsigned long dc_bcook;
   
   arpack.clearPacket();
   arpack.setup_aim();
   
   snprintf(buin, 31, "%lu", user.uin);
   uptime = timeToLong(time(NULL)) - user.uptime;

   arpack << (unsigned short)SN_TYP_BUDDYLIST
          << (unsigned short)SN_BLM_ONCOMINGxBUDDY
	  << (unsigned short)0x0000
	  << (unsigned  long)to_user.servseq2++ 
	  
	  << (char)strlen(buin)
	  << buin;
	  
   reply_pack.clearPacket();
   reply_pack.setup_aim();
   
   tlvs_num = 7;
   reply_pack << (unsigned short)0x0001 /* user class */
	      << (unsigned short)(sizeof(unsigned short))
	      << (unsigned short)user.uclass;

   /* ICQLite don't like estat=0 :( So I change it to 1- webaware */
   if (user.estat == 0)	user.estat = 0x0001;
   dc_bcook = user.dc_cookie + to_user.dc_cookie;
   
   if ((user.protocol == V3_PROTO) && (!lp_v7_direct_v3_connect()) ||
      ((user.protocol == V5_PROTO) && (!lp_v7_direct_v5_connect())))
   {
      /* disable dc with v3/v5 clients */
      reply_pack << (unsigned short)0x000c /* dc information */
  	         << (unsigned short)0x0025
	         << (unsigned  long)0x00000000
  	         << (unsigned  long)0x00000000
	         << (char)0x00
	         << (unsigned short)user.tcpver
	         << (unsigned  long)dc_bcook
  	         << (unsigned  long)user.web_port
	         << (unsigned  long)user.cli_futures
	         << (unsigned  long)user.info_utime
	         << (unsigned  long)user.more_utime
	         << (unsigned  long)user.stat_utime
	         << (unsigned short)0x0000;
   }
   else
   {
      reply_pack << (unsigned short)0x000c /* dc information */
	         << (unsigned short)0x0025
	         << (unsigned  long)ipToIcq2(user.int_ip)
  	         << (unsigned  long)user.tcp_port
	         << (char)user.dc_type
	         << (unsigned short)user.tcpver
	         << (unsigned  long)dc_bcook
  	         << (unsigned  long)user.web_port
	         << (unsigned  long)user.cli_futures
	         << (unsigned  long)user.info_utime
	         << (unsigned  long)user.more_utime
	         << (unsigned  long)user.stat_utime
	         << (unsigned short)0x0000;
   }
   
   reply_pack << (unsigned short)0x000a /* real ip address */
	      << (unsigned short)sizeof(user.ip)
	      << (unsigned  long)ipToIcq2(user.ip)
	      
	      << (unsigned short)0x0004 /* unknown field */
	      << (unsigned short)sizeof(unsigned short)
	      << (unsigned short)0x0001
	  
	      << (unsigned short)0x0006 /* user ext status */
	      << (unsigned short)(sizeof(user.estat)+sizeof(user.status))
	      << (unsigned short)user.estat
	      << (unsigned short)user.status
	      
              << (unsigned short)0x000f /* user idle_time TLV value */
	      << (unsigned short)(sizeof(unsigned long));

   if (user.idle_perms)
   {
      reply_pack << (unsigned  long)(time(NULL) - user.idle_time);
   }
   else
   {
      reply_pack << (unsigned  long)0x00000000;
   }
 
   reply_pack << (unsigned short)0x0003 /* account create time */
	      << (unsigned short)(sizeof(user.crtime))
	      << (unsigned  long)user.crtime;

   /* copy TLV chain from reply_pack to arpack */
   arpack << (unsigned short)0x0000
	  << (unsigned short)tlvs_num;
   
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   arpack.from_ip      = to_user.ip;
   arpack.from_port    = 0;
   arpack.sock_hdl     = to_user.sock_hdl;
   arpack.sock_rnd     = to_user.sock_rnd;
   arpack.sock_evt     = SOCK_DATA;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 2;
   
   DEBUG(450, ("Online: Sending status alert about %lu to user %lu\n", 
                user.uin, to_user.uin));

   /* send packet to client */
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* This function send presense notifications for user contact list	  */
/**************************************************************************/
void user_send_presense_full(struct online_user *user)
{
   struct online_user *auser;
   int on_cnt1, on_cnt2, i;
   PGresult *res;    
   cstring dbcomm_str;

/* Query define (to avoid multistring literals) */
#define BCST_V7BLMF168 \
 "SELECT uin,ishm FROM online_users \
  JOIN \
  ( \
     SELECT tuin FROM online_contacts \
     WHERE (ouin=%lu) AND (type=%d) \
     EXCEPT \
     ( \
        SELECT ouin FROM online_contacts \
        WHERE (tuin=%lu) AND (type=%d) \
     ) \
     LIMIT %d \
  ) \
  AS TMP on TMP.tuin=uin \
  WHERE (active=1) AND (stat<>%d)"
   
   /* now we'll try to get online users from our contact */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, 
	    BCST_V7BLMF168, user->uin, NORMAL_CONTACT, user->uin, 
	    INVISIBLE_CONTACT, lp_v7_max_contact_size(), ICQ_STATUS_PRIVATE);

   res = PQexec(users_dbconn, dbcomm_str);

   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[V7 CONTACT LOOKUP]");
      close_connection(user);
      return;
   }

   on_cnt1 = PQntuples(res);
   DEBUGADD(150, ("V7 online query returned %d normal users to alert...\n", on_cnt1));
       
   for (i=0;i<on_cnt1;i++) 
   {
      /* find online user in shm */
      auser = shm_iget_user(atoul(PQgetvalue(res, i, 0)), atoul(PQgetvalue(res, i, 1)));
      if (auser == NULL) continue;
      
      /* send alert about auser to user */
      v7_send_user_online(*user, *auser);
   }
       
   PQclear(res);

/* Query define (to avoid multistring literals) */
#define BCST_V7BLMF227 \
 "SELECT uin,ishm FROM online_users \
  JOIN \
  ( \
     SELECT tuin FROM online_contacts \
     WHERE (ouin=%lu) AND (type=%d) \
     INTERSECT \
     ( \
        SELECT ouin FROM online_contacts \
        WHERE tuin=%lu AND type=%d \
     ) \
     LIMIT %d \
  ) \
  AS TMP on TMP.tuin=uin \
  WHERE (active=1) AND (stat=%d)"

   /* now we have to check for invisible users (visible lists) */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, 
	    BCST_V7BLMF227, user->uin, NORMAL_CONTACT, user->uin, 
	    VISIBLE_CONTACT, lp_v7_max_contact_size(), ICQ_STATUS_PRIVATE);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[V7 CONTACT LOOKUP (invisible)]");
      close_connection(user);
      return;
   }

   on_cnt2 = PQntuples(res);
   DEBUGADD(150, ("V7 online query returned %d invisible users to alert...\n", on_cnt2));
       
   for (i=0;i<on_cnt2;i++) 
   {
      /* find online user in shm */
      auser = shm_iget_user(atoul(PQgetvalue(res, i, 0)), atoul(PQgetvalue(res, i, 1)));
      if (auser == NULL) continue;

      /* send alert about auser to user */
      v7_send_user_online(*user, *auser);
      if (((i % 100) == 0) & (i != 0)) results_delay(70);
   }
       
   PQclear(res);
}


/**************************************************************************/
/* This function send presense notifications for user contact list	  */
/**************************************************************************/
void user_send_presense_part(struct online_user *user, unsigned long rid)
{
   struct online_user *auser;
   int on_cnt1, on_cnt2, i;
   PGresult *res;    
   cstring dbcomm_str;

/* Query define (to avoid multistring literals) */
#define BCST_V7BLM168 \
 "SELECT uin,ishm FROM online_users \
  JOIN \
  ( \
     SELECT tuin FROM online_contacts \
     WHERE (ouin=%lu) AND (type=%d) AND (rid=%lu) \
     EXCEPT \
     ( \
        SELECT ouin FROM online_contacts \
        WHERE (tuin=%lu) AND (type=%d) \
     ) \
     LIMIT %d \
  ) \
  AS TMP on TMP.tuin=uin \
  WHERE (active=1) AND (stat<>%d)"
   
   /* now we'll try to get online users from our contact */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, 
	    BCST_V7BLM168, user->uin, NORMAL_CONTACT, rid, user->uin, 
	    INVISIBLE_CONTACT, lp_v7_max_contact_size(), ICQ_STATUS_PRIVATE);

   res = PQexec(users_dbconn, dbcomm_str);

   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[V7 CONTACT LOOKUP]");
      close_connection(user);
      return;
   }

   on_cnt1 = PQntuples(res);
   DEBUGADD(150, ("V7 online query returned %d normal users to alert...\n", on_cnt1));
       
   for (i=0;i<on_cnt1;i++) 
   {
      /* find online user in shm */
      auser = shm_iget_user(atoul(PQgetvalue(res, i, 0)), atoul(PQgetvalue(res, i, 1)));
      if (auser == NULL) continue;
      
      /* send alert about auser to user */
      v7_send_user_online(*user, *auser);
   }
       
   PQclear(res);

/* Query define (to avoid multistring literals) */
#define BCST_V7BLM227 \
 "SELECT uin,ishm FROM online_users \
  JOIN \
  ( \
     SELECT tuin FROM online_contacts \
     WHERE (ouin=%lu) AND (type=%d) AND (rid=%lu) \
     INTERSECT \
     ( \
        SELECT ouin FROM online_contacts \
        WHERE tuin=%lu AND type=%d \
     ) \
     LIMIT %d \
  ) \
  AS TMP on TMP.tuin=uin \
  WHERE (active=1) AND (stat=%d)"

   /* now we have to check for invisible users (visible lists) */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, 
	    BCST_V7BLM227, user->uin, NORMAL_CONTACT, rid, user->uin, 
	    VISIBLE_CONTACT, lp_v7_max_contact_size(), ICQ_STATUS_PRIVATE);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[V7 CONTACT LOOKUP (invisible)]");
      close_connection(user);
      return;
   }

   on_cnt2 = PQntuples(res);
   DEBUGADD(150, ("V7 online query returned %d invisible users to alert...\n", on_cnt2));
       
   for (i=0;i<on_cnt2;i++) 
   {
      /* find online user in shm */
      auser = shm_iget_user(atoul(PQgetvalue(res, i, 0)), atoul(PQgetvalue(res, i, 1)));
      if (auser == NULL) continue;

      /* send alert about auser to user */
      v7_send_user_online(*user, *auser);
   }
       
   PQclear(res);
}

