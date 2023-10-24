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
/*									  */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* Location services SNAC family packets handler			  */
/**************************************************************************/
void process_snac_location(struct snac_header &snac, Packet &pack)
{

   log_alarm_packet(100,pack);
   switch (snac.subfamily)
   {
      case SN_LOC_RIGHTSxREQUEST:   process_loc_get_rights(snac, pack); break;
      case SN_LOC_SETxUSERINFO:	    process_loc_set_info(snac, pack);   break;
      case SN_LOC_REQxUSERINFO:	    process_loc_get_info(snac, pack);	break;
      case SN_LOC_REQxUSERINFO2:    process_loc_get_info2(snac, pack);  break;
      
      default: DEBUG(10, ("Unknown location SNAC(0x2, %04X)\n", snac.subfamily));
               log_alarm_packet(0, pack);
   }
}


/**************************************************************************/
/* Client want to get user location info    				  */
/**************************************************************************/
void process_loc_get_info2(struct snac_header &snac, Packet &pack)
{
   struct online_user *user  = NULL;
   struct online_user *auser = NULL;
   unsigned long dmask = 0;
   unsigned long auin;

   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip)));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   pack >> dmask;
   auin  = read_buin(pack);
   auser = shm_get_user(auin);
   
   if (auser == NULL)
   {
      /* security risk - user can assume another invisible user presence */
      send_snac_error(snac.family, ERR_RECIPIENT_OFFLINE, snac.id, pack);
      return;
   }
   
   send_user_loc_info(snac, user, auser, dmask);
}


/**************************************************************************/
/* Client want to get user location info    				  */
/**************************************************************************/
void process_loc_get_info(struct snac_header &snac, Packet &pack)
{
   struct online_user *user  = NULL;
   struct online_user *auser = NULL;
   unsigned short type = 0;
   unsigned long dmask = 0;
   unsigned long auin;

   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip)));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   pack >> type;
   auin  = read_buin(pack);
   auser = shm_get_user(auin);
   
   if (auser == NULL)
   {
      /* security risk - user can assume another invisible user presence */
      send_snac_error(snac.family, ERR_RECIPIENT_OFFLINE, snac.id, pack);
      return;
   }
   
   switch (type)
   {
      case 0x0001: dmask = INFO_PROF; break;
      case 0x0002: dmask = INFO_EMPTY; break;
      case 0x0003: dmask = INFO_AWAY; break;
      case 0x0004: dmask = INFO_CAPS; break;
   }
   
   send_user_loc_info(snac, user, auser, dmask);
}


/**************************************************************************/
/* Client have sent its capabilitys CLSIDs to us			  */
/**************************************************************************/
void process_loc_set_info(struct snac_header &snac, Packet &pack)
{
   int i = 0;
   int i2 = 0;
   struct online_user *user;
   class tlv_chain_c tlv_chain;   
   class tlv_c *tlv, *tlv2;
   ffstring sn;

   int max_proflen = lp_v7_max_proflen();
   if (max_proflen > 1024) max_proflen = 1024;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip)));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   tlv_chain.read(pack);
   snprintf(sn, sizeof(ffstring)-1, "%lu", user->uin);

   /* Parse capabilities block */
   if ((tlv = tlv_chain.get(0x5)) != NULL)
   { 
      /* OK... we have tlv(5) ready */
      tlv->setup_aim();
      user->caps_num = tlv->size / 16;
   
      if (user->caps_num > MAX_CAPS) user->caps_num = MAX_CAPS;
   
      for (i=0; i<user->caps_num; i++)
      {
         for (i2=0; i2<16; i2++)
         {
            *tlv >> user->caps[i][i2];
         }
      }  

      DEBUG(100, ("User from %s sent %d CLSID caps\n", 
	         inet_ntoa(pack.from_ip), user->caps_num));
   }
   
   /* Parse profile block */
   if ((tlv2 = tlv_chain.get(0x2)) != NULL)
   { 
      if ((tlv = tlv_chain.get(0x1)) != NULL)
      {
         fstring  mime;
         cstring  data;
	 int datasize = tlv2->size;
	 int mimesize = tlv->size;
	 if (datasize > max_proflen) datasize = max_proflen;
	 if (mimesize > 128) mimesize = 128;
      
         DEBUG(50, ("Client %s sent profile loc-info (dsize=%d)\n", sn, datasize));

         /* tlv(1) - mime, tlv(2) - text */
         for (i=0; i<datasize; i++) *tlv2 >> data[i];
	 for (i=0; i<mimesize; i++) *tlv  >> mime[i];
	 mime[mimesize] = 0;
	
	 if (db_online_save_profile(sn, OPRF_PROFILE, mime, data, datasize) != 0)
	 {
	    DEBUG(50, ("Can't save profile data (sn=%s, dsize=%d)\n", sn, datasize));
	 }
      }
 
      if (tlv2->size == 0) db_online_save_profile(sn, OPRF_PROFILE, "", "", 0);
   }

   /* Parse away block */
   if ((tlv2 = tlv_chain.get(0x4)) != NULL)
   { 
      if ((tlv = tlv_chain.get(0x3)) != NULL)
      {
         fstring  mime;
         cstring  data;
	 int datasize = tlv2->size;
	 int mimesize = tlv->size;
	 if (datasize > max_proflen) datasize = max_proflen;
	 if (mimesize > 128) mimesize = 128;
      
         DEBUG(50, ("Client %s sent away loc-info (dsize=%d)\n", sn, datasize));

         /* tlv(1) - mime, tlv(2) - text */
         for (i=0; i<datasize; i++) *tlv2 >> data[i];
	 for (i=0; i<mimesize; i++) *tlv  >> mime[i];
	 mime[mimesize] = 0;
	
	 if (db_online_save_profile(sn, OPRF_AWAY, mime, data, datasize) != 0)
	 {
	    DEBUG(50, ("Can't save away data (sn=%s, dsize=%d)\n", sn, datasize));
	 }
      }
      
      if (tlv2->size == 0) db_online_save_profile(sn, OPRF_AWAY, "", "", 0);
   }
}


/**************************************************************************/
/* Client is acking location service limits				  */
/**************************************************************************/
void process_loc_get_rights(struct snac_header &snac, Packet &pack)
{
   struct online_user *user;
   int max_proflen = lp_v7_max_proflen();
   if (max_proflen > 1024) max_proflen = 1024;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s:%d sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip), pack.from_port));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();

   arpack << (unsigned short)SN_TYP_LOCATION
          << (unsigned short)SN_LOC_RIGHTSxRESPONSE
	  << (unsigned short)0x0000
	  << (unsigned  long)snac.id
	  
	  << (unsigned short)0x0001
	  << (unsigned short)sizeof(unsigned short)
	  << (unsigned short)max_proflen /* max profile length */
	  
	  << (unsigned short)0x0002
	  << (unsigned short)sizeof(unsigned short)
	  << (unsigned short)MAX_CAPS      /* max capabilities */
	  
	  << (unsigned short)0x0003
	  << (unsigned short)sizeof(unsigned short)
	  << (unsigned short)0x000A         /* mime max length */
	  
	  << (unsigned short)0x0004
	  << (unsigned short)0x0002
	  << (unsigned short)0x1000
	  
	  << (unsigned short)0x0005
	  << (unsigned short)0x0002
	  << (unsigned short)0x0080;

   arpack.flap_channel = 0x02;
   arpack.sock_evt     = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* SNAC(02,06) - client location info server reply			  */
/**************************************************************************/
void send_user_loc_info(struct snac_header &snac, struct online_user *user, 
                        struct online_user *auser, unsigned long datamask)
{
   char buin[32];
   int i,j;
   unsigned long uptime = timeToLong(time(NULL)) - auser->uptime;
   unsigned long dc_bcook = user->dc_cookie + auser->dc_cookie;
   fstring mime;
   cstring data;
         
   snprintf(buin, 31, "%lu", auser->uin);

   arpack.clearPacket();
   arpack.setup_aim();

   arpack << (unsigned short)SN_TYP_LOCATION
          << (unsigned short)SN_LOC_USERINFO
	  << (unsigned short)0x0000
	  << (unsigned  long)snac.id;
   
   /* now fixed data part: screenname, warn_level & 8 TLVs */
   arpack << (char)strlen(buin) << buin
          << (unsigned short)auser->warn_level
	  << (unsigned short)0x0005 /* tlv num in fixed part */

          << (unsigned short)0x0001 /* 1. user class */
	  << (unsigned short)sizeof(auser->uclass)
	  << (unsigned short)auser->uclass

          << (unsigned short)0x0006 /* 2. user ext status */
	  << (unsigned short)(sizeof(auser->estat)+sizeof(auser->status))
	  << (unsigned short)auser->estat
	  << (unsigned short)auser->status
	      
          << (unsigned short)0x0003 /* 3. user uptime value */
	  << (unsigned short)(sizeof(uptime))
	  << (unsigned  long)uptime

 	  << (unsigned short)0x0005 /* 4. account create time */
	  << (unsigned short)(sizeof(auser->crtime))
	  << (unsigned  long)auser->crtime;
	  
   if ((auser->protocol == V3_PROTO) && (!lp_v7_direct_v3_connect()) ||
      ((auser->protocol == V5_PROTO) && (!lp_v7_direct_v5_connect())))
   {
      /* disable dc with v3/v5 clients */
      arpack << (unsigned short)0x000c /* 5. dc information */
  	     << (unsigned short)0x0025
	     << (unsigned  long)0x00000000
  	     << (unsigned  long)0x00000000
	     << (char)0x00
	     << (unsigned short)auser->tcpver
	     << (unsigned  long)dc_bcook
  	     << (unsigned  long)auser->web_port
	     << (unsigned  long)auser->cli_futures
	     << (unsigned  long)auser->info_utime
	     << (unsigned  long)auser->more_utime
	     << (unsigned  long)auser->stat_utime
	     << (unsigned short)0x0000;
   }
   else
   {
      arpack << (unsigned short)0x000c /* 5. dc information */
  	     << (unsigned short)0x0025;

      if (auser->dc_perms != 2)
      {
         arpack << (unsigned  long)ipToIcq2(auser->int_ip)
  	        << (unsigned  long)auser->tcp_port
	        << (char)auser->dc_type
	        << (unsigned short)auser->tcpver
	        << (unsigned  long)dc_bcook;
      }
      else
      {
         arpack << (unsigned  long)0x00000000
		<< (unsigned  long)0x00000000
		<< (char)auser->dc_type
		<< (unsigned short)auser->tcpver
		<< (unsigned  long)0x00000000;
      }

      arpack << (unsigned  long)auser->web_port
	     << (unsigned  long)auser->cli_futures
	     << (unsigned  long)auser->info_utime
	     << (unsigned  long)auser->more_utime
	     << (unsigned  long)auser->stat_utime
	     << (unsigned short)0x0000;
   }

   /* here we should check mask and put requested data into packet */
   /* user profile info */
   int datasize = 0;
   if (datamask & INFO_PROF)
   {
      datasize = db_online_get_profile(buin, OPRF_PROFILE, mime, sizeof(mime)-1, 
                                       data, sizeof(data)-1);

      if (datasize > 0)
      {
         arpack << (unsigned short)0x0001
	        << (unsigned short)strlen(mime)
		<< mime
	        
		<< (unsigned short)0x0002
		<< (unsigned short)datasize;
	
	 /* data is not always ascii string ! */
         for (i=0;i<datasize;i++) arpack << (char)data[i];
      }
      else
      {
         /* empty profile info string */
	 strncpy(mime, "text/aolrtf; charset=\"us-ascii\"", sizeof(mime)-1);

         arpack << (unsigned short)0x0001
	        << (unsigned short)strlen(mime)
		<< mime
	        
		<< (unsigned short)0x0002
		<< (unsigned short)0x0001
		<< (char)0x00;	 
      }
   }
   
   /* user away info */
   if (datamask & INFO_AWAY)
   {
      datasize = db_online_get_profile(buin, OPRF_AWAY, mime, sizeof(mime)-1, 
                                       data, sizeof(data)-1);

      if (datasize > 0)
      {
         arpack << (unsigned short)0x0003
	        << (unsigned short)strlen(mime)
		<< mime
	        
		<< (unsigned short)0x0004
		<< (unsigned short)datasize;
	
	 /* data is not always ascii string ! */
         for (i=0;i<datasize;i++) arpack << (char)data[i];
      }
      else
      {
         /* empty away info string */
	 strncpy(mime, "text/aolrtf; charset=\"us-ascii\"", sizeof(mime)-1);

         arpack << (unsigned short)0x0003
	        << (unsigned short)strlen(mime)
		<< mime
	        
		<< (unsigned short)0x0004
		<< (unsigned short)0x0001
		<< (char)0x00;	 
      }
   }
   
   /* capability block requested by user */
   if ((datamask & INFO_CAPS) && (auser->caps_num))
   {
      arpack << (unsigned short)0x000D
	     << (unsigned short)(auser->caps_num*16);

      for (i=0;i<auser->caps_num;i++)
      {
         for (j=0;j<16;j++) arpack << (char)auser->caps[i][j];
      }
   }

   if (datamask & INFO_CERT) { /* certs ? i know nothing about certs */ };

   /* now we can send data to user */
   arpack.sock_hdl     = user->sock_hdl;
   arpack.sock_rnd     = user->sock_rnd;
   arpack.from_ip      = user->ip;
   arpack.sock_evt     = SOCK_DATA;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 2;
   arpack.from_port    = 0;
   tcp_writeback_packet(arpack);
   log_alarm_packet(100,arpack);
}


