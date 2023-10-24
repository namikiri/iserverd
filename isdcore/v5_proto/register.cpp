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
/* V5 registration functions 						  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#include "v5_defines.h"

/**************************************************************************/
/* CMD_ACK_NEW_UIN command handler					  */
/**************************************************************************/
void v5_process_ack_new_uin()
{
   unsigned short pvers, pcomm, seq1, seq2, i, plen;
   unsigned long uin_num, clivers, temp, session_id, xx1;
   char usrpass[20];
   char pemail[128];
   struct full_user_info new_user;

   memset(&new_user, 0, sizeof(new_user));
  
   int_pack.reset();
   int_pack  >> pvers   >> temp
	     >> uin_num >> session_id
 	     >> pcomm   >> seq1
             >> seq2    >> temp;
	   
   int_pack  >> plen; if (plen > 20) plen = 20; 
   for (i=0; i<plen; i++) int_pack >> usrpass[i];
  
   int_pack  >> clivers >> xx1;

   int_pack  >> plen; if (plen > 128) plen = 128; 
   for (i=0; i<plen; i++) int_pack >> pemail[i];

   LOG_USR(0, ("Registration request arrived from (%s:%d)\n", 
               inet_ntoa(int_pack.from_ip), int_pack.from_port));

   if (lp_v5_registration_enabled())
   {
      if (lp_v5_autoregister())
      {
         /* automatic registration */         
         /* we should fill user-info structure */
         
         strncpy(new_user.passwd, usrpass, 19);
         new_user.uin           = db_users_new_uin();
         new_user.can_broadcast = 0;
         new_user.wocup         = -1;
         new_user.ch_password   = 1;
         new_user.disabled      = 0;
         new_user.cr_date       = time(NULL);
         strncpy(new_user.email1, pemail, sizeof(new_user.email1));
         strncpy(new_user.email2, pemail, sizeof(new_user.email2));
   
         /* check for new_users_table */
         if (new_users_table_exist())
         {
            /*if exist we need to ajust uin number */
            temp = db_users_new_uin2();
            if (new_user.uin < temp) new_user.uin = temp;
      
         } 
                           
         db_users_add_user(new_user);
         LOG_USR(0, ("User from (%s:%d) now have uin = %lu\n", 
               inet_ntoa(int_pack.from_ip), int_pack.from_port, 
               new_user.uin));
         
         v5_send_ack_ext(int_pack, xx1, True);
         v5_send_new_uin(int_pack, new_user.uin, xx1);
	 
	 send_event2ap(papack, ACT_REGISTR, new_user.uin, 0,
	               ipToIcq(int_pack.from_ip), 0, longToTime(time(NULL)), "");
      }
      else
      {
         /* manual registration */
         
         /* I have no idea how to release this... V5 clients */
         /* haven't ability to show server notices and can't */
         /* handle manual registrations                      */

         /* check for new_users_table */
         if (new_users_table_exist())
         {
            /*if exist we need to ajust uin number */
            temp = db_users_new_uin2();
            if (new_user.uin < temp) new_user.uin = temp;
      
         } else { create_new_users_table(); }

         db_new_add_user(new_user);
         LOG_USR(0, ("User from (%s:%d) added to registration table...\n", 
               inet_ntoa(int_pack.from_ip), int_pack.from_port));
         
	 send_event2ap(papack, ACT_REGISTR, new_user.uin, 0,
	               ipToIcq(int_pack.from_ip), 0, longToTime(time(NULL)), "");
         
      }
   }
}


/**************************************************************************/
/* New user uin for V5 registration request                        	  */ 
/**************************************************************************/
void v5_send_new_uin(Packet &pack, unsigned long uin_num, unsigned long xx1)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  junk, session_id;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> junk >> junk >> session_id 
        >> pcomm >> seq1 >> seq2 ;
   
   reply_pack.clearPacket();
   reply_pack  << (unsigned short)V3_PROTO
  	       << (unsigned short)ICQ_CMDxSND_NEWxUIN
               << (unsigned short)0x0000
               << (unsigned short)seq2
               << (unsigned  long)uin_num
               << (unsigned  long)0x0000
  	       << (unsigned  long)xx1;

   reply_pack.from_ip   = pack.from_ip;
   reply_pack.from_port = pack.from_port;

   /* send packet to client */
   udp_send_direct_packet(reply_pack);

}


/**************************************************************************/
/* Extended ack command for V5 registration requests                  	  */ 
/**************************************************************************/
void v5_send_ack_ext(Packet &pack, unsigned long xx1, BOOL isOk)
{
   unsigned long  junk, session_id;
   unsigned short seq1, seq2, pcomm, vvers;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> junk >> junk >> session_id 
        >> pcomm >> seq1 >> seq2 ;
   

   reply_pack.clearPacket();
   
   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)session_id
  	       << (unsigned short)ICQ_CMDxSND_ACK
               << (unsigned short)seq1
               << (unsigned short)seq2
               << (unsigned  long)0x0000
  	       << (unsigned  long)0x0000;
               
   if (isOk)
   {
      reply_pack << (char)0x0a;
   }
   else
   {
      reply_pack << (char)0x32;
   }

   reply_pack << (unsigned  long)xx1;
  	    
   PutKey(reply_pack, calculate_checkcode(reply_pack));
    
   /* we do not need to encrypt packet */
   reply_pack.from_ip   = int_pack.from_ip;
   reply_pack.from_port = int_pack.from_port;

   /* send packets to client */
   udp_send_direct_packet(reply_pack);
}

