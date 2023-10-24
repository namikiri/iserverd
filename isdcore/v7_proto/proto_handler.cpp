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
/* Handle AIM/OSCAR transport user packets				  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

extern unsigned short type_limits[SSI_TYPES_NUM];

/**************************************************************************/
/* handle AOL AIM protocol (also known as V7 for icq2000/2001)    	  */
/**************************************************************************/
void handle_aim_proto(Packet &pack)
{
   unsigned short sn_type;
   unsigned short sn_subtype;
   struct online_user *user;
   
   /* it is new connection signal packet, that doesn't contain data */
   if (pack.sock_evt == SOCK_NEW)
   {
      send_connection_accepted(pack);
      return;
   }
   
   /* this packet arrived on socket disconnection */   
   if (pack.sock_evt == SOCK_TERM) 
   {
      /* delete user cookie and online_records */
      if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) != NULL)
      {
         unsigned long uptime = (unsigned long)(time(NULL) - longToTime(user->uptime));
	 if (uptime > 0x7FFFFFFF) uptime = 0; /* time paradox ugly fix :) */
	 
      	 LOG_USR(0, ("User %lu moved offline. Online time was: %lu seconds\n", user->uin, uptime));
         move_user_offline(user->uin);
	 return;
      }
   }
   
   if (pack.sock_evt != SOCK_DATA) return; /* internal error ? */

   pack.reset();                      
   pack.network_order(); 
   pack >> sn_type >> sn_subtype;
   
   /* Check if we have auth packet from channel 1 */
   if ((pack.flap_channel == 1) && 
       (sn_type == 0x0000) && 
       (sn_subtype == 0x0001) && 
       (pack.sizeVal > 4)) 
   {
      process_authorize_packet(pack);
      return;
   }
   
   /* This is flap version from client */
   if ((pack.flap_channel == 1) &&
       (sn_type == 0x0000) && 
       (sn_subtype == 0x0001) && 
       (pack.sizeVal == 4)) 
   {
      DEBUG(100, ("FLAP version from client\n"));
      return;
   }       
   
   /* check if we have snac packet (channel=2, sn_type!=0)*/   
   if ((pack.flap_channel == 2) && (sn_type != 0x0000))
   {
      process_snac_packet(pack);
      return;
   }
    
   /* Check if it is non-snac packet from channel 2 */
   /* probably it contain TLV(6) with auth cookie   */
   if ((pack.flap_channel == 2) && (sn_type == 0x0000) &&
       (sn_subtype == 0x0001))
   {
      process_cookie_packet(pack);
      return;
   }
   
   /* I don't know this packet type... :( */
   DEBUG(10, ("Unknown flap data from client (ip=%s)\n", inet_ntoa(pack.from_ip)));
   log_unknown_packet(pack);
   close_connection(pack);
}


/**************************************************************************/
/* Parse client auth request (we should give him a cookie and BOS addr)   */
/**************************************************************************/
void process_authorize_packet(Packet &pack)
{
   char screen_name[64];
   char password[64];
   char country[16], language[16];
   char server_cookie[300];
   char cli_profile[255];
   char *bos_addr = cli_profile;
   struct online_user *user;
   
   unsigned long  ljunk;
   unsigned short tlv_type;
   unsigned short passlen    = 0;
   unsigned short ver_major  = 0;
   unsigned short ver_minor  = 0;
   unsigned short ver_lesser = 0;
   unsigned short ver_build  = 0;
   unsigned  long ver_dunno  = 0;
   unsigned short sjunk;
   
   int lookup_res;
   struct login_user_info userinfo;
   unsigned long uin_num;
   
   /* TLVs can be in another sequence order so we should handle this */
   pack.reset();
   pack.network_order(); pack >> ljunk;
   
   strncpy(screen_name, "", 2);
   strncpy(password, "", 2);
   strncpy(country, "", 2);
   strncpy(language, "", 2);
   strncpy(cli_profile, "", 2);
   
   pack >> tlv_type;
   
   /* we using the same port for authorizer and BOS... so we should check */
   /* if it is a BOS signon packet with cookie placed in TLV(6) */
   if (tlv_type == 6) { process_cookie_packet(pack); return; }
   
   /* all stuff ready - we should read all available TLVs from it */
   /* this code was written before tlv class... but it works...   */
   while (pack.nextData < (pack.buff+pack.sizeVal))
   {
      /* now we can read TLV, determine its type and get its data */
      switch (tlv_type)
      {
         case 0x01: { tlv_read_string(pack, screen_name, 62);      break; }   
	 case 0x02: { passlen=tlv_read_string(pack, password, 62); break; }      
	 case 0x03: { tlv_read_string(pack, cli_profile, 253);     break; }   
	 case 0x09: { tlv_read_short (pack, sjunk); 	           break; }	
	 case 0x0E: { tlv_read_string(pack, country, 14);          break; }	
	 case 0x0F: { tlv_read_string(pack, language, 14);         break; }      
	 case 0x16: { tlv_read_short (pack, sjunk);                break; }
	 case 0x17: { tlv_read_short (pack, ver_major);            break; }
	 case 0x18: { tlv_read_short (pack, ver_minor);            break; }
	 case 0x19: { tlv_read_short (pack, ver_lesser);           break; }
	 case 0x1A: { tlv_read_short (pack, ver_build);            break; }
	 case 0x14: { tlv_read_long  (pack, ver_dunno);            break; }
	 default: 
	             DEBUG(10, ("Unknown TLV in auth packet (%d)\n", tlv_type));
		     log_unknown_packet(pack);
		     return;
      }
      
      pack >> tlv_type;
   } 
   
   DEBUG(100, ("Roasted password length=%d\n", passlen));
   decrypt_password(password, passlen);
   
   /* ok, we just have read all TLV... we need only scr_name, pass & profile */
   if (strlen(screen_name) == 0) 
   { 
      DEBUG(10, ("AIM: No screen name. Closing connection (%s)\n", 
                  inet_ntoa(pack.from_ip)));
      close_connection(pack); 
      return; 
   }
   
   if (strlen(cli_profile) == 0) 
   { 
      DEBUG(10, ("AIM: No cli profile. Closing connection (%s at %s)\n", 
                  screen_name, inet_ntoa(pack.from_ip)));
      close_connection(pack); 
      return; 
   }

   if (strlen(password) == 0)    
   { 
      DEBUG(10, ("AIM: No password. Closing connection (%s at %s)\n", 
                  screen_name, inet_ntoa(pack.from_ip)));
      close_connection(pack); 
      return; 
   } /* not sure */   

   /* now we have all nessesary data so we should check pass and other info */
   uin_num = atoul(screen_name);
   if (uin_num == 0)
   {
      /* well, it is an aim client - so it's screen name is not number */
      send_authorize_fail(pack, "", screen_name, AUTH_SERV_FAIL);
      return;
   }
   
   lookup_res = db_users_lookup(uin_num, userinfo);
   if (lookup_res == -2)
   {
      LOG_ALARM(0, ("SQL error in database - we shouldn't accept login\n"));
      send_authorize_fail(pack, "", screen_name, AUTH_SERV_FAIL);
      return;
   }
      
   if (lookup_res == -1)
   {
      LOG_USR(0, ("Can't find user %lu in database.\n", uin_num));
      send_authorize_fail(pack, "", screen_name, AUTH_ERR_UIN);
      return;
   }
      
   if (!strcsequal(password, userinfo.passwd))
   {
      LOG_USR(0, ("User %lu trying login with wrong password.\n", uin_num));
      send_authorize_fail(pack, "", screen_name, AUTH_ERR_PASS2);
      return;
   }

   if ((userinfo.ip_addr != 0) &&
       (lp_restrict2luip()) &&
       (userinfo.ip_addr != ipToIcq(pack.from_ip)))
   {
      
      LOG_USR(0, ("User %lu (v7) trying login from wrong ip-addres.\n", uin_num));
      send_authorize_fail(pack, "You can't login from this IP address", 
                          screen_name, AUTH_ERR_PASS2);
      return;
   }

   if (userinfo.disabled == 1) 
   {
      LOG_USR(0, ("User %lu trying login but his account locked.\n", uin_num));

      /* i don't know yet if icq2k+ cah handle server string-type error  */
      /* messages so I simply kick off client without any explanations   */      
      send_authorize_fail(pack, "Account locked by administrator", 
    		          screen_name, AUTH_SERV_FAIL);
      return;
   }

   /* well, now we should check if there is another user connected...      */
   /* i have no idea how to kick off v3/v5 clients                         */
   if ((user = shm_get_user(atoul(screen_name))) != NULL)
   {
      /* Another client with this uin connected */
      if ((lp_v7_accept_concurent()) && 
          ((user->protocol == V7_PROTO)))
      {
         /* Kick off user from server and allow this client to login */
	 send_user_disconnect(*user, 0x01, "New client with same uin connected");
	 move_user_offline(user->uin);
      }
      else
      {
         send_authorize_fail(pack, "Allready connected", screen_name, AUTH_SERV_FAIL);
	 return;
      }
   }

   db_cookie_delete(userinfo.uin);  
   generate_cookie(pack, server_cookie, screen_name);
   db_cookie_insert(userinfo.uin, server_cookie, TYPE_COOKIE);
   get_bos_server(bos_addr);
   
   send_auth_cookie(pack, screen_name, bos_addr, server_cookie, strlen(server_cookie));
   LOG_SYS(50, ("User %s (%s) redirected to %s\n", 
                screen_name, inet_ntoa(pack.from_ip), bos_addr));

   /* after:                                            */
   /* 1. Client closes connection by channel 4 packet   */
   /* 2. Connection closing on timeout                  */
}


/**************************************************************************/
/* Disconnect specified user 						  */
/**************************************************************************/
void v7_disconnect_user(struct online_user *user)
{
  close_connection(user);
}

/**************************************************************************/
/* Parse client cookie packet 						  */
/**************************************************************************/
void process_cookie_packet(Packet &pack)
{
   unsigned short tlv_type;
   unsigned long  uin, ljunk;
   struct online_user user;
   struct full_user_info tuser;

   pack.reset();
   pack.setup_aim();
   
   /* first we should extract cookie from packet and then compare it with */
   /* cookies, stored in database, cookie can be smaller then 256 bytes   */
   pack >> ljunk >> tlv_type;
   
   /* cookie len = 256 bytes + zero */
   tlv_read_string(pack, tempst, 257);
   if (strlen(tempst) == 0) 
   { 
      LOG_SYS(0, ("Auth: client from %s sent invalid authorization cookie\n", 
                   inet_ntoa(pack.from_ip)));
		   
      close_connection(pack); return; 
   }
   
   uin = db_cookie_check(tempst);
   db_cookie_delete(uin);

   if (uin == 0)
   {
      LOG_SYS(0, ("Authorization cookie for client from %s doesn't exist\n", 
                  inet_ntoa(pack.from_ip)));
      
      close_connection(pack);
      return;
   }
   
   if (uin == 1)
   {
      LOG_SYS(0, ("Can't check cookie because of database error.\n"));
      /* TODO: send a packet to notify client about problem */
      close_connection(pack);
      return;
   }
   
   if (db_users_lookup(uin, tuser) < 0)
   { 
      /* we can't found user info db/internal error ? */
      LOG_SYS(0, ("Can't find client %lu information. Closing connection...\n", uin));
      close_connection(pack);
      return;
   }
   
   DEBUG(10, ("Client with uin=%lu have sent a cookie. Signon complete...\n", uin));
   
   /* We'll add user to online db - in non-active mode 
      (dinfo, caps and other stuff is not ready yet)   */

   bzero((void *)&user, sizeof(user));
   
   user.uin         = uin;
   user.usid        = rand();
   user.ip          = pack.from_ip;
   user.uptime      = longToTime(time(NULL));
   user.lutime      = longToTime(time(NULL));
   user.crtime      = tuser.cr_date;
   user.uclass      = CLASS_FREE | CLASS_ICQ;
   user.sock_hdl    = pack.sock_hdl;
   user.sock_rnd    = pack.sock_rnd;
   user.protocol    = V7_PROTO;
   user.dc_perms    = tuser.iphide;
   user.servseq2    = (rand() % 0x20000000) + 0x80000001;

   if (user.status != 0) user.uclass = user.uclass | CLASS_AWAY;

   /* channel 1 limits */
   user.mopt[MCH1].max_msglen   = lp_v7_dmax_msgsize();
   user.mopt[MCH1].max_sevil    = lp_v7_dmax_sevil();
   user.mopt[MCH1].max_revil    = lp_v7_dmax_revil();
   user.mopt[MCH1].min_interval = lp_v7_dmin_msg_interval();
   user.mopt[MCH1].icbm_flags   = ICBM_FLG_BASE | ICBM_FLG_MISSED;

   /* channel 2 limits */
   user.mopt[MCH2].max_msglen   = lp_v7_dmax_msgsize();
   user.mopt[MCH2].max_sevil    = lp_v7_dmax_sevil();
   user.mopt[MCH2].max_revil    = lp_v7_dmax_revil();
   user.mopt[MCH2].min_interval = lp_v7_dmin_msg_interval();
   user.mopt[MCH2].icbm_flags   = ICBM_FLG_BASE | ICBM_FLG_MISSED;

   /* channel 3 limits */
   user.mopt[MCH4].max_msglen   = lp_v7_dmax_msgsize();
   user.mopt[MCH4].max_sevil    = lp_v7_dmax_sevil();
   user.mopt[MCH4].max_revil    = lp_v7_dmax_revil();
   user.mopt[MCH4].min_interval = lp_v7_dmin_msg_interval();
   user.mopt[MCH4].icbm_flags   = ICBM_FLG_BASE | ICBM_FLG_MISSED;

   if (db_online_insert(user) != 0)
   {
      send_session_fail(pack, "Server users table full...Try again later", 0);
      close_connection(pack);
      return;
   }

   /* now we can tell client what snac families we understand */
   LOG_USR(20, ("User %lu from %s signed on (v7).\n", 
                 user.uin, inet_ntoa(user.ip)));

   send_srv_families(pack, user);
}


/**************************************************************************/
/* Init V7 transport protocol  						  */
/**************************************************************************/
void v7_proto_init()
{
   /* init services table   */
   init_aim_services();		
   
   /* init ssi limits table */
   type_limits[0x00] = lp_v7_max_contact_size();
   type_limits[0x01] = lp_v7_max_ssi_groups();
   type_limits[0x02] = lp_v7_max_visible_size();
   type_limits[0x03] = lp_v7_max_invisible_size();
   type_limits[0x04] = 0x0001; /* visible/invisible perms */
   type_limits[0x05] = 0x0001; /* presence info item      */
   type_limits[0x06] = 0x0032;
   type_limits[0x07] = 0x0000;
   type_limits[0x08] = 0x0000;
   type_limits[0x09] = 0x0003; /* shortcut bar items (?)  */
   type_limits[0x0A] = 0x0000;
   type_limits[0x0B] = 0x0000;
   type_limits[0x0C] = 0x0000;
   type_limits[0x0D] = 0x0080;
   type_limits[0x0E] = lp_v7_max_ssi_ignore();
   type_limits[0x0F] = 0x0014; /* LastUpdateDate item (!) */
   type_limits[0x10] = lp_v7_max_ssi_nonicq();
   type_limits[0x11] = 0x0001;
   type_limits[0x12] = 0x0000;
   type_limits[0x13] = 0x0001; /* Roster import time item */
   type_limits[0x14] = lp_v7_max_ssi_avatars();
   type_limits[0x15] = 0x0001;
   type_limits[0x16] = 0x0028; /* wtf? */
   type_limits[0x17] = 0x0000;
   type_limits[0x18] = 0x0000;
   type_limits[0x19] = 0x0000;
   
}

