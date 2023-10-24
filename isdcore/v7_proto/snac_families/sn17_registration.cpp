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
/* This module contain snac creating/processing/authorization functions   */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* ICQ registration services SNAC family packets handler		  */
/**************************************************************************/
void process_snac_registration(struct snac_header &snac, Packet &pack)
{
   switch (snac.subfamily)
   {
      case SN_IES_REQxNEWxUIN:   process_ies_uin_req(snac, pack);  break;
      case SN_IES_AUTHxREQUEST:  process_ies_auth_req(snac, pack); break;
      case SN_IES_AUTHxLOGIN:	 process_ies_auth_login(snac, pack); break;
      case SN_IES_SECURIDxREPLY: process_ies_securid(snac,pack); break;
      case SN_IES_REQxIMAGE: process_ies_image_req(snac,pack); break;
      
      default: DEBUG(10, ("Unknown icq ies SNAC(0x17, %04X)\n", snac.subfamily));
   }
}


/**************************************************************************/
/* Parse client md5 login request 					  */
/**************************************************************************/
void process_ies_auth_login(struct snac_header &snac, Packet &pack)
{
   class tlv_chain_c tlv_chain;
   class tlv_c *tlv;
   char disable_blm = 0;
   char md5_digest[16];
   unsigned short md5_method = false;
   char screen_name[64];
   char country[16], language[16];
   char server_cookie[300];
   char cli_profile[255];
   char *bos_addr = cli_profile;
   struct online_user *user;
   
   unsigned short ver_major  = 0;
   unsigned short ver_minor  = 0;
   unsigned short ver_lesser = 0;
   unsigned short ver_build  = 0;
   unsigned  long ver_dunno  = 0;
   unsigned short used;
   
   int lookup_res, i;
   struct login_user_info userinfo;
   unsigned long uin_num;
   
   /* TLVs can be in another sequence order so we should handle this */
   strncpy(screen_name, "", 2);
   strncpy(country, "", 2);
   strncpy(language, "", 2);
   strncpy(cli_profile, "", 2);

   /* now we can read tlv chain from user packet */   
   tlv_chain.read(pack);

   /* check for screen_name - tlv 0x01 */
   if (((tlv = tlv_chain.get(0x01)) == NULL) ||
       (!v7_extract_string(screen_name, *tlv, 63)))
   { 
      DEBUG(50, ("auth_md5: bad/empty screen name. closing connection\n"));
      
      send_authmd5_fail(pack, AUTH_INVALID_PACKET, "");
      close_connection(pack); 
      return; 
   }

   /* check for client string - tlv 0x03 */
   if (((tlv = tlv_chain.get(0x03)) == NULL) ||
       (!v7_extract_string(cli_profile, *tlv, 253)))
   { 
      DEBUG(50, ("auth_md5: bad/empty client id string. closing connection\n"));

      send_authmd5_fail(pack, AUTH_INVALID_PACKET, "");
      close_connection(pack); 
      return; 
   }

   /* check for password digest - tlv 0x25 */
   if (((tlv = tlv_chain.get(0x25)) == NULL) ||
       (tlv->size != 16))
   { 
      DEBUG(50, ("auth_md5: bad/empty password md5 digest. closing connection\n"));

      send_authmd5_fail(pack, AUTH_INVALID_SECUREID, "");
      close_connection(pack); 
      return; 
   }

   for (i=0;i<16;i++) *tlv >> md5_digest[i];
   
   /* check for client md5 NewHash method - tlv 0x4c */
   if ((tlv = tlv_chain.get(0x4c)) != NULL)
   { 
      md5_method = true;
   }

   /* check for client language - tlv 0x0f */
   if (((tlv = tlv_chain.get(0x0f)) == NULL) ||
       (!v7_extract_string(language, *tlv, 15)))
   { 
      DEBUG(50, ("auth_md5: bad/empty client language. closing connection\n"));

      send_authmd5_fail(pack, AUTH_INVALID_PACKET, "");
      close_connection(pack); 
      return; 
   }

   /* check for client country - tlv 0x0e */
   if (((tlv = tlv_chain.get(0x0e)) == NULL) ||
       (!v7_extract_string(country, *tlv, 15)))
   { 
      DEBUG(50, ("auth_md5: bad/empty client country. closing connection\n"));

      send_authmd5_fail(pack, AUTH_INVALID_PACKET, "");
      close_connection(pack); 
      return; 
   }

   /* check for client major version - tlv 0x17 */
   if ((tlv = tlv_chain.get(0x17)) != NULL)
   { 
      *tlv >> ver_major;
   }
   
   /* check for client minor version - tlv 0x0e */
   if ((tlv = tlv_chain.get(0x18)) != NULL)
   { 
      *tlv >> ver_minor;
   }

   /* check for client lesser version - tlv 0x19 */
   if ((tlv = tlv_chain.get(0x19)) != NULL)
   { 
      *tlv >> ver_lesser;
   }

   /* check for client build number - tlv 0x1A */
   if ((tlv = tlv_chain.get(0x1A)) != NULL)
   { 
      *tlv >> ver_build;
   }
   
   /* check for client distribution number - tlv 0x14 */
   if ((tlv = tlv_chain.get(0x14)) != NULL)
   { 
      *tlv >> ver_dunno;
   }

   /* check if we should disable client-side buddy lists */
   /* if so - client should use SSC only - tlv 0x4a      */
   if ((tlv = tlv_chain.get(0x4A)) != NULL)
   { 
      *tlv >> disable_blm;
   }
   
   DEBUG(100, ("User's screen name is \"%s\"\n", screen_name));
   uin_num = atoul(screen_name);
   lookup_res = db_users_lookup(uin_num, userinfo);
   
   if (lookup_res == -2)
   {
      LOG_ALARM(0, ("SQL error in database - we shouldn't accept login\n"));
      send_authmd5_fail(pack, AUTH_SERV_FAIL, "");
      return;
   }
      
   if (lookup_res == -1)
   {
      LOG_USR(0, ("Can't find user %lu in database.\n", uin_num));
      send_authmd5_fail(pack, AUTH_ERR_UIN2, "");
      return;
   }

   /* now time to check password */
   if (db_cookie_get(uin_num, server_cookie, used, TYPE_AUTH) != 0)
   {
      LOG_USR(0, ("User %lu password crypt key doesn't exist.\n", uin_num));
      send_authmd5_fail(pack, AUTH_TEMP_DOWN, "");
      return;
   }

   if (!aim_check_digest(md5_digest, md5_method, userinfo.passwd, server_cookie))
   {
      LOG_USR(0, ("User %lu trying login with wrong password.\n", uin_num));
      send_authmd5_fail(pack, AUTH_ERR_PASS2, "");
      return;   
   }

   if (userinfo.disabled == 1) 
   {
      LOG_USR(0, ("User %lu trying login but his account locked.\n", uin_num));
      send_authmd5_fail(pack, AUTH_ACCOUNT_SUSPENDED, "");
      return;
   }

   /* well, now we should check if there is another user connected...      */
   /* i have no idea how to kick off v3/v5 clients                         */
   if ((user = shm_get_user(atoul(screen_name))) != NULL)
   {
      /* wtf ? another client with this uin connected */
      if ((lp_v7_accept_concurent()) && 
          ((user->protocol == V7_PROTO)))
      {
         /* Kick off user from server and allow this client to login */
	 send_user_disconnect(*user, 0x01, "New client with same uin connected");
	 move_user_offline(user->uin);
      }
      else
      {
	 send_authmd5_fail(pack, AUTH_TEMP_DOWN, "");
	 return;
      }
   }

   db_cookie_delete(userinfo.uin);  
   generate_cookie(pack, server_cookie, screen_name);
   db_cookie_insert(userinfo.uin, server_cookie, TYPE_COOKIE);
   get_bos_server(bos_addr);

   send_authmd5_cookie(pack, screen_name, bos_addr, server_cookie, strlen(server_cookie));
   LOG_SYS(50, ("User %s (%s) redirected to %s (md5)\n", 
                 screen_name, inet_ntoa(pack.from_ip), bos_addr));

   /* after:                                            */
   /* 1. Client closes connection by channel 4 packet   */
   /* 2. Connection closing on timeout                  */
}



/**************************************************************************/
/* Handle auth request from new client (md5 login sequence)		  */
/**************************************************************************/
void process_ies_auth_req(struct snac_header &snac, Packet &pack)
{
   class tlv_chain_c tlv_chain;
   class tlv_c *tlv;
   char scrname[32];
   char authkey[256];
   unsigned long uin;
   int lookup_res;
   struct login_user_info userinfo;

   DEBUG(10, ("Client trying to login via md5 sequence...\n"));

   tlv_chain.read(pack);
   
   if ((tlv = tlv_chain.get(0x1)) == NULL)
   {
      DEBUG(10, ("ERROR: Mailformed md5 auth request from %s (tlv(0x1))\n", 
                inet_ntoa(pack.from_ip)));
		
      close_connection(pack);
      return;
   }

   if (!v7_extract_string(scrname, *tlv, 31))
   {
      DEBUG(10, ("ERROR: Too big screenname from %s (tlv(0x1))\n", 
                inet_ntoa(pack.from_ip)));
      
      close_connection(pack);
      return;   
   }

   uin = atoul(scrname);

   /* OK, now we should put random keystring into packet and send it to */
   /* client. Client use md5 routines to append this value to its pass  */
   /* Also we should add it to database */      
   
   if (uin > 0)
   {    
      lookup_res = db_users_lookup(uin, userinfo);
   
      if (lookup_res == -2)
      {
         LOG_ALARM(0, ("SQL error in database - we shouldn't accept login\n"));
	 send_authmd5_fail(pack, AUTH_TEMP_DOWN, "");
         return;
      }
      
      if (lookup_res == -1)
      {
         LOG_USR(0, ("Can't find user %lu in database. Auth_md5 denied.\n", uin));
	 send_authmd5_fail(pack, AUTH_ERR_UIN2, "");
         return;
      }

      generate_key(authkey);
      db_cookie_delete(uin);
      db_cookie_insert(uin, authkey, TYPE_AUTH);
      v7_send_authkey(pack, authkey);
   }
   else
   {
      DEBUG(10, ("ERROR: Wrong screenname (\"%s\") from %s\n",
                 scrname, inet_ntoa(pack.from_ip)));
		 
      send_authmd5_fail(pack, AUTH_ERR_PASS2, "");
      close_connection(pack);
   }
}


/**************************************************************************/
/* Handle get_new_uin request 						  */
/**************************************************************************/
void process_ies_uin_req(struct snac_header &snac, Packet &pack)
{
   unsigned long  ljunk, lunk1, req_cookie, temp;
   unsigned short sjunk, cli_version;
   class tlv_chain_c tlv_chain;
   class tlv_c *tlv;
   char password[32];
   struct full_user_info new_user;

   PGresult *res;
   fstring dbcomm_str;
   char regcode[9],text_etalon[9];
  
   /* Well, this is registration SNAC so we shouldn't */
   /* check if user authorized to perform operation   */

   LOG_SYS(0, ("Client from %s want new uin... Registration started...\n", 
                inet_ntoa(pack.from_ip)));

   tlv_chain.read(pack);   
   
   if ((tlv = tlv_chain.get(0x1)) == NULL)
   {
      DEBUG(10, ("ERROR: Mailformed request_new_uin packet from %s (tlv(0x1))\n", 
                  inet_ntoa(pack.from_ip)));

      close_connection(pack);
      return;
   }

   DEBUG(10, ("TLV 0x01 exists... All is ok...\n"));
   tlv->intel_order();
   tlv->null_terminated();
   
   /* Here we should extract unknown value */
   *tlv >> ljunk 
        >> lunk1
	>> ljunk 
	>> ljunk
	>> req_cookie 
	>> ljunk
	>> ljunk 
	>> ljunk
	>> ljunk 
	>> ljunk;
	
   /* now password */
   if (!v7_extract_string(password, *tlv, 31, "reg password", pack))
   {
      /* password problem... exiting... */
      return;
   }

   /* and client version number */   
   *tlv >> ljunk
        >> sjunk
	>> cli_version;
	
   if ((tlv = tlv_chain.get(0x9)) != NULL)
   {
#ifdef HAVE_GD
      v7_extract_string(regcode, *tlv, 8);
      bzero(text_etalon, 9);
      slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
            "SELECT code FROM Regimage_Codes WHERE iadr=%lu", ipToIcq(pack.from_ip));
	    
      res = PQexec(users_dbconn, dbcomm_str);
      if (PQresultStatus(res) != PGRES_TUPLES_OK)
      {
	 handle_database_error(res, "[REGCODE LOOKUP]");
      }
      else
      {
	  if (PQntuples(res) == 1)
	  {
	     strncpy(text_etalon, PQgetvalue(res, 0, 0), sizeof(text_etalon)-1);	 
	  }
      }

      PQclear(res);

      slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
            "DELETE FROM Regimage_Codes WHERE iadr=%lu", ipToIcq(pack.from_ip));
	    
      res = PQexec(users_dbconn, dbcomm_str);
      
      PQclear(res);

      for (int c=0;c<8;c++)
      {
	 if (text_etalon[c]!=regcode[c])
	 {
	    v7_send_reg_refused(pack, req_cookie);
	 }
      }
#endif      
   } 
        
   /* V7 supports automatic registration _ONLY_ */
   if (lp_v7_registration_enabled())
   {
      memset(&new_user, 0, sizeof(new_user));
      strncpy(new_user.passwd, password, 31);
      
      new_user.uin           = db_users_new_uin();
      new_user.can_broadcast = 0;
      new_user.wocup         = -1;
      new_user.ch_password   = 0;
      new_user.disabled      = 0;
      new_user.cr_date       = time(NULL);
      new_user.lastlogin     = time(NULL);
      new_user.ip_addr       = ipToIcq(pack.from_ip); 
   
      /* check for new_users_table */
      if (new_users_table_exist())
      {
         /*if exist we need to ajust uin number */
         temp = db_users_new_uin2();
         if (new_user.uin < temp) new_user.uin = temp;   
      } 
                           
      db_users_add_user(new_user);
      LOG_USR(0, ("Autoregistration: User from (%s) now have uin=%lu\n", 
                   inet_ntoa(pack.from_ip), new_user.uin));
         
      v7_send_new_uin(pack, new_user.uin, req_cookie);
	 
      send_event2ap(papack, ACT_REGISTR, new_user.uin, 0,
                    ipToIcq(pack.from_ip), 0, longToTime(time(NULL)), "");

   }
   else
   {
      LOG_USR(0, ("Autoregistration disabled. Request from %s refused\n",
                   inet_ntoa(pack.from_ip)));
      v7_send_reg_refused(pack, req_cookie);
   }
}


/**************************************************************************/
/* Send new uin to user 						  */
/**************************************************************************/
void v7_send_new_uin(Packet &pack, unsigned long new_uin, 
                     unsigned long req_cookie)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_REGISTRATION
          << (unsigned short)SN_IES_SRVxNEWxUIN
	  << (unsigned short)0x0000
	  << (unsigned  long)0x00000000;

   reply_pack.clearPacket();
   reply_pack.intel_order();
   reply_pack.null_terminated(); 
   
   reply_pack << (unsigned  long)0x00000000  /* for uin (unused) */
	      << (unsigned short)0x002d      /* subcommand       */
	      << (unsigned short)0x0003	     /* sequence number  */
	      
	      << (unsigned  long)pack.from_port
	      << (unsigned  long)ipToIcq(pack.from_ip)
	      << (unsigned  long)0x04000000
	      << (unsigned  long)req_cookie
	      << (unsigned  long)0x00000000
	      << (unsigned  long)0x00000000
	      << (unsigned  long)0x00000000
	      << (unsigned  long)0x00000000
	      << (unsigned  long)new_uin
	      << (unsigned  long)req_cookie;
	      
   /* now time to put data into arpack */
   arpack << (unsigned short)0x0001
          << (unsigned short)(reply_pack.size() + sizeof(unsigned short));
	  
   /* tlv data chunk size in intel_order */
   arpack.intel_order();
   arpack << (unsigned short)(reply_pack.size());
   
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending new uin (%lu) meta-reply to user\n", new_uin));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
   
}


/**************************************************************************/
/* Send registration_refused notice to user				  */
/**************************************************************************/
void v7_send_reg_refused(Packet &pack, unsigned long req_cookie)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_REGISTRATION
          << (unsigned short)SN_IES_ERROR
	  << (unsigned short)0x0000
	  << (unsigned  long)0x00000000
	  << (unsigned short)0x0005;

   reply_pack.clearPacket();
   reply_pack.setup_aim(); 
   
   reply_pack << (unsigned  long)0x00000000
	      << (unsigned short)0x0000
	      << (unsigned  long)req_cookie
	      << (unsigned  long)pack.from_port
	      << (unsigned  long)ipToIcq(pack.from_ip)
	      << (unsigned  long)0x00000004
	      << (unsigned  long)req_cookie
	      << (unsigned  long)0x400464F8;
	      
   /* now time to put data into arpack */
   arpack << (unsigned short)0x0021
          << (unsigned short)reply_pack.size();
	  
   /* requested info */
   memcpy(arpack.nextData, reply_pack.buff, reply_pack.size());
   arpack.sizeVal += reply_pack.size();
   
   DEBUG(100, ("Sending registration refused reply to user from %s\n", 
               inet_ntoa(pack.from_ip)));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
      
}


/**************************************************************************/
/* Send auth key to user						  */
/**************************************************************************/
void v7_send_authkey(Packet &pack, char *authkey)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_REGISTRATION
          << (unsigned short)SN_IES_AUTHxKEY
	  << (unsigned short)0x0000
	  << (unsigned  long)0x00000000;
	  
   arpack << (unsigned short)strlen(authkey)
	  << authkey;
   
   DEBUG(100, ("Sending md5 auth key to user from %s\n", 
               inet_ntoa(pack.from_ip)));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
      
}


/**************************************************************************/
/* Send auth md5 error to user						  */
/**************************************************************************/
void send_authmd5_fail(Packet &pack, unsigned short errcode, char *url)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_REGISTRATION
          << (unsigned short)SN_IES_LOGINxREPLY
	  << (unsigned short)0x0000
	  << (unsigned  long)0x00000000;
	  
   arpack << (unsigned short)0x0008
	  << (unsigned short)0x0002
	  << (unsigned short)errcode;
	  
   if (strlen(url) > 0)
   {
      arpack << (unsigned short)0x0004
             << (unsigned short)strlen(url)
	     << url;
   }
   
   DEBUG(100, ("Sending md5 auth failed (code=%d) to user from %s\n", 
               errcode, inet_ntoa(pack.from_ip)));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
      
}


/**************************************************************************/
/* Send auth md5 cookie (md5 sign on finished)				  */
/**************************************************************************/
void send_authmd5_cookie(Packet &pack, char *screen_name, char* bos_address, 
                         char *server_cookie, unsigned short cookie_length)
{
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();
   
   arpack << (unsigned short)SN_TYP_REGISTRATION
          << (unsigned short)SN_IES_LOGINxREPLY
	  << (unsigned short)0x0000
	  << (unsigned  long)0x00000000;
	  
   arpack << (unsigned short)0x008E
	  << (unsigned short)0x0001
	  << (char)'\0';

   arpack << (unsigned short)0x0001
	  << (unsigned short)strlen(screen_name)
	  << screen_name;

   arpack << (unsigned short)0x0005
	  << (unsigned short)strlen(bos_address)
	  << bos_address;
	  
   arpack << (unsigned short)0x0006
	  << (unsigned short)cookie_length
	  << server_cookie;
	  
   DEBUG(100, ("Sending md5 signon reply (redirect) to user from %s\n", 
               inet_ntoa(pack.from_ip)));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
   
   arpack.reset();
   arpack.setup_aim();
   arpack.flap_channel = 0x04;

   DEBUG(400, ("Sending close connection packet to user from %s\n", 
               inet_ntoa(pack.from_ip)));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
 
}


/**************************************************************************/
/* Handle securid reply from client (admin login sequence) - for testing  */
/**************************************************************************/
void process_ies_securid(struct snac_header &snac, Packet &pack)
{
   DEBUG(100, ("User \"%s\" have sent SecurID number...\n", 
                inet_ntoa(pack.from_ip)));
   send_authmd5_fail(pack, AUTH_INVALID_SECUREID, "");
}

/**************************************************************************/
/* Handle signon request from new client                                  */
/**************************************************************************/
void process_ies_image_req(struct snac_header &snac, Packet &pack)
{
#ifdef HAVE_GD
   DEBUG(100, ("User \"%s\" have sent Signon request...\n",
                inet_ntoa(pack.from_ip)));

   /* Create verify code image */
   gdImagePtr im;
   PGresult *res;
   fstring dbcomm_str;
   int regcode_img_size;
   char *regcode_img_data;
   char text_out[9];
   int black, white;
   int pic_w = 240;
   int pic_h = 100;
   char src[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

   im = gdImageCreate(pic_w, pic_h);

   /* first color will be background color */
   white = gdImageColorAllocate(im, 255, 250, 200);

   black = gdImageColorAllocate(im, 0, 0, 0);

   srand (time (NULL));
   for (int i=0;i<8;i++)
   {
      snprintf(text_out+i, 8, "%c", src[rand()%36]);
   }
   
   gdImageString(im, gdFontGiant, 80, 45, (unsigned char*)text_out, black);

   regcode_img_data = (char *) gdImageJpegPtr(im, &regcode_img_size, -1);
   if (!regcode_img_data)
   {
      DEBUG(10, ("Can't create image for send to new client!\n"));
      return;
   }
   gdImageDestroy(im);

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
      "DELETE FROM Regimage_Codes WHERE iadr=%lu", ipToIcq(pack.from_ip));
      
   res = PQexec(users_dbconn, dbcomm_str);

   PQclear(res);
   
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
      "INSERT INTO Regimage_Codes values (%lu,%lu,'%s')", ipToIcq(pack.from_ip),0,text_out);

   res = PQexec(users_dbconn, dbcomm_str);

   PQclear(res);
   
   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();

   arpack << (unsigned short)SN_TYP_REGISTRATION
          << (unsigned short)SN_IES_REQxIMAGExREPLY
	  << (unsigned short)0x0000
	  << (unsigned  long)0x00000000
	  << (unsigned short)0x0001
	  << (unsigned short)0x000a
	  << "image/jpeg"
	  << (unsigned short)0x0002
	  << (unsigned short)regcode_img_size;

   memcpy(arpack.nextData, regcode_img_data, regcode_img_size);
   arpack.sizeVal += regcode_img_size;
   
   gdFree(regcode_img_data);

   DEBUG(300, ("DEBUG: Sending image with code to user (%s) for new registration\n", inet_ntoa(pack.from_ip)));

   arpack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(arpack);
#else
   DEBUG(100, ("User \"%s\" have sent Signon request, but server havn't this feature...",
                inet_ntoa(pack.from_ip)));
#endif
}
