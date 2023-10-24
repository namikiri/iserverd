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
/* Generic SNAC family packets handler					  */
/**************************************************************************/
void process_snac_generic(struct snac_header &snac, Packet &pack)
{
   switch (snac.subfamily)
   {
      case SN_GEN_REQUESTxVERS:   process_gen_req_versions(snac, pack); break;
      case SN_GEN_REQUESTxRATE:   process_gen_req_rates(snac, pack);    break;
      case SN_GEN_RATExACK:	  process_gen_ack_rates(snac, pack);    break;
      case SN_GEN_INFOxREQUEST:   process_gen_get_info(snac, pack);     break;
      case SN_GEN_SETxSTATUS:	  process_gen_set_status(snac, pack);   break;
      case SN_GEN_CLIENTxREADY:   process_gen_cli_ready(snac, pack);	break;
      case SN_GEN_SETxIDLETIME:	  process_gen_set_idletime(snac, pack); break;
      case SN_GEN_SERVICExREQ:	  process_gen_service_req(snac, pack);  break;
      
      default: DEBUG(10, ("Unknown generic SNAC(0x1, %04X)\n", snac.subfamily));
   }
}


/**************************************************************************/
/* Client have sent its idle time					  */
/**************************************************************************/
void process_gen_service_req(struct snac_header &snac, Packet &pack)
{
   struct online_user *user;
   unsigned short req_family;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip)));
      
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   pack >> req_family;
   DEBUG(10, ("Client request service %04X (uin=%lu, ip=%s)\n", 
             req_family, user->uin, inet_ntoa(pack.from_ip)));
   
   /* Here we should search requested service in all */
   /* cluster members and return server addr/cookie  */
   
   send_snac_error(snac.family, ERR_SERVICE_NON_DEFINED, snac.id, pack);
}


/**************************************************************************/
/* Client have sent its idle time					  */
/**************************************************************************/
void process_gen_set_idletime(struct snac_header &snac, Packet &pack)
{
   struct online_user *user;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip)));
      
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   pack >> user->idle_perms;
   user->idle_time = time(NULL) - 60;
}


/**************************************************************************/
/* generic_request_needed_family_versions handler			  */
/**************************************************************************/
void process_gen_req_versions(struct snac_header &snac, Packet &pack)
{
   struct aim_family *bam = aim_root.aim_families;
   unsigned short family, version;
   int pair_num = (pack.sizeVal - 10) / 4;
   int i;
   struct online_user *user;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s sent versions request but not authorized...\n", 
                  inet_ntoa(pack.from_ip)));
      
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   DEBUG(10, ("Client uin=%lu, active=%d, ip:%s, dc_type=%d\n", user->uin, user->active, inet_ntoa(user->ip), user->dc_type));
      
   /* Prepare reply packet */
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();

   arpack << (unsigned short)SN_TYP_GENERIC
          << (unsigned short)SN_GEN_VERSxRESPONSE
	  << (unsigned short)0x0000
	  << (unsigned  long)user->servseq2++;

   for(i=0;i<pair_num;i++)   
   {
      pack >> family >> version;
      
      /* check if we can enable SSI    */
      if (family == SN_TYP_SSI)
      {
	 user->enable_ssi = 1;
	 user->ssi_version = version;
      }
   }
   
   while(bam)
   {
      if (bam->number != SN_TYP_REGISTRATION)
      {
         arpack << (unsigned short)bam->number
                << (unsigned short)bam->version;
	
	 if ((bam->number == SN_TYP_SSI) &&
	     (user->ssi_version > 0) &&
  	     (user->ssi_version >= bam->version))
	 {
	    user->ssi_version = 0;
	 }
	 else
	 {
	    user->ssi_version = bam->version;
	 }
      }
            
      bam = bam->next;
   }

   DEBUG(200, ("Sending server families versions packet\n"));
   
   arpack.sock_evt     = SOCK_DATA;
   arpack.flap_channel = 0x02;
   tcp_writeback_packet(arpack);
   
   snac.id = user->servseq2++;
   send_srv_motd(snac, arpack);
}


/**************************************************************************/
/* generic_request_rate_limits handler					  */
/**************************************************************************/
void process_gen_req_rates(struct snac_header &snac, Packet &pack)
{
   struct aim_family *bam = aim_root.aim_families;
   struct rate_class *brc = aim_root.rate_classes;
   struct subtype    *subtypes;
   unsigned short rclasses_num, i;
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
   rclasses_num = get_rate_classes_num();

   arpack << (unsigned short)SN_TYP_GENERIC
          << (unsigned short)SN_GEN_RATExRESPONSE
	  << (unsigned short)0x0000
	  << (unsigned  long)snac.id;
	     
   /* Dump rate classes to reply packet */
   arpack << (unsigned short)rclasses_num;

   while(brc)
   {
      /* this is just ugly immitation of real limits */
      arpack << (unsigned short)brc->rate_index;
      arpack << (unsigned long)get_rate_param(brc->rate_index, "Window_size");
      arpack << (unsigned long)get_rate_param(brc->rate_index, "Clear_level");
      arpack << (unsigned long)get_rate_param(brc->rate_index, "Alert_level");
      arpack << (unsigned long)get_rate_param(brc->rate_index, "Limit_level");
      arpack << (unsigned long)get_rate_param(brc->rate_index, "Disconnect_level");
      arpack << (unsigned long)(get_rate_param(brc->rate_index, "Max_level")-100); /* current level */
      arpack << (unsigned long)get_rate_param(brc->rate_index, "Max_level");
      arpack << (unsigned long)0x00000000;
      arpack << (char)0x00;
      
      brc = brc->next;
   }
   
   /* Dump families with subtypes and rate indexes to packet */
   for (i=1; i<=rclasses_num; i++)
   {
      arpack << (unsigned short)i
             << (unsigned short)get_subtypes_num(i);
      
      bam = aim_root.aim_families;
      
      while (bam)
      {
         subtypes = bam->subtypes;
	 
         while (subtypes)
         {
            if (subtypes->rate_ind == i)
	    {
	       arpack << (unsigned short)bam->number
	              << (unsigned short)subtypes->num;
	    }
	    
            subtypes = subtypes->next;
         }
      
         bam = bam->next;
      }
   }
   
   DEBUG(200, ("Sending server rate limites packet\n"));
   arpack.flap_channel = 0x02;
   arpack.sock_evt     = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* client tell us that it accept out rate limits, we should check this	  */
/**************************************************************************/
void process_gen_ack_rates(struct snac_header &snac, Packet &pack)
{
   struct rate_class *brc = aim_root.rate_classes;
   int pair_num = (pack.sizeVal - 10) / 2;
   unsigned short rate_ind;
   struct online_user *user;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s:%d request but not authorized...\n", 
                  inet_ntoa(pack.from_ip), pack.from_port));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   while (brc)
   {
      pack >> rate_ind;
      pair_num--;
      
      if ((pair_num < 0) || (brc->rate_index != rate_ind))
      {
         DEBUG(50, ("Client refuses our rate-limits, what the hell ?\n"));
         log_alarm_packet(10, pack);
	 close_connection(pack);
	 return;
      }
      
      brc = brc->next;      
   }   
}


/**************************************************************************/
/* Client wants to know its online info (to check if it is up-to-date)	  */
/**************************************************************************/
void process_gen_get_info(struct snac_header &snac, Packet &pack)
{
   struct online_user *user;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s request but not authorized...\n", 
                  inet_ntoa(pack.from_ip)));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   send_status_info(snac, pack, user);
}


/**************************************************************************/
/* client sent its status						  */
/**************************************************************************/
void process_gen_set_status(struct snac_header &snac, Packet &pack)
{
   struct online_user *user;
   unsigned long int_ip;
   unsigned short old_status;
   int active = 0;
   class tlv_chain_c tlv_chain;
   class tlv_c *tlv;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip)));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }
   
   active = user->active;
   tlv_chain.read(pack);
   tlv_chain.network_order();

   if (((tlv = tlv_chain.get(0x6)) == NULL) ||
        (tlv->size < 4)) 
   {
      DEBUG(10, ("ERROR: Mailformed set_status packet from %s:%d (tlv(0x6))\n", 
                inet_ntoa(pack.from_ip), pack.from_port));
		
      send_snac_error(snac.family, ERR_SNAC_DATA_INVALID, snac.id, pack);
      return;
   }
   
   /* lets get data from tlv chunk using packet class */
   old_status = user->status;
   
   *tlv >> user->estat
        >> user->status;

   if (user->status != 0) user->uclass = user->uclass | CLASS_AWAY;
   if (user->status == 0) user->uclass = user->uclass & (~CLASS_AWAY);
   
   db_online_setstatus(*user);
   
   /* check for direct connection info in user_status snac */
   if ((tlv = tlv_chain.get(0xC)) != NULL)
   {   
     *tlv >> int_ip
          >> user->tcp_port
          >> user->dc_type
          >> user->tcpver
          >> user->dc_cookie
          >> user->web_port
          >> user->cli_futures
          >> user->info_utime
	  >> user->more_utime
	  >> user->stat_utime;
	  
      user->int_ip = icqToIp2(int_ip);
      
      DEBUG(50, ("iip=%s,tprt=%lu,dctype=%d,dccook=%08X,tcpv=%d,wprt=%lu,ftr=%lu\n", 
                inet_ntoa(user->int_ip), user->tcp_port, user->dc_type, 
		(unsigned short)user->dc_cookie, user->tcpver, user->web_port, 
		user->cli_futures));

      send_status_info2(user, user);
   }

   send_event2ap(papack, ACT_STATUS, user->uin, old_status,
                 ipToIcq(user->ip), user->status, longToTime(time(NULL)), "");

   /* if user is activated we should broadcast its new status */
   if (active) broadcast_status(*user, old_status);
}


/**************************************************************************/
/* This func create and send online_info packet	(mod 1)			  */
/**************************************************************************/
void send_status_info(struct snac_header &snac, Packet &pack, struct online_user *user)
{
   /* Prepare reply packet */
   char scr_name[32];
   struct full_user_info tuser;
   
   if (db_users_lookup(user->uin, tuser) < 0)
   { 
      /* we can't found user info db/internal error ? */
      LOG_SYS(0, ("Can't find client %lu information. Closing connection...\n", user->uin));
      close_connection(pack);
      return;
   }

   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();

   arpack << (unsigned short)SN_TYP_GENERIC
          << (unsigned short)SN_GEN_INFOxRESPONSE
	  << (unsigned short)0x0000
	  << (unsigned  long)snac.id;

   /* Well, header ready... now we can drop into packet all needed data */   
   snprintf(scr_name, 31, "%lu", user->uin);
   arpack << (char)(strlen(scr_name));
   arpack << scr_name;
   
   arpack << (unsigned short)0x0000	/* warn level     */
          << (unsigned short)0x0008	/* number of TLVs */
	  
	  << (unsigned short)0x0001
	  << (unsigned short)sizeof(user->uclass)
	  << (unsigned short)user->uclass	  

	  << (unsigned short)0x000c
	  << (unsigned short)0x0025
	  << (unsigned  long)ipToIcq2(user->int_ip)
	  << (unsigned  long)user->tcp_port
	  << (char)user->dc_type
	  << (unsigned short)user->tcpver
	  << (unsigned  long)user->dc_cookie
	  << (unsigned  long)user->web_port
	  << (unsigned  long)user->cli_futures
	  << (unsigned  long)user->info_utime
	  << (unsigned  long)user->more_utime
	  << (unsigned  long)user->stat_utime
	  << (unsigned short)0x0000
	  
	  << (unsigned short)0x000a
	  << (unsigned short)sizeof(user->ip)
	  << (unsigned  long)ipToIcq2(user->ip)
	  
	  << (unsigned short)0x000f
	  << (unsigned short)sizeof(user->idle_time)
	  << (unsigned  long)user->idle_time
	  
	  << (unsigned short)0x0003
	  << (unsigned short)sizeof(user->uptime)
	  << (unsigned  long)user->uptime
	  
	  << (unsigned short)0x000a
	  << (unsigned short)sizeof(user->ip)
	  << (unsigned  long)ipToIcq2(user->ip)

	  << (unsigned short)0x001E
	  << (unsigned short)sizeof(unsigned long)
	  << (unsigned  long)0x00000000
	  	  
	  << (unsigned short)0x0005
	  << (unsigned short)sizeof(tuser.cr_date)
	  << (unsigned  long)tuser.cr_date;

   arpack.flap_channel = 0x02;
   arpack.sock_evt     = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* This func create and send online_info packet	(mod 2)			  */
/**************************************************************************/
void send_status_info2(struct online_user *to_user, struct online_user *user)
{
   char scr_name[32];
   
   arpack.clearPacket();
   arpack.setup_aim();

   arpack << (unsigned short)SN_TYP_GENERIC
          << (unsigned short)SN_GEN_INFOxRESPONSE
	  << (unsigned short)0x8000
	  << (unsigned  long)to_user->servseq2++
	  << (unsigned short)0x0006
	  << (unsigned short)0x0001
	  << (unsigned short)0x0002
	  << (unsigned short)0x0003;

   /* Well, header ready... now we can drop into packet all needed data */
   snprintf(scr_name, 31, "%lu", user->uin);
   arpack << (char)(strlen(scr_name));
   arpack << scr_name;
   
   arpack << (unsigned short)0x0000	/* warn level ?   */
          << (unsigned short)0x0007	/* number of TLVs */
	  
	  << (unsigned short)0x0001
	  << (unsigned short)sizeof(user->uclass)
	  << (unsigned short)user->uclass
	  
	  << (unsigned short)0x0006
	  << (unsigned short)(sizeof(user->estat)+sizeof(user->status))
	  << (unsigned short)user->estat
	  << (unsigned short)user->status

	  << (unsigned short)0x000f
	  << (unsigned short)sizeof(user->idle_time)
	  << (unsigned  long)user->idle_time
	  
	  << (unsigned short)0x0003
	  << (unsigned short)sizeof(user->uptime)
	  << (unsigned  long)user->uptime
	  
	  << (unsigned short)0x000a
	  << (unsigned short)sizeof(user->ip)
	  << (unsigned  long)ipToIcq2(user->ip)

	  << (unsigned short)0x001E
	  << (unsigned short)sizeof(unsigned long)
	  << (unsigned  long)0x00000000
	  	  
	  << (unsigned short)0x0005
	  << (unsigned short)sizeof(user->crtime)
	  << (unsigned  long)user->crtime;

   arpack.from_ip      = to_user->ip;
   arpack.from_port    = 0x0000;
   arpack.sock_hdl     = to_user->sock_hdl;
   arpack.sock_rnd     = to_user->sock_rnd;
   arpack.sock_evt     = SOCK_DATA;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 0x02;

   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* Process client ready packet						  */
/**************************************************************************/
void process_gen_cli_ready(struct snac_header &snac, Packet &pack)
{
   struct online_user *user;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s request but not authorized...\n", 
                  inet_ntoa(pack.from_ip)));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   /* we should activate it and broadcast its presense */
   if (!user->active)
   {
      db_online_activate_user(user->uin);
      move_user_online(*user);

      stats_send_interval(user);
   }

   if (user->cloaded)
   {
      user_send_presense_full(user);
   }
   
   LOG_USR(0, ("User %lu from %s moved online.\n", 
                user->uin, inet_ntoa(user->ip)));

}

