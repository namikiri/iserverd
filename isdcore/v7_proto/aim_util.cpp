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
/* This module contain convinient utility functions related AIM protocol  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* Ack SP to close connection with specified hdl, rnd parameters 	  */
/**************************************************************************/
void close_connection(Packet &pack)
{
   struct online_user *user;

   /* delete user cookie and online_records */
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) != NULL)
   {
      unsigned long uptime = (unsigned long)(time(NULL) - longToTime(user->uptime));
      LOG_USR(0, ("User %lu moved offline. Online time was: %lu seconds.\n", user->uin, uptime));
      move_user_offline(user->uin);
      return;
   }

   pack.clearPacket();
   pack << (unsigned long)0x00000000;
   pack.sock_evt = SOCK_TERM;
   
   tcp_writeback_packet(pack);
}


/**************************************************************************/
/* Ack SP to close connection for specified user		 	  */
/**************************************************************************/
void close_connection(struct online_user *user)
{
   /* delete user cookie and online_records */
   unsigned long uptime = (unsigned long)(time(NULL) - longToTime(user->uptime));
   LOG_USR(0, ("User %lu moved offline. Online time was: %lu seconds..\n", user->uin, uptime));

   arpack.clearPacket();
   arpack << (unsigned long)0x00000000;
   arpack.sock_evt     = SOCK_TERM;
   arpack.from_ip      = user->ip;
   arpack.from_port    = 0;
   arpack.sock_hdl     = user->sock_hdl;
   arpack.sock_rnd     = user->sock_rnd;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 2;
   
   tcp_writeback_packet(arpack);
   move_user_offline(user->uin);
}


/**************************************************************************/
/* Read a string TLV from packet (tlv_type variable already read)	  */
/**************************************************************************/
unsigned short tlv_read_string(Packet &pack, char *string, int max)
{
   unsigned short tlv_length;
   char *str = string;
   
   pack >> tlv_length; 
   
   if (tlv_length > max) 
   { 
      tlv_length = 0; 
      str[0] = 0; 
      return(tlv_length); 
   }
 
   for (int i=0; i<tlv_length; i++) pack >> str[i];
   str[tlv_length] = 0; /* null-terminator */   
   
   return(tlv_length);
}


/**************************************************************************/
/* Read a short TLV from packet (tlv_type variable already read)	  */
/**************************************************************************/
void tlv_read_short(Packet &pack, unsigned short &dest)
{
   unsigned short tlv_length;
   pack >> tlv_length; 
   
   if (tlv_length != 2) 
   { 
      DEBUG(200, ("TLV should be len 2 (real len=%d)\n", tlv_length)); 
      dest = 0;
      return;
   }
   
   pack >> dest;
}


/**************************************************************************/
/* Read a long TLV from packet (tlv_type variable already read)		  */
/**************************************************************************/
void tlv_read_long(Packet &pack, unsigned long &dest)
{
   unsigned short tlv_length;
   pack >> tlv_length; 
   
   if (tlv_length != 4) 
   { 
      DEBUG(200, ("TLV should be len 4 (real len=%d)\n", tlv_length)); 
      dest = 0;
      return;
   }
   
   pack >> dest;
}


/**************************************************************************/
/* Check if it is an ICQ client (it also can be AIM client)		  */
/**************************************************************************/
BOOL isIcq(char *profile)
{
   char profile_trunc[10];
   
   if (strlen(profile) < 4) return False;
   
   strncpy(profile_trunc, profile, 3);
   profile_trunc[3] = 0;
   
   if (strequal(profile_trunc, "ICQ")) return True;
   
   return True;
}


/**************************************************************************/
/* Log unknown packet with comments 		 			  */
/**************************************************************************/
void log_unknown_packet(Packet &pack)
{
   DEBUG(10, ("We have unknown AIM packet (channel %d)....\n", pack.flap_channel));
   log_alarm_packet(10, pack);  /* dump unknown packet */
}


/**************************************************************************/
/* Decrypt password				 			  */
/**************************************************************************/
void decrypt_password(char *password, unsigned short passlen)
{
   char *pass = password;
   
   for (int i=0, k=0; i<passlen; i++, k++)
   {
      if (k > 16) k = 0;
      pass[i] = pass[i] ^ v7_cryptpass[k];
   }
}


/**************************************************************************/
/* Return BOS server address (load-balancing)	 			  */
/**************************************************************************/
void get_bos_server(char *server_addr)
{
   char *saddr = server_addr;
   
   /* TODO: Write load-balancing code for BOS services */
   strncpy(saddr, lp_v7_bos_address(), 254);
}


/**************************************************************************/
/* Generate a cookie for aim client authorization  			  */
/**************************************************************************/
void generate_cookie(Packet &pack, char *result, char *screen_name)
{
   md5_state_t state;
   u_int8_t md5_digest[64];

   const char *hex = "0123456789aBcDef";
   char *r;
   int i;

   r = result; 
   bzero(result, 256);
   
   strncpy(result, screen_name, 128);
   if (strlen(screen_name) > 128) 
   {
      r = result + 128;
   }
   else
   {
      r = result + strlen(screen_name);
   }
   
   /* first we should 2K buffer of random bytes */   
   random_fill(tempst, 2000);
   
   md5_init(&state);	
   md5_append(&state, (const md5_byte_t *)tempst, 1000);
   md5_finish(&state, (md5_byte_t *)md5_digest);

   for (i=0; i<16; i++) 
   {
      if ((md5_digest[i] > '0') && (md5_digest[i] < '[') ||
          (md5_digest[i] > 'a') && (md5_digest[i] < '}') || 
	  (md5_digest[i] > '(') && (md5_digest[i] < '.'))
      {
         *(r++) = md5_digest[i];
      }
      else
      {
	 *(r++) = hex[md5_digest[i] >> 4];
	 *(r++) = hex[md5_digest[i] & 0xF];
      }	 
   }
   
   md5_init(&state);	
   md5_append(&state, (const md5_byte_t *)tempst+1000, 1000);
   md5_finish(&state, (md5_byte_t *)md5_digest);

   for (i=0; i<16; i++) 
   {
      if ((md5_digest[i] > '0') && (md5_digest[i] < '[') ||
          (md5_digest[i] > 'a') && (md5_digest[i] < '}'))
      {
         *(r++) = md5_digest[i];
      }
      else
      {
	 *(r++) = hex[md5_digest[i] >> 4];
	 *(r++) = hex[md5_digest[i] & 0xF];
      }	 
   }

   /* we use only first ~60 bytes for cookie, but some */
   /* badly written clients like gaim and kxicq wants  */
   /* 256 byte cookie :( So we should fill free space  */
   /* by some value */
   
   r = result;
   
   for (i=0; i<256; i++) 
   {
      if (*(r+i) == 0) *(r+i) = 'A';
   }
   
   *(r+256) = '\0';
}


/**************************************************************************/
/* Generate the auth string for aim client md5 authorization 		  */
/**************************************************************************/
void generate_key(char *result)
{
   md5_state_t state;
   u_int8_t md5_digest[64];
   const char *hex = "0123456789aBcDef";
   char *r;
   int i;
   
   r = result;    
   bzero(result, 254);
   
   /* first we should 2K buffer of random bytes */   
   random_fill(tempst, 1000);

   md5_init(&state);	
   md5_append(&state, (const md5_byte_t *)tempst, 1000);
   md5_finish(&state, (md5_byte_t *)md5_digest);
   
   for (i=0; i<16; i++) 
   {
      if ((md5_digest[i] > '0') && (md5_digest[i] < '[') ||
          (md5_digest[i] > 'a') && (md5_digest[i] < '}') || 
	  (md5_digest[i] > '(') && (md5_digest[i] < '.'))
      {
         *(r++) = md5_digest[i];
      }
      else
      {
	 *(r++) = hex[md5_digest[i] >> 4];
	 *(r++) = hex[md5_digest[i] & 0xF];
      }	 
   }

   *r = 0;
}




/**************************************************************************/
/* copy sys data from dest packet to source	  			  */
/**************************************************************************/
void pcopy(Packet &dpack, Packet &spack)
{
   dpack.sock_hdl = spack.sock_hdl;
   dpack.from_local = spack.from_local;
   dpack.asciiz = spack.asciiz;
   dpack.net_order = spack.net_order;
   dpack.sock_rnd = spack.sock_rnd;
   dpack.sock_type = spack.sock_type;
   dpack.flap_channel = spack.flap_channel;
   dpack.sock_evt = spack.sock_evt;
   dpack.maxSize = spack.maxSize;
   dpack.from_port = spack.from_port;
   dpack.from_ip = spack.from_ip;
   dpack.sizeVal = spack.sizeVal;
}


/**************************************************************************/
/* Extract string from tlv			  			  */
/**************************************************************************/
BOOL v7_extract_string(char *dst_str, class tlv_c &spack, int max_len, 
		       char *fname, struct online_user &user, Packet &pack)
{
   unsigned short str_len, i;
   char description[32];
   
   spack >> str_len;

   snprintf(description, 32, "V7 %s string", fname);

   if (!islen_valid(str_len, max_len, description, user))
   {
      return(False);	 
   }
   else
   {
      for(i=0; i<str_len; i++) spack >> dst_str[i];
   }
   
   dst_str[i] = 0;
   
   return(True);
}


/**************************************************************************/
/* Extract string from tlv			  			  */
/**************************************************************************/
BOOL v7_extract_string(char *dst_str, class tlv_c &spack, int max_len)
{
   unsigned short str_len, i;
   
   str_len = spack.size;

   DEBUG(150, ("v7es: string size=%d, maximum_size=%d\n", str_len, max_len));
   if (str_len > max_len)
   {
      return(False);	 
   }
   else
   {
      for(i=0; i<str_len; i++) spack >> dst_str[i];
   }
   
   dst_str[i] = 0;
   
   return(True);
}


/**************************************************************************/
/* Extract string from tlv (light version)	  			  */
/**************************************************************************/
BOOL v7_extract_string(char *dst_str, class tlv_c &spack, int max_len, 
		       char *fname, Packet &pack)
{
   unsigned short str_len, i;
   char description[32];
   
   spack >> str_len;

   snprintf(description, 32, "V7 %s string", fname);

   if (str_len > max_len)
   {
      LOG_SYS(10, ("User from %s have sent too big %s\n", 
                   inet_ntoa(pack.from_ip), description));
		   
      close_connection(pack);
      return(False);	 
   }
   else
   {
      for(i=0; i<str_len; i++) spack >> dst_str[i];
   }
   
   dst_str[i] = 0;
   
   return(True);
}


/**************************************************************************/
/* Extract string from packet (light version)	  			  */
/**************************************************************************/
BOOL v7_extract_string(char *dst_str, Packet &spack, int max_len, 
		       char *fname)
{
   unsigned short str_len, i;
   char description[32];
   
   spack >> str_len;

   snprintf(description, 32, "V7 %s string", fname);

   if (str_len > max_len)
   {
      LOG_SYS(10, ("User from %s have sent too big %s\n", 
                   inet_ntoa(spack.from_ip), description));
		   
      close_connection(spack);
      return(False);	 
   }
   else
   {
      for(i=0; i<str_len; i++) spack >> dst_str[i];
   }
   
   dst_str[i] = 0;
   
   return(True);
}
/**************************************************************************/
/* Read a string uin value (BUIN) from packet 				  */
/**************************************************************************/
unsigned long read_buin(Packet &pack)
{
   size_t blen;
   char str[32];
   
   pack >> blen; 
   
   if ((blen > 31) ||
       (blen == 0))
   { 
      blen = 0; 
      str[0] = 0; 
      pack.append(); 
      return(0); 
   }
 
   for (unsigned int i=0; i<blen; i++) pack >> str[i];
   
   str[blen] = 0; /* null-terminator */   
   
   return(atoul(str));
}


/**************************************************************************/
/* Vx<-->V7 message type converter         				  */
/**************************************************************************/
unsigned short v7_convert_message_type(unsigned short type)
{
      switch (type)
      {
         case 0x14: return(0x09); /* broadcast message type */
         default:   return(type);
      }
}


/**************************************************************************/
/* Vx<-->V7 message text converter    				    	  */
/**************************************************************************/
char *v7_convert_message_text(unsigned long from_uin, unsigned short type, 
                              char *message)
{
   char *nmessage = msg_buff2;
   
   switch (type)
   {
      case 0x14: /* v3 client broadcast message */
      {
         if (from_uin != ADMIN_UIN)
	 {
            snprintf(nmessage, MAX_PACKET_SIZE, "ICQ Admin%s%s%s%s%s3%s%s", 
	             CLUE_CHAR, CLUE_CHAR, CLUE_CHAR, lp_admin_email(), 
		     CLUE_CHAR, CLUE_CHAR, message);
         }
         return(nmessage);
      }
      break;
      
      default:   return(NULL);
   }
   
   return(NULL);
}


/**************************************************************************/
/* Check if user supplied password digest is correct		    	  */
/**************************************************************************/
BOOL aim_check_digest(char *md5_digest, unsigned short md5_method, char *passwd, char *authkey)
{
   int i;
   BOOL result = True;
   char md5_real[16];
   char md5_pass[16];
   md5_state_t state;
   md5_state_t pstat;

   /* Old Method Hash = MD5(KEY + AIM_PASSWORD + "AOL Instant Messenger (SM)") */
   if (md5_method == false)
   {
   md5_init(&state);	
   md5_append(&state, (const md5_byte_t *)authkey, strlen(authkey));
   md5_append(&state, (const md5_byte_t *)passwd, strlen(passwd));
   md5_append(&state, (const md5_byte_t *)AIM_MD5_STRING, strlen(AIM_MD5_STRING));
   md5_finish(&state, (md5_byte_t *)md5_real);

      DEBUG(5, ("auth_md5: password crypted old md5 hash method...\n"));

   }

   /* New Method Hash = MD5(KEY + MD5(AIM_PASSWORD) + "AOL Instant Messenger (SM)") */
   else if (md5_method == true)
   {
   /* first we need md5-hash of our password */
   md5_init(&pstat);
   md5_append(&pstat, (const md5_byte_t *)passwd, strlen(passwd));
   md5_finish(&pstat, (md5_byte_t *)md5_pass);   

   /* calculate md5-hash to compare with */
   md5_init(&state);	
   md5_append(&state, (const md5_byte_t *)authkey, strlen(authkey));
   md5_append(&state, (const md5_byte_t *)md5_pass, sizeof(md5_pass));
   md5_append(&state, (const md5_byte_t *)AIM_MD5_STRING, strlen(AIM_MD5_STRING));
   md5_finish(&state, (md5_byte_t *)md5_real);

      DEBUG(5, ("auth_md5: password crypted new md5 hash method...\n"));

   }
 
   for (i=0; i<16; i++)
   {
      if (md5_digest[i] != md5_real[i]) 
      { 
         result = False;
	 break;
      }
   }
   
   if (!result)
   {
      DEBUG(5, ("ERROR: auth_md5 password digest check failed...\n"));
      return False;
   }
   else
   {
      DEBUG(5, ("auth_md5: password digest check success...\n"));
      return True;
   }

}


