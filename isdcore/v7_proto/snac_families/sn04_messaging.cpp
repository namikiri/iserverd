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
/* This module contain v7 messages creating/processing functions          */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* Messaging services SNAC family packets handler			  */
/**************************************************************************/
void process_snac_messaging(struct snac_header &snac, Packet &pack)
{
   switch (snac.subfamily)
   {
      case SN_MSG_PARAMxREQUEST:  process_msg_req_rights(snac, pack); break;
      case SN_MSG_ADDxICBMxPARAM: process_msg_set_rights(snac, pack); break;
      case SN_MSG_SENDxMESSAGE:   process_msg_send(snac, pack); break;
      case SN_MSG_MESSxACK:	  process_msg_ack(snac, pack);  break;
      case SN_MSG_MTN:		  process_msg_mtn(snac, pack); break;
      
      default: DEBUG(10, ("Unknown messaging SNAC(0x4, %04X)\n", snac.subfamily));
   }
}

/**************************************************************************/
/* Client just sent its parameters 					  */
/**************************************************************************/
void process_msg_set_rights(struct snac_header &snac, Packet &pack)
{
   struct online_user *user;
   unsigned short channel, uchannel;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s:%d sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip), pack.from_port));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   pack >> channel;
   uchannel = channel;
   
   if (channel <= MAX_ICBM_CHANNELS)
   {
      if (channel > 0) channel--;
      pack >> user->mopt[channel].icbm_flags
 	   >> user->mopt[channel].max_msglen
	   >> user->mopt[channel].max_sevil
	   >> user->mopt[channel].max_revil
	   >> user->mopt[channel].min_interval;
      
      if (uchannel == 0) /* user sent limits for all channels (?) */
      {
         DEBUG(50, ("MOpt for all channels from %lu\n", user->uin));
         for (int i=1; i<MAX_ICBM_CHANNELS; i++) user->mopt[i] = user->mopt[channel];
      }
            
      DEBUG(50, ("MOpt(%lu)-%d: flg=%08X,len=%d,sev=%d,rev=%d,int=%d\n", 
                  user->uin,uchannel,(unsigned int)(user->mopt[channel].icbm_flags), 
		  user->mopt[channel].max_msglen, user->mopt[channel].max_sevil, 
		  user->mopt[channel].max_revil, user->mopt[channel].min_interval));
   }
   else
   {
         /* sorry - no room for this channel */
         send_snac_error(snac.family, ERR_LIST_OVERFLOW, snac.id, pack);
   }
}


/**************************************************************************/
/* Client sent Mini Typing Notifocation (MTN) to its buddy		  */
/**************************************************************************/
void process_msg_mtn(struct snac_header &snac, Packet &pack)
{
   struct online_user *user, *to_user;
   unsigned long ljunk;
   unsigned short channel;
   unsigned long to_uin;
   unsigned short mtn_type;
   char buin[32];
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s:%d sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip), pack.from_port));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }
   
   /* extract user parameters from packet */
   pack >> ljunk >> ljunk >> channel;
   to_uin = read_buin(pack);
   if ((channel > MAX_ICBM_CHANNELS) || (channel == 0)) return;

   pack >> mtn_type;
   if (mtn_type > 2) mtn_type = 0;

   DEBUG(150, ("Typing notification: %lu --> %lu, chan=%d, type=%d\n", 
                user->uin, to_uin, channel, mtn_type));

   /* Now we can send typing notification to user */
   if (((to_user = shm_get_user(to_uin)) != NULL) && 
        (to_user->protocol == V7_PROTO) &&
	((to_user->mopt[channel-1].icbm_flags & ICBM_FLG_MTN) != 0))
   {
      arpack.clearPacket();
      arpack.setup_aim();
      
      snprintf(buin, 30, "%lu", user->uin);

      arpack << (unsigned short)SN_TYP_MESSAGING
             << (unsigned short)SN_MSG_MTN
             << (unsigned short)0x0000
	     << (unsigned  long)to_user->servseq2++
	     << (unsigned  long)0x00000000
	     << (unsigned  long)0x00000000
	     << (unsigned short)channel
             << (char)strlen(buin)
  	     << buin
	     << (unsigned short)mtn_type;

      arpack.from_ip      = to_user->ip;
      arpack.from_port    = 0;
      arpack.sock_hdl     = to_user->sock_hdl;
      arpack.sock_rnd     = to_user->sock_rnd;
      arpack.sock_evt     = SOCK_DATA;
      arpack.sock_type    = SAIM;
      arpack.flap_channel = 2;     
       
      tcp_writeback_packet(arpack);
   }
}


/**************************************************************************/
/* Client just sent ack for type-2 message we should deliver it           */
/**************************************************************************/
void process_msg_ack(struct snac_header &snac, Packet &pack)
{
   struct online_user *user, *to_user;
   unsigned short dchunk_size = 0;
   unsigned short mkind = 0;
   unsigned long touin = 0;
   char msg_cookie[8];
   char buin[32];
   int i = 0;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s:%d sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip), pack.from_port));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   /* extract cookie, message kind and target uin */
   for (i=0; i<8; i++) pack >> msg_cookie[i];
   pack >> mkind;
   touin = read_buin(pack);

   if ((to_user = shm_get_user(touin)) == NULL)
   {
      /* target user just gone offline so we should notify sender */
      send_snac_error(snac.family, ERR_RECIPIENT_OFFLINE, snac.id, user);
      return;
   }
   
   snprintf(buin, 31, "%lu", user->uin);
   dchunk_size = pack.sizeVal - (pack.nextData - pack.buff);

   /* Reassemble ack to reply_packet */
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();

   arpack << (unsigned short)SN_TYP_MESSAGING
          << (unsigned short)SN_MSG_MESSxACK
	  << (unsigned short)0x0000
	  << (unsigned  long)to_user->servseq2++;

   for (i=0; i<8; i++) arpack << (char)msg_cookie[i];

   arpack << (unsigned short)mkind
          << (char)strlen(buin)
	  << buin;   

   memcpy(arpack.nextData, pack.nextData, dchunk_size);
   arpack.sizeVal += dchunk_size;
	  
   arpack.from_ip      = to_user->ip;
   arpack.from_port    = 0;
   arpack.sock_hdl     = to_user->sock_hdl;
   arpack.sock_rnd     = to_user->sock_rnd;
   arpack.sock_evt     = SOCK_DATA;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 2;
   
   DEBUG(100, ("Ack: Type-2 ack (%lu-->%lu)\n", user->uin, to_user->uin));

   /* send packet to client */
   tcp_writeback_packet(arpack);

}


/**************************************************************************/
/* Client wants its current messages limits				  */
/**************************************************************************/
void process_msg_req_rights(struct snac_header &snac, Packet &pack)
{
   struct online_user *user;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s:%d sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip), pack.from_port));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   /* Prepare reply packet */
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();

   /* Currently this reply is only for channel2 limits */
   arpack << (unsigned short)SN_TYP_MESSAGING
          << (unsigned short)SN_MSG_PARAMxRESPONSE
	  << (unsigned short)0x0000
	  << (unsigned  long)snac.id
	  << (unsigned short)0x0002
	  << (unsigned  long)user->mopt[MCH2].icbm_flags
	  << (unsigned short)user->mopt[MCH2].max_msglen
	  << (unsigned short)user->mopt[MCH2].max_sevil
	  << (unsigned short)user->mopt[MCH2].max_revil
	  << (unsigned short)user->mopt[MCH2].min_interval
	  << (unsigned short)1000; /* ??? */

   arpack.flap_channel = 0x02;
   arpack.sock_evt     = SOCK_DATA;
   tcp_writeback_packet(arpack);

}


/**************************************************************************/
/* Client wants to send message thru server				  */
/**************************************************************************/
void process_msg_send(struct snac_header &snac, Packet &pack)
{
   struct online_user *user;
   struct msg_header   msg_hdr;
   
   bzero((void *)&msg_hdr, sizeof(msg_hdr));
   
   /* check if user have rights to send this message */
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s:%d sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip), pack.from_port));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   for (int i=0;i<8;i++) pack >> msg_hdr.msg_cookie[i];
   pack >> msg_hdr.mkind;
   msg_hdr.touin     = read_buin(pack);
   msg_hdr.fromuin   = user->uin;
   msg_hdr.fromindex = user->shm_index;
   msg_hdr.msgsize   = pack.size();
   
   /* OK.. we have several kinds of messages */
   switch (msg_hdr.mkind)
   {
      case MSG_KIND_TXT:
              /* simple plain text message (just text) */
              process_txt_message(pack, snac, user, msg_hdr);
   	      break;
	   
      case MSG_KIND_ADV:
              /* advanced message (rtf, file, etc... ) */
              process_adv_message(pack, snac, user, msg_hdr);
	      break;
   
      case MSG_KIND_SYS:
              /* system message ("you were added", "auth request", etc... */
              process_sys_message(pack, snac, user, msg_hdr);
	      break;
	   
      default: 
              /* just drop this message and complain to log */
              DEBUG(10, ("Unk msg_channel (%lu->%lu): ch=%d...\n", 
	                 user->uin, msg_hdr.touin, msg_hdr.mkind));

	      send_snac_error(snac.family, ERR_SNAC_DATA_INVALID, snac.id, user);

	      break;
	     
   }
}


/**************************************************************************/
/* Plain text message from client 					  */
/**************************************************************************/
void process_txt_message(Packet &pack, struct snac_header &snac, 
                         struct online_user *user,  struct msg_header &msg_hdr)
{
   class tlv_chain_c tlv_chain, tlv_chain2;
   class tlv_c *tlv;
   char unk_flag = 0;
   struct online_user *to_user;
   unsigned short mess_len = 0;
   int i;
   
   tlv_chain.read(pack);
   tlv_chain.network_order();
   
   /* well, this chain should contain TLV(0x02), and TLV(0x06) */
   if ((tlv = tlv_chain.get(0x02)) == NULL)
   {
      DEBUG(100, ("ERROR: Invalid txt_msg snac from %lu (%s) - (tlv(0x2))\n", 
                   user->uin, inet_ntoa(pack.from_ip)));

      /* notices to sender and receiver */
      send_missed_message(user, msg_hdr, 1, MISSED_MSG_INVALID);
      send_snac_error(snac.family, ERR_SNAC_DATA_INVALID, snac.id, user);
      return;
   }

   tlv_chain2.read(*tlv);
   tlv_chain2.network_order();
   
   /* OK, now we should have TLV chain with 0x0501/0x0101 TLVs */
   if ((tlv = tlv_chain2.get(0x0501)) != NULL)
   {
      /* i'll find out purpose of this flag later */
      *tlv >> unk_flag;
   }
   
   if ((tlv = tlv_chain2.get(0x0101)) == NULL)
   {
      DEBUG(100, ("ERROR: Can't find TLV 0x0101 with message (txt_message)\n"));

      send_missed_message(user, msg_hdr, 1, MISSED_MSG_INVALID);      
      send_snac_error(snac.family, ERR_SNAC_DATA_INVALID, snac.id, user);
      return;
   }
   
   *tlv >> msg_hdr.mcharset;
   *tlv >> msg_hdr.mcsubset;
   mess_len = tlv->size - 4;
   msg_hdr.fromuin = user->uin;
   msg_hdr.msglen  = mess_len;
   
   if (mess_len > (MAX_PACKET_SIZE-256))
   {
      DEBUG(10, ("ERROR: too big msg from %lu. Msglen = %d.\n", user->uin, mess_len));

      send_missed_message(user, msg_hdr, 1, MISSED_MSG_INVALID);
      send_snac_error(snac.family, ERR_SNAC_DATA_INVALID, snac.id, user);
      return;
   }
   
   /* read string into buffer */
   for(i=0; i<mess_len; i++) *tlv >> msg_buff[i]; 
   msg_buff[i] = 0;
   
   DEBUG(100, ("Plaintext msg (unk_flag=%d) to user %lu from %lu.\n", 
                unk_flag, msg_hdr.touin, msg_hdr.fromuin));

   msg_hdr.mtype = 0x0001; /* normal message       */
   msg_hdr.mkind = 0x0001; /* plain text message   */

   /* There is also some data after message string, but we should          */
   /* take care about it only if target uin is online and it is V7+ client */
 
   if (((to_user = shm_get_user(msg_hdr.touin)) != NULL) &&
       (to_user->active == 1))
   {
      /* check for message size & user ch1 permissions */
      if ((to_user->mopt[MCH1].max_msglen < msg_hdr.msglen) ||
         ((to_user->mopt[MCH1].icbm_flags & ICBM_FLG_BASE) == 0))
      {
         DEBUG(10, ("Error: Ch1 msg (%lu-->%lu) too big or channel closed\n", 
	            user->uin, to_user->uin));

         send_missed_message(to_user, user, msg_hdr, 1, MISSED_MSG_TOO_LARGE);
         send_snac_error(snac.family, ERR_CLI_NON_SUPPORTED, snac.id, user);
         return;
      }

      /* check for user sender evil level limit */
      if (to_user->mopt[MCH1].max_sevil < user->warn_level)
      {
         send_snac_error(snac.family, ERR_SENDER_TOO_EVIL, snac.id, user);
         return;
      }
	 
      /* check for user receiver evil level limit */
      if (user->mopt[MCH1].max_revil < to_user->warn_level)
      {
         send_snac_error(snac.family, ERR_RECIPIENT_TOO_EVIL, snac.id, user);
         return;
      }

      /* check for user message rate limit */
      if (to_user->mopt[MCH1].min_interval > abs(static_cast<int>(time(NULL) - to_user->mopt[MCH1].last_msg)))
      {
         DEBUG(10, ("Error: Ch1 msg (%lu-->%lu) - rate limit hit\n", 
	            user->uin, to_user->uin));
		    
         send_snac_error(snac.family, ERR_CLI_RATE_LIMIT, snac.id, user);
         return;
      }

       /* TLV 0x03 mean that user want server ack */
      if (tlv_chain.get(0x3) != NULL)
      { 
         send_type2_ack(user, snac, msg_hdr, MSG_KIND_TXT);
      }

      if (to_user->protocol == V7_PROTO)
      {
         /* this is v7+ client so we should send type-1 message */
	 v7_send_user_message_x1(msg_hdr, *to_user, msg_buff);
      }
      else
      {
         /* This is old-style client so we should send simply message */
         send_online_message(msg_hdr, *to_user, msg_buff);
      }
   } 
   else 
   {
      /* check message size (maxsize = 450) */
      if (msg_hdr.msglen > 450)
      {
         DEBUG(10, ("Big msg to offline from %lu. Len>450 (%d)\n", 
	             user->uin, msg_hdr.msglen));
         return;
      }
	
       /* TLV 0x03 mean that user want server ack */
      if (tlv_chain.get(0x3) != NULL)
      { 
         send_type2_ack(user, snac, msg_hdr, MSG_KIND_TXT);
      }

      DEBUG(150, ("OFMessage (%d) from user %lu to user %lu\n", msg_hdr.mtype, 
                  msg_hdr.fromuin, msg_hdr.touin));

      db_add_message(msg_hdr, msg_buff);
   }
}


/**************************************************************************/
/* Advanced text message from client					  */
/**************************************************************************/
void process_adv_message(Packet &pack, struct snac_header &snac, 
                         struct online_user *user,  struct msg_header &msg_hdr)
{
   class tlv_chain_c tlv_chain, tlv_chain2;
   class tlv_c *tlv;
   struct online_user *to_user;
   unsigned short errcode = 0;
   char required_cap[16];
   BOOL ack_flag = False;
   char cc;
   int i;
   
   tlv_chain.read(pack);
   tlv_chain.network_order();
   
   /* well, this chain should contain TLV(0x05), and TLV(0x03) */
   if ((tlv = tlv_chain.get(0x05)) == NULL)
   {
      /* to disable ISee/SIM invisible clients detection */
      send_snac_error(snac.family, ERR_RECIPIENT_OFFLINE, snac.id, user);
      return;
   }

   tlv->network_order();
   *tlv >> msg_hdr.fabt_ack;  /* file abort/ack flag */

   for (i=0; i<8; i++) *tlv >> cc; /* we already have cookie */
   
   /* dump capability CLSID into array */
   for (i=0; i<16; i++) *tlv >> required_cap[i];
   
   tlv_chain2.readRev(*tlv);
   tlv_chain2.network_order();

   if (((to_user = shm_get_user(msg_hdr.touin)) == NULL) ||
       (to_user->active == 0))
   {
      /* User shouldn't send type-2 message to offline client */
      DEBUG(10, ("Can't send type2 (%lu-->%lu) - recipient offline\n", 
                 user->uin, msg_hdr.touin));
		 
      send_snac_error(snac.family, ERR_RECIPIENT_OFFLINE, snac.id, user);
      return;
   }

   if (!user_has_cap(to_user, required_cap))
   {
      /* user shouldn't send type-2 message to client that     */
      /* will not understand it. This is checked by capability */
      DEBUG(10, ("Can't send type2 (%lu-->%lu) - no capability\n", 
                 user->uin, to_user->uin));
		 
      send_snac_error(snac.family, ERR_CLI_NON_SUPPORTED, snac.id, user);
      return;
   }

   /* check for message size */
   if (to_user->mopt[MCH2].max_msglen < msg_hdr.msgsize)
   {
      DEBUG(10, ("Can't send type2 (%lu-->%lu) - too big (%d<%d)\n", 
                 user->uin, to_user->uin, to_user->mopt[MCH2].max_msglen, 
		 msg_hdr.msgsize));
		 
      send_missed_message(to_user, user, msg_hdr, 1, MISSED_MSG_TOO_LARGE);
      send_snac_error(snac.family, ERR_CLI_NON_SUPPORTED, snac.id, user);
      return;
   }

   /* check for user sender evil level limit */
   if (to_user->mopt[MCH2].max_sevil < user->warn_level)
   {
      DEBUG(10, ("Can't send type2 (%lu-->%lu) - sender evil (%d)\n", 
                 user->uin, to_user->uin, user->warn_level));
		 
      send_missed_message(to_user, user, msg_hdr, 1, MISSED_MSG_SENDER_EVIL);
      send_snac_error(snac.family, ERR_SENDER_TOO_EVIL, snac.id, user);
      return;
   }
	 
   /* check for user receiver evil level limit */
   if (user->mopt[MCH2].max_revil < to_user->warn_level)
   {
      DEBUG(10, ("Can't send type2 (%lu-->%lu) - receiver evil (%d)\n", 
                 user->uin, to_user->uin, to_user->warn_level));
      
      send_missed_message(to_user, user, msg_hdr, 1, MISSED_MSG_RECEIVER_EVIL);
      send_snac_error(snac.family, ERR_RECIPIENT_TOO_EVIL, snac.id, user);
      return;
   }

   /* check for user message rate limit */
   if (to_user->mopt[MCH2].min_interval > abs(static_cast<int>(time(NULL) - to_user->mopt[MCH2].last_msg)))
   {
      DEBUG(10, ("Can't send type2 (%lu-->%lu) - receiver rate limit\n", 
                 user->uin, to_user->uin));

      send_snac_error(snac.family, ERR_CLI_RATE_LIMIT, snac.id, user);
      return;
   }

   /* Here we should check if we know message format for    */
   /* this capability. If not we should just copy TLV chain */
   /* into snac(04,07) packet and send it to user           */
   /* ------------------------------------------------------*/

   if (caps_match(required_cap, aim_caps[AIM_CAPS_ICQxEXTxMSG]))
   {
      /* we known this message format - let's check tlv chain */
      /* errcode = msgext_validate_chain(tlv_chain2, msg_hdr, pack); */
   }
   else
   {
      /* unknown message format - we can't validate it */
      DEBUG(100, ("Msg format from %lu client-specific. No validation\n", user->uin));
      errcode = 0;
   }
        
   if (errcode != 0)
   {
      send_missed_message(user, msg_hdr, 1, MISSED_MSG_INVALID);
      send_snac_error(snac.family, errcode, snac.id, user);
      return;
   }

   /* TLV 0x03 mean that user want server ack */
   if (tlv_chain.get(0x3) != NULL)
   { 
      send_type2_ack(user, snac, msg_hdr, MSG_KIND_ADV);
      ack_flag = True;
   }

   /* all is OK so we can send message to client ! */
   v7_send_user_message_x2(msg_hdr, to_user, user, required_cap, 
                           tlv_chain2, ack_flag);
 
}


/**************************************************************************/
/* Ext message validator CLSID://{09461349-4C7F-11D1-8222-444553540000}	  */
/**************************************************************************/
unsigned short msgext_validate_chain(class tlv_chain_c &tlv_chain2, 
                                     struct msg_header &msg_hdr, 
				     Packet &pack)
{
   class tlv_c *tlv, *tlv2711;

   DEBUG(10, ("Validator for {09461349-4C7F-11D1-8222-444553540000} message\n"));

   /* parse abort message modification */
   if (msg_hdr.fabt_ack == 0x0001)
   {
      /* we should have only one tlv - 0x000B */
      if (((tlv = tlv_chain2.get(0x000B)) == NULL) || 
           (tlv_chain2.num() != 1))
      {
         DEBUG(100, ("ERROR: Mailformed file_abort packet from %s (tlv(0x0x000B))\n", 
                   inet_ntoa(pack.from_ip)));

         return(ERR_SNAC_DATA_INVALID);
      }
      else
      {
         return(0);
      }
   }

   /* Parse normal message */   
   if ((tlv2711 = tlv_chain2.get(0x2711)) == NULL)
   {
      DEBUG(100, ("ERROR: Mailformed adv_message packet from %s (tlv(0x2711))\n", 
                  inet_ntoa(pack.from_ip)));

      return(ERR_SNAC_DATA_INVALID);
   }
      
   if (tlv2711->size > MAX_PACKET_SIZE - 256)
   {
      return(ERR_SNAC_DATA_INVALID);
   }      

   return(0);
}


/**************************************************************************/
/* System text message from client					  */
/**************************************************************************/
void process_sys_message(Packet &pack, struct snac_header &snac, 
                         struct online_user *user,  struct msg_header &msg_hdr)
{
   struct online_user *to_user;
   class tlv_chain_c tlv_chain;
   class tlv_c *tlv;
   unsigned short rest_size = 0;

   tlv_chain.read(pack);
   tlv_chain.network_order();
   
   /* well, this chain should contain TLV(0x05), and TLV(0x06) */
   if ((tlv = tlv_chain.get(0x05)) == NULL)
   {
      DEBUG(100, ("ERROR: Mailformed sys_msg snac from %lu (%s) - (tlv(0x5))\n", 
                user->uin, inet_ntoa(pack.from_ip)));

      send_snac_error(snac.family, ERR_SNAC_DATA_INVALID, snac.id, user);
      return;
   }
   
   tlv->intel_order();
   
   /* added_by_uin field. It should be eq user->uin */
   *tlv >> msg_hdr.fromuin;
   
   if (msg_hdr.fromuin != user->uin) 
   {
      DEBUG(100, ("ERROR: Spoofed from_uin field in message from %lu (%s)\n", 
                  user->uin, inet_ntoa(pack.from_ip)));
		 
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, user);
      return;
   }

   /* message type field. Note: mtype high byte is a message flag */
   /* low byte is a type. */
   *tlv >> msg_hdr.mtype;
   
   /* There is also some data after message string, but we should          */
   /* take care about it only if target uin is online and it is V7+ client */
 
   if (((to_user = shm_get_user(msg_hdr.touin)) != NULL) &&
       (to_user->active == 1))
   {
      /* now we should extract message string from tlv */
      if (!v7_extract_string(msg_buff, *tlv, MAX_PACKET_SIZE-256, "sysmsg", *user, pack)) 
      {
         DEBUG(10, ("Error: type-4 msg (%lu-->%lu) len > %d\n", 
	             user->uin, to_user->uin, MAX_PACKET_SIZE-256));
		     
         send_snac_error(snac.family, ERR_SNAC_DATA_INVALID, snac.id, user);
         return;
      }
	
      rest_size = tlv->size - (tlv->nextData - tlv->value);
      msg_hdr.msgsize = strlen(msg_buff);
      
      DEBUG(150, ("ONMessage (%d) from user %lu to user %lu\n", msg_hdr.mtype, 
                  msg_hdr.fromuin, msg_hdr.touin));

      /* check for message size */
      if (to_user->mopt[MCH4].max_msglen < msg_hdr.msgsize)
      {
         DEBUG(10, ("Error: Type4 msg (%lu-->%lu) len=%d > %d\n", 
	            user->uin, to_user->uin, msg_hdr.msgsize, 
		    to_user->mopt[MCH4].max_msglen));
		    
         send_snac_error(snac.family, ERR_CLI_NON_SUPPORTED, snac.id, user);
         return;
      }

      /* check for user sender evil level limit */
      if (to_user->mopt[MCH4].max_sevil < user->warn_level)
      {
         send_snac_error(snac.family, ERR_SENDER_TOO_EVIL, snac.id, user);
         return;
      }
	 
      /* check for user receiver evil level limit */
      if (user->mopt[MCH4].max_revil < to_user->warn_level)
      {
         send_snac_error(snac.family, ERR_RECIPIENT_TOO_EVIL, snac.id, user);
         return;
      }

      /* check for user message rate limit */
      if (to_user->mopt[MCH4].min_interval > abs(static_cast<int>(time(NULL) - to_user->mopt[MCH4].last_msg)))
      {
         send_snac_error(snac.family, ERR_CLI_RATE_LIMIT, snac.id, user);
         return;
      }

      /* check for empty "you were added message" - miranda lasy bug */
      if ((strlen(msg_buff) < 1) &&
          (msg_hdr.mtype == 12))
      {
         DEBUG(100, ("V7 Empty you were added message from %lu\n", msg_hdr.fromuin));

	 struct full_user_info userinfo;
         bzero((void *)&userinfo, sizeof(userinfo));
         if (db_users_lookup_short(msg_hdr.fromuin, userinfo) == 0)
         {
            snprintf(msg_buff, 128, "%s%s%s%s%s%s-", userinfo.nick, CLUE_CHAR,
                     userinfo.first, CLUE_CHAR, userinfo.last, CLUE_CHAR);
         }
         else
         {
            snprintf(msg_buff, 128, "-%s-%s-%s-", CLUE_CHAR, CLUE_CHAR, CLUE_CHAR);
         }

	 msg_hdr.msgsize = strlen(msg_buff);
      }

      /* TLV 0x03 mean that user want server ack */
      if (tlv_chain.get(0x3) != NULL)
      { 
         send_type2_ack(user, snac, msg_hdr, MSG_KIND_SYS);
      }
   
      if ((to_user->protocol == V7_PROTO) &&
	  (rest_size != 0))
      {
         /* this is v7+ client so we should send ext type-4 message */
	 DEBUG(150, ("Extended type-4 message thru server (rest_size = %d)\n", rest_size));
	 	 
	 v7_send_user_message_x4e(msg_hdr, *to_user, msg_buff, 
	                          (char *)tlv->nextData, rest_size);
      }
      else
      {
         /* This is old-style client so we should send simply message */
	 DEBUG(150, ("Normal type-4 message thru server\n"));
         send_online_message(msg_hdr, *to_user, msg_buff);
      }
   } 
   else 
   {
      /* now we should extract message string from tlv -> maxsize = 450 */
      if (!v7_extract_string(msg_buff, *tlv, 450, "sysmsg", *user, pack)) 
         return;
	
      DEBUG(150, ("OFMessage (%d) from user %lu to user %lu\n", msg_hdr.mtype, 
                  msg_hdr.fromuin, msg_hdr.touin));

      /* TLV 0x03 mean that user want server ack */
      if (tlv_chain.get(0x3) != NULL)
      { 
         send_type2_ack(user, snac, msg_hdr, MSG_KIND_SYS);
      }
   
      db_add_message(msg_hdr, msg_buff);
   }

}


/**************************************************************************/
/* Send online message to user					  	  */
/**************************************************************************/
int v7_send_user_message(struct msg_header &msg_hdr, struct online_user &to_user, 
                         char *message)
{
   unsigned short i,j,s=0,cnt;
   struct online_user *tuser = NULL;
   tuser = shm_iget_user(to_user.uin, to_user.shm_index);

   /* some messages should be sent thru SSI */
   if (tuser != NULL)
   {
      if ((msg_hdr.mtype == MSG_TYPE_AUTH_GRANTED) &&
          (tuser->enable_ssi))
      {
         grant_ssi_authorization(msg_hdr.fromuin, to_user.uin);
         return(0);
      }

      if ((msg_hdr.mtype == MSG_TYPE_ADDED) &&
          (tuser->enable_ssi))
      {
         ssi_send_you_added(msg_hdr.fromuin, tuser);
         return(0);
      }

      if ((msg_hdr.mtype == MSG_TYPE_AUTH_REQ) &&
          (tuser->enable_ssi))
      {
         /* Here I should ajust message - remove \xFE units */
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
	 
         ssi_send_auth_req(msg_hdr.fromuin, tuser, message+i);
         return(0);
      }

      if ((msg_hdr.mtype == MSG_TYPE_AUTH_DENIED) &&
          (tuser->enable_ssi))
      {
         ssi_send_auth_denied(msg_hdr.fromuin, tuser, message);
         return(0);
      }
   }      

   if (msg_hdr.mtype != 0x0001)
   {
      /* message from client (type-4) */
      v7_send_user_message_x4(msg_hdr, to_user, message);
   }
   else
   {
      /* old-style message (type-1) This message also doesn't */
      /* contain message_type field, so it can be only normal */
      /* text message */
      v7_send_user_message_x1(msg_hdr, to_user, message);
   }
   
   return(0);
}


/**************************************************************************/
/* Send old-style online message to user (type-1 message)	  	  */
/* looks like simple online message, type-2 --> offline message           */
/**************************************************************************/
int v7_send_user_message_x1(struct msg_header &msg_hdr, struct online_user &to_user, 
                            char *message)
{
   char buin[32];
   unsigned short tlvs_num = 0;
   unsigned long uptime;
   struct online_user *fuser;

   arpack.clearPacket();
   arpack.setup_aim();
   
   snprintf(buin, 31, "%lu", msg_hdr.fromuin);
   fuser = shm_iget_user(msg_hdr.fromuin, msg_hdr.fromindex);

   /* first part of the message - constant */
   arpack << (unsigned short)SN_TYP_MESSAGING
          << (unsigned short)SN_MSG_RECVxMESSAGE
	  << (unsigned short)0x0000
	  << (unsigned  long)to_user.servseq2++
	  
	  << (unsigned  long)timeToLong(msg_hdr.mtime)
	  << (unsigned  long)lrandom_num()
	  << (unsigned short)MSG_TYPE_1
	  << (char)strlen(buin)
	  << buin;
	  
   /* now time to put second part of message - tlv chain */
   reply_pack.clearPacket();
   reply_pack.setup_aim();
   
   if (fuser != NULL)
   {
      tlvs_num = 4;
      uptime   = timeToLong(time(NULL)) - fuser->uptime;
      
      reply_pack << (unsigned short)0x0001 /* user class */
 	         << (unsigned short)sizeof(fuser->uclass)
	         << (unsigned short)fuser->uclass

                 << (unsigned short)0x0006 /* user ext status */
	         << (unsigned short)(sizeof(fuser->estat)+sizeof(fuser->status))
	         << (unsigned short)fuser->estat
	         << (unsigned short)fuser->status
	      
                 << (unsigned short)0x000f /* user uptime TLV value   */
	         << (unsigned short)(sizeof(uptime))
	         << (unsigned  long)uptime

 	         << (unsigned short)0x0003 /* account create time */
	         << (unsigned short)(sizeof(fuser->crtime))
	         << (unsigned  long)fuser->crtime;
   }

   reply_pack << (unsigned short)0x0002
	      << (unsigned short)(msg_hdr.msglen+13)
	      
	      << (unsigned short)0x0501 /* i'm not sure... */
	      << (unsigned short)0x0001
	      << (char)0x00

	      << (unsigned short)0x0101
	      << (unsigned short)(msg_hdr.msglen+4)
	      << (unsigned short)msg_hdr.mcharset
	      << (unsigned short)msg_hdr.mcsubset;

   /* now time to copy message. Not asciiz string - text/unicode/utf-8 */
   memcpy((void *)reply_pack.nextData, (const void *)message, msg_hdr.msglen);
   reply_pack.sizeVal += msg_hdr.msglen;

   /* copy TLV chain from reply_pack to arpack */
   arpack << (unsigned short)0x0000
	  << (unsigned short)tlvs_num;

   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());

   arpack.sizeVal     += reply_pack.size();
   arpack.from_ip      = to_user.ip;
   arpack.from_port    = 0;
   arpack.sock_hdl     = to_user.sock_hdl;
   arpack.sock_rnd     = to_user.sock_rnd;
   arpack.sock_evt     = SOCK_DATA;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 2;
   
   DEBUG(100, ("Msg_txt: Plain-text message from %lu to user %lu\n", 
                msg_hdr.fromuin, to_user.uin));

   /* send packet to client */
   tcp_writeback_packet(arpack);
   return(0);
}


/**************************************************************************/
/* Send old-style online message to user (type-2 message) from old client */
/**************************************************************************/
int v7_send_user_message_x4(struct msg_header &msg_hdr, struct online_user &to_user, 
                            char *message)
{
   char buin[32];
   char *mstring = NULL;
   unsigned long uptime;
   unsigned short mtype;
   struct online_user *fuser;

   arpack.clearPacket();
   arpack.setup_aim();
   
   mtype = v7_convert_message_type(msg_hdr.mtype);
   mstring = v7_convert_message_text(msg_hdr.fromuin, msg_hdr.mtype, message);
   
   if (mstring == NULL) mstring = message;
   if (msg_hdr.mtype == 0x14) msg_hdr.fromuin = ADMIN_UIN;
   snprintf(buin, 31, "%lu", msg_hdr.fromuin);
   fuser = shm_iget_user(msg_hdr.fromuin, msg_hdr.fromindex);
   
   /* first part of the message - constant */
   arpack << (unsigned short)SN_TYP_MESSAGING
          << (unsigned short)SN_MSG_RECVxMESSAGE
	  << (unsigned short)0x0000
	  << (unsigned  long)to_user.servseq2++
	  
	  << (unsigned  long)timeToLong(msg_hdr.mtime)
	  << (unsigned  long)lrandom_num()
	  << (unsigned short)MSG_TYPE_4
	  << (char)strlen(buin)
	  << buin
	  << (unsigned short)0x0000;


   if (fuser != NULL)
   {
      uptime = timeToLong(time(NULL)) - fuser->uptime;

      arpack << (unsigned short)0x0004
      
             << (unsigned short)0x0001 /* user class */
	     << (unsigned short)sizeof(fuser->uclass)
	     << (unsigned short)fuser->uclass
	     
             << (unsigned short)0x0006 /* user ext status */
  	     << (unsigned short)(sizeof(fuser->estat)+sizeof(fuser->status))
	     << (unsigned short)fuser->estat
	     << (unsigned short)fuser->status
	       
             << (unsigned short)0x000f /* user uptime TLV value   */
 	     << (unsigned short)(sizeof(uptime))
	     << (unsigned  long)uptime

 	     << (unsigned short)0x0003 /* account create time */
 	     << (unsigned short)(sizeof(fuser->crtime))
	     << (unsigned  long)fuser->crtime;
   }
   else
   {
      arpack << (unsigned short)0x0000;
   }

   /* not time to put message TLV */
   reply_pack.clearPacket();
   reply_pack.null_terminated();
   reply_pack.intel_order();

   reply_pack << (unsigned  long)msg_hdr.fromuin
              << (unsigned short)mtype
	      << (unsigned short)(strlen(mstring)+1)
	      << mstring;

   arpack << (unsigned short)0x0005
          << (unsigned short)reply_pack.size();
	     
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();

   arpack.from_ip      = to_user.ip;
   arpack.from_port    = 0;
   arpack.sock_hdl     = to_user.sock_hdl;
   arpack.sock_rnd     = to_user.sock_rnd;
   arpack.sock_evt     = SOCK_DATA;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 2;
   
   DEBUG(100, ("Msg_4: Sending type-4 message from %lu to user %lu\n", 
                msg_hdr.fromuin, to_user.uin));

   /* send packet to client */
   tcp_writeback_packet(arpack);

   return(0);
}


/**************************************************************************/
/* Send new-style online message to user (type-4 message) from new client */
/**************************************************************************/
int v7_send_user_message_x4e(struct msg_header &msg_hdr, struct online_user &to_user, 
                             char *message, char *rest_data, unsigned short rest_size)
{
   char buin[32];
   unsigned long uptime;
   struct online_user *fuser;

   arpack.clearPacket();
   arpack.setup_aim();
   
   snprintf(buin, 31, "%lu", msg_hdr.fromuin);
   fuser = shm_iget_user(msg_hdr.fromuin, msg_hdr.fromindex);

   /* first part of the message - constant */
   arpack << (unsigned short)SN_TYP_MESSAGING
          << (unsigned short)SN_MSG_RECVxMESSAGE
	  << (unsigned short)0x0000
	  << (unsigned  long)to_user.servseq2++
	  
	  << (unsigned  long)timeToLong(msg_hdr.mtime)
	  << (unsigned  long)lrandom_num()
	  << (unsigned short)MSG_TYPE_4
	  << (char)strlen(buin)
	  << buin;
	
   if (fuser != NULL)
   {
      uptime = timeToLong(time(NULL)) - fuser->uptime;
      arpack << (unsigned short)fuser->warn_level
	     << (unsigned short)0x0004
	  
             << (unsigned short)0x0001 /* user class field */
	     << (unsigned short)sizeof(fuser->uclass)
	     << (unsigned short)fuser->uclass

             << (unsigned short)0x0006 /* user ext status */
	     << (unsigned short)(sizeof(fuser->estat)+sizeof(fuser->status))
	     << (unsigned short)fuser->estat
	     << (unsigned short)fuser->status
	      
             << (unsigned short)0x000f /* user uptime value */
	     << (unsigned short)(sizeof(uptime))
	     << (unsigned  long)uptime

 	     << (unsigned short)0x0003 /* account create time */
	     << (unsigned short)(sizeof(fuser->crtime))
	     << (unsigned  long)fuser->crtime;
   }
   else
   {
      arpack << (unsigned short)0x0000
             << (unsigned short)0x0000;
   }
   
   /* not time to put message TLV */
   reply_pack.clearPacket();
   reply_pack.null_terminated();
   reply_pack.intel_order();

   reply_pack << (unsigned  long)msg_hdr.fromuin
              << (unsigned short)msg_hdr.mtype
	      << (unsigned short)(strlen(message)+1)
	      << message;

   /* put rest data after message string (colors, gcard data, etc..) */
   memcpy(reply_pack.nextData, rest_data, rest_size);
   reply_pack.sizeVal += rest_size;

   arpack << (unsigned short)0x0005
          << (unsigned short)reply_pack.size();
	     
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   arpack.from_ip      = to_user.ip;
   arpack.from_port    = 0;
   arpack.sock_hdl     = to_user.sock_hdl;
   arpack.sock_rnd     = to_user.sock_rnd;
   arpack.sock_evt     = SOCK_DATA;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 2;
   
   DEBUG(100, ("Msg_4e: Sending type4 msg (%lu-->%lu)\n",
                msg_hdr.fromuin, to_user.uin));

   /* send packet to client */
   tcp_writeback_packet(arpack);
   return(0);
}


/**************************************************************************/
/* Send real new-style online message to user (type-2 message)	  	  */
/* It has very complex structure and too many fields :(                   */
/**************************************************************************/
int v7_send_user_message_x2(struct msg_header &msg_hdr, struct online_user *to_user, 
                            struct online_user *fuser, char *required_cap, 
			    class tlv_c *tlv2711)
{
   int i;
   char buin[32];
   unsigned long uptime;

   arpack.clearPacket();
   arpack.setup_aim();
   
   snprintf(buin, 31, "%lu", msg_hdr.fromuin);
   uptime = timeToLong(time(NULL)) - fuser->uptime;

   /* first part of the message - constant */
   arpack << (unsigned short)SN_TYP_MESSAGING
          << (unsigned short)SN_MSG_RECVxMESSAGE
	  << (unsigned short)0x0000
	  << (unsigned  long)to_user->servseq2++;
	  
   for (i=0; i<8; i++) arpack << (char)msg_hdr.msg_cookie[i];
   
   arpack << (unsigned short)MSG_KIND_ADV
	  << (char)strlen(buin)
	  << buin
	  << (unsigned short)fuser->warn_level
	  << (unsigned short)0x0004 /* tlv number */

	  << (unsigned short)0x0001 /* user class */
	  << (unsigned short)sizeof(fuser->uclass)
	  << (unsigned short)fuser->uclass

          << (unsigned short)0x0006 /* user ext status */
	  << (unsigned short)(sizeof(fuser->estat)+sizeof(fuser->status))
	  << (unsigned short)fuser->estat
	  << (unsigned short)fuser->status
	      
          << (unsigned short)0x000f /* user uptime TLV value */
	  << (unsigned short)(sizeof(uptime))
	  << (unsigned  long)uptime

 	  << (unsigned short)0x0003 /* account create time */
	  << (unsigned short)(sizeof(fuser->crtime))
	  << (unsigned  long)fuser->crtime;

   /* Ok now we should create second part - TLV 0x05 */
   reply_pack.clearPacket();
   reply_pack.setup_aim();
 
   reply_pack << msg_hdr.fabt_ack;
   
   for (i=0; i<8;  i++) reply_pack << (char)msg_hdr.msg_cookie[i]; 
   for (i=0; i<16; i++) reply_pack << (char)required_cap[i];
   
   if (msg_hdr.fabt_ack == 0x0001)
   {
      reply_pack << (unsigned short)0x000B
    	         << (unsigned short)sizeof(msg_hdr.f_abort)
		 << (unsigned short)msg_hdr.f_abort;
   }
   else
   {
      reply_pack << (unsigned short)0x000A
                 << (unsigned short)sizeof(msg_hdr.fok_ack)
		 << (unsigned short)msg_hdr.fok_ack;
	
      if (msg_hdr.int_ip != 0)
      {
         reply_pack << (unsigned short)0x0003
	            << (unsigned short)sizeof(msg_hdr.int_ip)
		    << (unsigned  long)msg_hdr.int_ip;
      }

      if (msg_hdr.tport != 0)
      {
         reply_pack << (unsigned short)0x0005
	            << (unsigned short)sizeof(msg_hdr.tport)
		    << (unsigned short)msg_hdr.tport;
      }
      
      reply_pack << (unsigned short)0x000F
    	         << (unsigned short)0x0000;
		       
      /* OK.. Here we should put TLV(2711) */
      reply_pack << (unsigned short)0x2711
                 << (unsigned short)tlv2711->size;

      memcpy(reply_pack.nextData, tlv2711->value, tlv2711->size);
      reply_pack.sizeVal  += tlv2711->size;
      reply_pack.nextData += tlv2711->size;

      /* Now we should check if we should put tlv 0x04 with fuser ext_ip */
      if ((msg_hdr.fabt_ack != 0x0001) &&
          (msg_hdr.tport != 0))
      {
         reply_pack << (unsigned short)0x0004
		    << (unsigned short)sizeof(fuser->ip)
  	            << (unsigned  long)ipToIcq2(fuser->ip);
      }
   }

   /* now reply_pack contain tlv(0x05) so we should put it into arpack */
   arpack << (unsigned short)0x0005
          << (unsigned short)reply_pack.size();
	  
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();

   arpack.from_ip      = to_user->ip;
   arpack.from_port    = 0;
   arpack.sock_hdl     = to_user->sock_hdl;
   arpack.sock_rnd     = to_user->sock_rnd;
   arpack.sock_evt     = SOCK_DATA;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 2;
   
   DEBUG(51, ("Adv_msgR: Sending message from %lu to user %lu\n", 
                fuser->uin, to_user->uin));

   /* Uff... send packet to client */
   tcp_writeback_packet(arpack);   
   return(0);
}


/**************************************************************************/
/* Send type-2 message ack to specified user			  	  */
/**************************************************************************/
void send_type2_ack(struct online_user *user, struct snac_header &snac, 
                    struct msg_header &msg_hdr, unsigned long channel)
{
   char buin[32];   
   snprintf(buin, 31, "%lu", msg_hdr.touin);

   reply_pack.clearPacket();
   reply_pack.network_order();
   reply_pack.no_null_terminated();
   
   reply_pack << (unsigned short)SN_TYP_MESSAGING
              << (unsigned short)SN_MSG_SRVxMESSxACK
              << (unsigned short)0x0000
              << (unsigned  long)snac.id;

   for (int i=0;i<8;i++) reply_pack << msg_hdr.msg_cookie[i];
   
   reply_pack << (unsigned short)channel
              << (char)strlen(buin)
	      << buin;
	      
   reply_pack.from_ip      = user->ip;
   reply_pack.from_port    = 0x0000;
   reply_pack.sock_hdl     = user->sock_hdl;
   reply_pack.sock_rnd     = user->sock_rnd;
   reply_pack.sock_evt     = SOCK_DATA;
   reply_pack.sock_type    = SAIM;
   reply_pack.flap_channel = 0x02;


   DEBUG(100, ("Sending type-2 message ack to %lu...\n", user->uin));
   tcp_writeback_packet(reply_pack);

}


/**************************************************************************/
/* Send message missed snac to receiver				  	  */
/**************************************************************************/
void send_missed_message(struct online_user *from_user, 
                         struct msg_header &msg_hdr, unsigned short missed_num, 
			 unsigned short reason)
{
   struct online_user *to_user;
   char buin[32];   

   if ((to_user = shm_get_user(msg_hdr.touin)) == NULL) return;   
   if ((msg_hdr.mkind == 0) || (msg_hdr.mkind > MAX_ICBM_CHANNELS)) return;
   if ((to_user->mopt[msg_hdr.mkind-1].icbm_flags & ICBM_FLG_MISSED) == 0) return;
   
   reply_pack.clearPacket();
   reply_pack.network_order();
   reply_pack.no_null_terminated();
   snprintf(buin, 31, "%lu", from_user->uin);   
   
   reply_pack << (unsigned short)SN_TYP_MESSAGING
              << (unsigned short)SN_MSG_MISSEDxCALLS
              << (unsigned short)0x0000
              << (unsigned  long)to_user->servseq2++;

   /* OK... Now put userinfo into packet */
   reply_pack << (unsigned short)msg_hdr.mkind
              << (char)strlen(buin)
	      << buin
	      
              << (unsigned short)from_user->warn_level
	      << (unsigned short)0x0004 /* num of userinfo TLVs */
	      
   	      << (unsigned short)0x0001 
	      << (unsigned short)sizeof(from_user->uclass)
	      << (unsigned short)from_user->uclass

              << (unsigned short)0x0006 /* user ext status */
	      << (unsigned short)(sizeof(from_user->estat)+sizeof(from_user->status))
	      << (unsigned short)from_user->estat
	      << (unsigned short)from_user->status
	      
              << (unsigned short)0x000f /* user idle time TLV value */
              << (unsigned short)(sizeof(from_user->idle_time));
	      
   if (from_user->idle_perms)
   {
      reply_pack << (unsigned long)(time(NULL) - from_user->idle_time);
   }
   else
   {
      reply_pack << (unsigned long)0x00000000;
   }

   reply_pack << (unsigned short)0x0003 /* account create time */
	      << (unsigned short)(sizeof(from_user->crtime))
  	      << (unsigned  long)from_user->crtime;

   /* now num of missed messages and reason */
   reply_pack << (unsigned short)missed_num
              << (unsigned short)reason;
	      
   reply_pack.from_ip      = to_user->ip;
   reply_pack.from_port    = 0x0000;
   reply_pack.sock_hdl     = to_user->sock_hdl;
   reply_pack.sock_rnd     = to_user->sock_rnd;
   reply_pack.sock_evt     = SOCK_DATA;
   reply_pack.sock_type    = SAIM;
   reply_pack.flap_channel = 0x02;

   DEBUG(100, ("Sending message missed notice to %lu...\n", to_user->uin));
   tcp_writeback_packet(reply_pack);

}


/**************************************************************************/
/* Send message missed snac to receiver (without user lookup)	  	  */
/**************************************************************************/
void send_missed_message(struct online_user *to_user, struct online_user *from_user, 
                         struct msg_header &msg_hdr, unsigned short missed_num, 
			 unsigned short reason)
{
   char buin[32];   

   if ((msg_hdr.mkind == 0) || (msg_hdr.mkind > MAX_ICBM_CHANNELS)) return;
   if ((to_user->mopt[msg_hdr.mkind-1].icbm_flags & ICBM_FLG_MISSED) == 0) return;
   
   reply_pack.clearPacket();
   reply_pack.network_order();
   reply_pack.no_null_terminated();
   snprintf(buin, 31, "%lu", from_user->uin);
   
   reply_pack << (unsigned short)SN_TYP_MESSAGING
              << (unsigned short)SN_MSG_MISSEDxCALLS
              << (unsigned short)0x0000
              << (unsigned  long)to_user->servseq2++;

   /* OK... Now put userinfo into packet */
   reply_pack << (unsigned short)msg_hdr.mkind
              << (char)strlen(buin)
	      << buin;
	      
   reply_pack << (unsigned short)from_user->warn_level
	      << (unsigned short)0x0004 /* num of userinfo tlvs */
	      
   	      << (unsigned short)0x0001 /* user class */
	      << (unsigned short)sizeof(from_user->uclass)
	      << (unsigned short)from_user->uclass

              << (unsigned short)0x0006 /* user ext status */
	      << (unsigned short)(sizeof(from_user->estat)+sizeof(from_user->status))
	      << (unsigned short)from_user->estat
	      << (unsigned short)from_user->status
	      
              << (unsigned short)0x000f /* user uptime TLV value   */
	      << (unsigned short)(sizeof(from_user->idle_time))
	      << (unsigned  long)from_user->idle_time

 	      << (unsigned short)0x0003 /* account create time */
	      << (unsigned short)(sizeof(from_user->crtime))
  	      << (unsigned  long)from_user->crtime;

   /* now num of missed messages and reason */
   reply_pack << (unsigned short)missed_num
              << (unsigned short)reason;
	      
   reply_pack.from_ip      = to_user->ip;
   reply_pack.from_port    = 0x0000;
   reply_pack.sock_hdl     = to_user->sock_hdl;
   reply_pack.sock_rnd     = to_user->sock_rnd;
   reply_pack.sock_evt     = SOCK_DATA;
   reply_pack.sock_type    = SAIM;
   reply_pack.flap_channel = 0x02;

   DEBUG(100, ("Sending message missed notice to %lu...\n", to_user->uin));
   tcp_writeback_packet(reply_pack);

}


/**************************************************************************/
/* Send new-style online message to user (using tlv_chain)	  	  */
/**************************************************************************/
int v7_send_user_message_x2(struct msg_header &msg_hdr, struct online_user *to_user, 
                            struct online_user *fuser, char *required_cap, 
			    class tlv_chain_c &tlv_chain, BOOL ack_flag)
{
   int i;
   char buin[32];
   unsigned long  uptime;

   arpack.clearPacket();
   arpack.setup_aim();
   
   snprintf(buin, 31, "%lu", msg_hdr.fromuin);
   uptime = timeToLong(time(NULL)) - fuser->uptime;

   /* first part of the message - constant */
   arpack << (unsigned short)SN_TYP_MESSAGING
          << (unsigned short)SN_MSG_RECVxMESSAGE
	  << (unsigned short)0x0000
	  << (unsigned  long)to_user->servseq2++;
	  
   for (i=0; i<8; i++) arpack << (char)msg_hdr.msg_cookie[i];
   
   arpack << (unsigned short)MSG_KIND_ADV
	  << (char)strlen(buin)
	  << buin
	  << (unsigned short)fuser->warn_level
	  << (unsigned short)0x0004 /* number of info tlvs */

	  << (unsigned short)0x0001 /* user class */
	  << (unsigned short)sizeof(fuser->uclass)
	  << (unsigned short)fuser->uclass

          << (unsigned short)0x0006 /* user ext status */
	  << (unsigned short)(sizeof(fuser->estat)+sizeof(fuser->status))
	  << (unsigned short)fuser->estat
	  << (unsigned short)fuser->status
	      
          << (unsigned short)0x000f /* user uptime TLV value */
	  << (unsigned short)(sizeof(uptime))
	  << (unsigned  long)uptime

 	  << (unsigned short)0x0003 /* account create time */
	  << (unsigned short)(sizeof(fuser->crtime))
	  << (unsigned  long)fuser->crtime;

   /* Ok now we should create second part - TLV 0x05 */
   reply_pack.clearPacket();
   reply_pack.setup_aim();
 
   reply_pack << msg_hdr.fabt_ack;
   
   for (i=0; i<8;  i++) reply_pack << (char)msg_hdr.msg_cookie[i]; 
   for (i=0; i<16; i++) reply_pack << (char)required_cap[i];
   
   tlv_chain.addToPacket(reply_pack);

   /* now reply_pack contain tlv(0x05) so we should put it into arpack */
   arpack << (unsigned short)0x0005
          << (unsigned short)reply_pack.size();
	  
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal  += reply_pack.size();
   arpack.nextData += reply_pack.size();

   /* This is the sender wish to receive type-2 */
   /* ack from receiver --> SNAC(04,0B) */
   if (ack_flag)
   {
      arpack << (unsigned short)0x0003
             << (unsigned short)0x0000;
   }
   
   /* ICQ2003 want this for DC connections */
   if (tlv_chain.get(0x0003) != NULL)
   {
      arpack << (unsigned short)0x0004
             << (unsigned short)SIZEOF_UNSIGNED_LONG
	     << (unsigned  long)ipToIcq(fuser->ip);
   }
   
   arpack.from_ip      = to_user->ip;
   arpack.from_port    = 0;
   arpack.sock_hdl     = to_user->sock_hdl;
   arpack.sock_rnd     = to_user->sock_rnd;
   arpack.sock_evt     = SOCK_DATA;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 2;
   
   DEBUG(51, ("Adv_msg(chain): Sending message from %lu to user %lu\n", 
                fuser->uin, to_user->uin));

   /* Uff... send packet to client */
   tcp_writeback_packet(arpack);   
   return(0);
}


