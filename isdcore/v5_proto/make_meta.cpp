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
/* V5 USER_META packets assembling					  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#include "v5_defines.h"


/**************************************************************************/
/* INFO_META_USER FAIL packet creator				   	  */
/**************************************************************************/
void v5_prepare_meta_fail(unsigned short command, unsigned short seq2, 
		       struct online_user &user)
{
   reply_pack.clearPacket();
  
   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)user.session_id
   	       << (unsigned short)ICQ_CMDxSND_METAxUSER
               << (unsigned short)user.servseq
               << (unsigned short)seq2
 	       << (unsigned  long)user.uin
 	       << (unsigned  long)0x0000  
 	       << (unsigned short)command
 	       << (char)0x32;
}


/**************************************************************************/
/* INFO_META_USER packet creator				   	  */
/**************************************************************************/
void v5_send_meta_info(unsigned short seq2, struct online_user &user,
		       struct full_user_info &tuser, BOOL success)
{
   reply_pack.clearPacket();
   
   if (success)
   {
      reply_pack  << (unsigned short)V5_PROTO
                  << (char)0x00
                  << (unsigned  long)user.session_id
   	          << (unsigned short)ICQ_CMDxSND_METAxUSER
                  << (unsigned short)user.servseq
	          << (unsigned short)seq2
	          << (unsigned  long)user.uin
	          << (unsigned  long)0x0000  
	          << (unsigned short)SRV_META_USER_INFO
	          << (char)0x0a
	          << (unsigned short)(strlen(tuser.nick)+1)
	          << tuser.nick
	          << (unsigned short)(strlen(tuser.first)+1)
	          << tuser.first
	          << (unsigned short)(strlen(tuser.last)+1)
	          << tuser.last
	          << (unsigned short)(strlen(tuser.email2)+1)
	          << tuser.email2
	          << (char)tuser.auth
	          << (char)tuser.gender
	          << (char)0x00;
   }
   else
   {
     /* we haven't user info so we should send meta fail yo user */
     v5_prepare_meta_fail(SRV_META_USER_INFO, seq2, user);
   }
   	       
   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending simply info meta-reply to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}


/**************************************************************************/
/* INFO2_META_USER packet creator				   	  */
/**************************************************************************/
void v5_send_meta_info2(unsigned short seq2, struct online_user &user,
		        struct full_user_info &tuser, BOOL success)
{
   reply_pack.clearPacket();
   
   if (success)
   {
      reply_pack  << (unsigned short)V5_PROTO
                  << (char)0x00
                  << (unsigned  long)user.session_id
   	          << (unsigned short)ICQ_CMDxSND_METAxUSER
                  << (unsigned short)user.servseq
	          << (unsigned short)seq2
	          << (unsigned  long)user.uin
	          << (unsigned  long)0x0000  
	          << (unsigned short)SRV_META_USER_INFO2
	          << (char)0x0a
	          << (unsigned short)(strlen(tuser.nick)+1)
	          << tuser.nick
	          << (unsigned short)(strlen(tuser.first)+1)
	          << tuser.first
	          << (unsigned short)(strlen(tuser.last)+1)
	          << tuser.last;

      if ((tuser.e1publ != 1) || (tuser.uin == user.uin))
      {
         reply_pack  << (unsigned short)(strlen(tuser.email1)+1)
	             << tuser.email1;
      }
      else
      {
         reply_pack  << (unsigned short)(strlen("")+1)
	             << "";
      }
		  
      reply_pack  << (unsigned short)(strlen(tuser.email2)+1)
		  << tuser.email2
		  << (unsigned short)(strlen(tuser.email3)+1)
		  << tuser.email3
		  << (unsigned short)(strlen(tuser.hcity)+1)
		  << tuser.hcity
		  << (unsigned short)(strlen(tuser.hstate)+1)
		  << tuser.hstate
		  << (unsigned short)(strlen(tuser.hphone)+1)
		  << tuser.hphone
		  << (unsigned short)(strlen(tuser.hfax)+1)
		  << tuser.hfax
		  << (unsigned short)(strlen(tuser.haddr)+1)
		  << tuser.haddr
		  << (unsigned short)(strlen(tuser.hcell)+1)
		  << tuser.hcell
		  << (unsigned  long)tuser.hzip
		  << (unsigned short)tuser.hcountry
		  << (unsigned short)tuser.gmt_offset
		  << (char)tuser.auth
		  << (char)tuser.webaware
		  << (char)tuser.iphide
		  << (char)0x00
		  << (char)0x00;
   }
   else
   {
     /* we haven't user info so we should send meta fail yo user */
     v5_prepare_meta_fail(SRV_META_USER_INFO2, seq2, user);                     
   }
   	       
   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending home info meta-reply to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);
}


/**************************************************************************/
/* INFO3_META_USER packet creator (used with icq99b)		   	  */
/**************************************************************************/
void v5_send_meta_info3(unsigned short seq2, struct online_user &user,
		        struct full_user_info &tuser, BOOL success)
{

   ffstring hzip;
   reply_pack.clearPacket();
   
   if (success)
   {
      snprintf(hzip, 31, "%lu", tuser.hzip);
      
      reply_pack  << (unsigned short)V5_PROTO
                  << (char)0x00
                  << (unsigned  long)user.session_id
   	          << (unsigned short)ICQ_CMDxSND_METAxUSER
                  << (unsigned short)user.servseq
	          << (unsigned short)seq2
	          << (unsigned  long)user.uin
	          << (unsigned  long)0x0000  
	          << (unsigned short)SRV_META_USER_INFO2
	          << (char)0x0a
	          << (unsigned short)(strlen(tuser.nick)+1)
	          << tuser.nick
	          << (unsigned short)(strlen(tuser.first)+1)
	          << tuser.first
	          << (unsigned short)(strlen(tuser.last)+1)
	          << tuser.last;

      if ((tuser.e1publ != 1) || (tuser.uin == user.uin))
      {
         reply_pack  << (unsigned short)(strlen(tuser.email1)+1)
	             << tuser.email1;
      }
      else
      {
         reply_pack  << (unsigned short)(strlen("")+1)
	             << "";
      }

      reply_pack  << (unsigned short)(strlen(tuser.email2)+1)
		  << tuser.email2
		  << (unsigned short)(strlen(tuser.email3)+1)
		  << tuser.email3
		  << (unsigned short)(strlen(tuser.hcity)+1)
		  << tuser.hcity
		  << (unsigned short)(strlen(tuser.hstate)+1)
		  << tuser.hstate
		  << (unsigned short)(strlen(tuser.hphone)+1)
		  << tuser.hphone
		  << (unsigned short)(strlen(tuser.hfax)+1)
		  << tuser.hfax
		  << (unsigned short)(strlen(tuser.haddr)+1)
		  << tuser.haddr
		  << (unsigned short)(strlen(tuser.hcell)+1)
		  << tuser.hcell
		  << (unsigned short)(strlen(hzip)+1)
		  << hzip
		  << (unsigned short)tuser.hcountry
		  << (unsigned short)tuser.gmt_offset
		  << (char)0x01
		  << (char)tuser.e1publ
		  << (char)0x00
		  << (char)0x00
		  << (char)0x00;
   }
   else
   {
     /* we haven't user info so we should send meta fail yo user */
     v5_prepare_meta_fail(SRV_META_USER_INFO2, seq2, user);                     
   }
   	       
   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending home info meta-reply to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}



/**************************************************************************/
/* INFO_MORE_META_USER packet creator				   	  */
/**************************************************************************/
void v5_send_meta_more(unsigned short seq2, struct online_user &user,
		        struct full_user_info &tuser, BOOL success)
{
   char temp_year;
   
   reply_pack.clearPacket();
   
   if (success)
   {
      if (tuser.byear < 1900) {temp_year = tuser.byear; }
                         else {temp_year = tuser.byear - 1900; };
   
      reply_pack  << (unsigned short)V5_PROTO
                  << (char)0x00
                  << (unsigned  long)user.session_id
   	          << (unsigned short)ICQ_CMDxSND_METAxUSER
                  << (unsigned short)user.servseq
	          << (unsigned short)seq2
	          << (unsigned  long)user.uin
	          << (unsigned  long)0x0000  
	          << (unsigned short)SRV_META_INFO_MORE
	          << (char)0x0a
		  << (unsigned short)tuser.age
		  << (char)tuser.gender
		  << (unsigned short)(strlen(tuser.hpage)+1)
		  << tuser.hpage
		  << (char)temp_year
		  << (char)tuser.bmonth
		  << (char)tuser.bday
		  << (char)tuser.lang1
		  << (char)tuser.lang2
		  << (char)tuser.lang3;
   }
   else
   {
      /* we haven't user info so we should send meta fail yo user */
      v5_prepare_meta_fail(SRV_META_INFO_MORE, seq2, user);                     
   }
   	       
   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending more info meta-reply to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}


/**************************************************************************/
/* INFO_MORE_META_USER packet creator				   	  */
/**************************************************************************/
void v5_send_meta_more2(unsigned short seq2, struct online_user &user,
		        struct full_user_info &tuser, BOOL success)
{
   reply_pack.clearPacket();
   
   if (success)
   {
      reply_pack  << (unsigned short)V5_PROTO
                  << (char)0x00
                  << (unsigned  long)user.session_id
   	          << (unsigned short)ICQ_CMDxSND_METAxUSER
                  << (unsigned short)user.servseq
	          << (unsigned short)seq2
	          << (unsigned  long)user.uin
	          << (unsigned  long)0x0000  
	          << (unsigned short)SRV_META_INFO_MORE
	          << (char)0x0a
		  << (unsigned short)tuser.age
		  << (char)tuser.gender
		  << (unsigned short)(strlen(tuser.hpage)+1)
		  << tuser.hpage
		  << (unsigned short)tuser.byear
		  << (char)tuser.bmonth
		  << (char)tuser.bday
		  << (char)tuser.lang1	/* user language 1 */
		  << (char)tuser.lang2	/* user language 2 */
		  << (char)tuser.lang3;	/* user language 3 */
   }
   else
   {
      /* we haven't user info so we should send meta fail yo user */
      v5_prepare_meta_fail(SRV_META_INFO_MORE, seq2, user);                     
   }
   	       
   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending more info meta-reply to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}


/**************************************************************************/
/* WORK_META_USER packet creator				   	  */
/**************************************************************************/
void v5_send_meta_work(unsigned short seq2, struct online_user &user,
		       struct full_user_info &tuser, BOOL success)
{
   reply_pack.clearPacket();
   
   if (success)
   {
      reply_pack  << (unsigned short)V5_PROTO
                  << (char)0x00
                  << (unsigned  long)user.session_id
   	          << (unsigned short)ICQ_CMDxSND_METAxUSER
                  << (unsigned short)user.servseq
	          << (unsigned short)seq2
	          << (unsigned  long)user.uin
	          << (unsigned  long)0x0000  
	          << (unsigned short)SRV_META_INFO_WORK
	          << (char)0x0a
	          << (unsigned short)(strlen(tuser.wcity)+1)
	          << tuser.wcity
	          << (unsigned short)(strlen(tuser.wstate)+1)
	          << tuser.wstate
	          << (unsigned short)(strlen(tuser.wphone)+1)
	          << tuser.wphone
	          << (unsigned short)(strlen(tuser.wfax)+1)
	          << tuser.wfax
		  << (unsigned short)(strlen(tuser.waddr)+1)
		  << tuser.waddr
		  << (unsigned  long)tuser.wzip
		  << (unsigned short)tuser.wcountry
		  << (unsigned short)(strlen(tuser.wcompany)+1)
		  << tuser.wcompany
		  << (unsigned short)(strlen(tuser.wdepart2)+1)
		  << tuser.wdepart2
		  << (unsigned short)(strlen(tuser.wtitle)+1)
		  << tuser.wtitle
		  << (unsigned short)tuser.wocup 
		  << (unsigned short)(strlen(tuser.wpage)+1)
		  << tuser.wpage;
   }
   else
   {
     /* we haven't user info so we should send meta fail yo user */
     v5_prepare_meta_fail(SRV_META_INFO_WORK, seq2, user);                     
   }
   	       
   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending work info meta-reply to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}


/**************************************************************************/
/* WORK_META_USER packet creator (used with icq99b)		   	  */
/**************************************************************************/
void v5_send_meta_work2(unsigned short seq2, struct online_user &user,
		       struct full_user_info &tuser, BOOL success)
{
   ffstring wzip;
   reply_pack.clearPacket();
   
   if (success)
   {
      snprintf(wzip, 31, "%lu", tuser.wzip);
      
      reply_pack  << (unsigned short)V5_PROTO
                  << (char)0x00
                  << (unsigned  long)user.session_id
   	          << (unsigned short)ICQ_CMDxSND_METAxUSER
                  << (unsigned short)user.servseq
	          << (unsigned short)seq2
	          << (unsigned  long)user.uin
	          << (unsigned  long)0x0000  
	          << (unsigned short)SRV_META_INFO_WORK
	          << (char)0x0a
	          << (unsigned short)(strlen(tuser.wcity)+1)
	          << tuser.wcity
	          << (unsigned short)(strlen(tuser.wstate)+1)
	          << tuser.wstate
	          << (unsigned short)(strlen(tuser.wphone)+1)
	          << tuser.wphone
	          << (unsigned short)(strlen(tuser.wfax)+1)
	          << tuser.wfax
		  << (unsigned short)(strlen(tuser.waddr)+1)
		  << tuser.waddr
		  << (unsigned short)(strlen(wzip)+1)
		  << wzip
		  << (unsigned short)tuser.wcountry
		  << (unsigned short)(strlen(tuser.wcompany)+1)
		  << tuser.wcompany
		  << (unsigned short)(strlen(tuser.wdepart2)+1)
		  << tuser.wdepart2
		  << (unsigned short)(strlen(tuser.wtitle)+1)
		  << tuser.wtitle
		  << (unsigned short)tuser.wocup 
		  << (unsigned short)(strlen(tuser.wpage)+1)
		  << tuser.wpage;
   }
   else
   {
     /* we haven't user info so we should send meta fail yo user */
     v5_prepare_meta_fail(SRV_META_INFO_WORK, seq2, user);                     
   }
   	       
   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending work info meta-reply to user %lu\n", user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}



/**************************************************************************/
/* ABOUT_META_USER packet creator				   	  */
/**************************************************************************/
void v5_send_meta_about(unsigned short seq2, struct online_user &user,
		        struct notes_user_info &tuser, BOOL success)
{
   reply_pack.clearPacket();
   
   if (success)
   {
      reply_pack  << (unsigned short)V5_PROTO
                  << (char)0x00
                  << (unsigned  long)user.session_id
   	          << (unsigned short)ICQ_CMDxSND_METAxUSER
                  << (unsigned short)user.servseq
	          << (unsigned short)seq2
	          << (unsigned  long)user.uin
	          << (unsigned  long)0x0000  
	          << (unsigned short)SRV_META_INFO_ABOUT
	          << (char)0x0a
	          << (unsigned short)(strlen(tuser.notes)+1)
	          << tuser.notes;
   }
   else
   {
      /* we haven't user info so we should send meta fail yo user */
      v5_prepare_meta_fail(SRV_META_INFO_ABOUT, seq2, user);                     
   }
   	       
   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending work info meta-reply to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* sending packets to client, we need confirm! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}


/**************************************************************************/
/* USER_FOUND packet creator				   	          */
/**************************************************************************/
void v5_send_user_found(unsigned short seq2, struct online_user &user,
		        struct full_user_info &tuser, BOOL last, 
			BOOL success, unsigned long users_left)
{
   reply_pack.clearPacket();
   
   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)user.session_id
               << (unsigned short)ICQ_CMDxSND_METAxUSER
               << (unsigned short)user.servseq
               << (unsigned short)seq2
	       << (unsigned  long)user.uin
	       << (unsigned  long)0x0000;
	       
	       if (last != 1)
	       { 
	           reply_pack << (unsigned short)SRV_META_USER_FOUND;
	       }
	       else
	       {
	       	   reply_pack << (unsigned short)SRV_META_USER_LAST_FOUND;
	       }
		   
	       if  (success) 
	       {
	          reply_pack << (char)0x0a;
	       }
	       else
	       {
	          reply_pack << (char)0x32;
	       }
	       
   if (success)
   {
      reply_pack  << (unsigned  long)tuser.uin
   	          << (unsigned short)(strlen(tuser.nick)+1)
	          << tuser.nick
	          << (unsigned short)(strlen(tuser.first)+1)
	          << tuser.first
	          << (unsigned short)(strlen(tuser.last)+1)
	          << tuser.last
	          << (unsigned short)(strlen(tuser.email2)+1)
	          << tuser.email2
	          << (char)tuser.auth
	          << (char)0x01;
   }
   
   if (last == 1) 
   {
      reply_pack << users_left;
   }
	       
   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending search info meta-reply to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}


/**************************************************************************/
/* WHITE_SEARCH_USER_FOUND packet creator				  */
/**************************************************************************/
void v5_send_white_user_found(unsigned short seq2, struct online_user &user,
		    	      struct full_user_info &tuser, BOOL last, 
			      BOOL success, unsigned long users_left)
{
   reply_pack.clearPacket();
   
   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)user.session_id
               << (unsigned short)ICQ_CMDxSND_METAxUSER
               << (unsigned short)user.servseq
               << (unsigned short)seq2
	       << (unsigned  long)user.uin
	       << (unsigned  long)0x0000;
	       
	       if (last != 1)
	       { 
	           reply_pack << (unsigned short)SRV_META_WHITE_FOUND;
	       }
	       else
	       {
	       	   reply_pack << (unsigned short)SRV_META_WHITE_LAST_FOUND;
	       }
		   
	       if  (success) 
	       {
	          reply_pack << (char)0x0a;
	       }
	       else
	       {
	          reply_pack << (char)0x32;
	       }
	       
   if (success)
   {
      reply_pack  << (unsigned  long)tuser.uin
   	          << (unsigned short)(strlen(tuser.nick)+1)
	          << tuser.nick
	          << (unsigned short)(strlen(tuser.first)+1)
	          << tuser.first
	          << (unsigned short)(strlen(tuser.last)+1)
	          << tuser.last
	          << (unsigned short)(strlen(tuser.email2)+1)
	          << tuser.email2
	          << (char)tuser.auth
	          << (char)tuser.webaware;

       if (last == 1) 
       {
          reply_pack << users_left;
       }

   }
   
	       
   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending white search info meta-reply to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}


/**************************************************************************/
/* WHITE_SEARCH_USER_FOUND2 packet creator				  */
/**************************************************************************/
void v5_send_white_user_found2(unsigned short seq2, struct online_user &user,
		    	      struct full_user_info &tuser, BOOL last, 
			      BOOL success, unsigned long users_left)
{
   unsigned short pack_len;

   reply_pack.clearPacket();
   
   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)user.session_id
               << (unsigned short)ICQ_CMDxSND_METAxUSER
               << (unsigned short)user.servseq
               << (unsigned short)seq2
	       << (unsigned  long)user.uin
	       << (unsigned  long)0x0000;
	       
	       if (last != 1)
	       { 
	           reply_pack << (unsigned short)SRV_META_WHITE_FOUND;
	       }
	       else
	       {
	       	   reply_pack << (unsigned short)SRV_META_WHITE_LAST_FOUND;
	       }
		   
	       if  (success) 
	       {
	          reply_pack << (char)0x0a;
	       }
	       else
	       {
	          reply_pack << (char)0x32;
	       }
	       
   if (success)
   {
   
      pack_len = 15 + strlen(tuser.nick) + strlen(tuser.first) + 
                      strlen(tuser.last) + strlen(tuser.email2) + 4;

      reply_pack  << (unsigned short)pack_len
                  << (unsigned  long)tuser.uin
   	          << (unsigned short)(strlen(tuser.nick)+1)
	          << tuser.nick
	          << (unsigned short)(strlen(tuser.first)+1)
	          << tuser.first
	          << (unsigned short)(strlen(tuser.last)+1)
	          << tuser.last
	          << (unsigned short)(strlen(tuser.email2)+1)
	          << tuser.email2
	          << (char)tuser.auth
	          << (char)tuser.webaware
		  << (char)0x00;

       if (last == 1) 
       {
          reply_pack << users_left;
       }

   }
	       
   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending white search info meta-reply to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}


/**************************************************************************/
/* USER_FOUND packet creator				   	          */
/**************************************************************************/
void v5_send_user_found2(unsigned short seq2, struct online_user &user,
		        struct full_user_info &tuser, BOOL last, 
			BOOL success, unsigned long users_left)
{
   unsigned short pack_len;

   reply_pack.clearPacket();
   
   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)user.session_id
               << (unsigned short)ICQ_CMDxSND_METAxUSER
               << (unsigned short)user.servseq
               << (unsigned short)seq2
	       << (unsigned  long)user.uin
	       << (unsigned  long)0x0000;
	       
	       if (last != 1)
	       { 
	           reply_pack << (unsigned short)SRV_META_USER_FOUND;
	       }
	       else
	       {
	       	   reply_pack << (unsigned short)SRV_META_USER_LAST_FOUND;
	       }
		   
	       if  (success) 
	       {
	          reply_pack << (char)0x0a;
	       }
	       else
	       {
	          reply_pack << (char)0x32;
	       }
	       
   if (success)
   {
      pack_len = 15 + strlen(tuser.nick) + strlen(tuser.first) + 
                      strlen(tuser.last) + strlen(tuser.email2) + 4;
      
      reply_pack  << (unsigned short)pack_len
                  << (unsigned  long)tuser.uin
   	          << (unsigned short)(strlen(tuser.nick)+1)
	          << tuser.nick
	          << (unsigned short)(strlen(tuser.first)+1)
	          << tuser.first
	          << (unsigned short)(strlen(tuser.last)+1)
	          << tuser.last
	          << (unsigned short)(strlen(tuser.email2)+1)
	          << tuser.email2
	          << (char)tuser.auth
		  << (char)tuser.webaware
		  << (char)0x00;
       
       if (last == 1) 
       {
	  reply_pack << users_left;
       }
   }
   	       
   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending search info meta-reply2 to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}


/**************************************************************************/
/* SET_ACK_META_USER packet creator				   	  */
/**************************************************************************/
void v5_send_meta_set_ack(unsigned short seq2, struct online_user &user,
		          unsigned short command, BOOL success)
{
   reply_pack.clearPacket();
   
   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)user.session_id
  	       << (unsigned short)ICQ_CMDxSND_METAxUSER
               << (unsigned short)user.servseq
	       << (unsigned short)seq2
	       << (unsigned  long)user.uin
	       << (unsigned  long)0x0000  
	       << (unsigned short)command;
	       
   if (success) 
   { 
      reply_pack << (char)0x0a;
   }
   else
   {
      reply_pack << (char)0x32;
   }
   	       
   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending set ack meta-reply to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);
}


/**************************************************************************/
/* HPAGE CATEGORY_META_INFO packet creator			   	  */
/**************************************************************************/
void v5_send_meta_hpage_cat(unsigned short seq2, struct online_user &user, 
			    struct full_user_info &tuser, BOOL success)
{
   reply_pack.clearPacket();
   
   if ((tuser.hpage_cf > 1) | (tuser.hpage_cf < 0)) tuser.hpage_cf = 0;
   
   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)user.session_id
  	       << (unsigned short)ICQ_CMDxSND_METAxUSER
               << (unsigned short)user.servseq
	       << (unsigned short)seq2
	       << (unsigned  long)user.uin
	       << (unsigned  long)0x0000  
	       << (unsigned short)SRV_META_INFO_HPAGE_CAT
	       << (char)0x0a
	       << (char)tuser.hpage_cf
	       << (unsigned short)tuser.hpage_cat
	       << (unsigned short)(strlen(tuser.hpage_txt)+1)
	       << tuser.hpage_txt
	       << (char)0x00;
   	       
   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending hpage category info meta-reply to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);
}


/**************************************************************************/
/* INTERESTSINFO_META_USER packet creator			   	  */
/**************************************************************************/
void v5_send_meta_interestsinfo(unsigned short seq2, struct online_user &user, 
				struct full_user_info &tuser, BOOL success)
{
   char i;
   if ((tuser.int_num > 4) | (tuser.int_num < 0)) tuser.int_num = 0;

   reply_pack.clearPacket();   
   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)user.session_id
  	       << (unsigned short)ICQ_CMDxSND_METAxUSER
               << (unsigned short)user.servseq
	       << (unsigned short)seq2
	       << (unsigned  long)user.uin
	       << (unsigned  long)0x0000  
	       << (unsigned short)SRV_META_INFO_INTERESTS
	       << (char)0x0a	       
	       << (char)tuser.int_num;
	       
	       
   for (i=0; i<tuser.int_num; i++)
   {
       if (i==0)
       {
           reply_pack << (unsigned short)tuser.int_ind1
	              << (unsigned short)(strlen(tuser.int_key1)+1)
	              << tuser.int_key1;
       }
       if (i==1)
       {
           reply_pack << (unsigned short)tuser.int_ind2
	              << (unsigned short)(strlen(tuser.int_key2)+1)
	              << tuser.int_key2;
       }
       if (i==2)
       {
           reply_pack << (unsigned short)tuser.int_ind3
	              << (unsigned short)(strlen(tuser.int_key3)+1)
	              << tuser.int_key3;
       }
       if (i==3)
       {
           reply_pack << (unsigned short)tuser.int_ind4
	              << (unsigned short)(strlen(tuser.int_key4)+1)
	              << tuser.int_key4;
       }
   }	       

   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending interests info meta-reply to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* sending packets to client, we need confirm! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);
}


/**************************************************************************/
/* FINAL_META_USER packet creator			   	  	  */
/**************************************************************************/
void v5_send_meta_affilationsinfo(unsigned short seq2, 
				  struct online_user &user, 
				  struct full_user_info &tuser, BOOL success)
{
   reply_pack.clearPacket();

   if ((tuser.past_num > 3) | (tuser.past_num < 0)) tuser.past_num = 0;
   
   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)user.session_id
  	       << (unsigned short)ICQ_CMDxSND_METAxUSER
               << (unsigned short)user.servseq
	       << (unsigned short)seq2
	       << (unsigned  long)user.uin
	       << (unsigned  long)0x0000  
	       << (unsigned short)SRV_META_INFO_AFFILATIONS
	       << (char)0x0a

	       << (char)0x03
	       << (unsigned short)tuser.past_ind1
	       << (unsigned short)(strlen(tuser.past_key1)+1)
	       << tuser.past_key1
               << (unsigned short)tuser.past_ind2
	       << (unsigned short)(strlen(tuser.past_key2)+1)
	       << tuser.past_key2
	       << (unsigned short)tuser.past_ind3
	       << (unsigned short)(strlen(tuser.past_key3)+1)
               << tuser.past_key3
	       
	       << (char)0x03
	       << (unsigned short)tuser.aff_ind1
	       << (unsigned short)(strlen(tuser.aff_key1)+1)
	       << tuser.aff_key1
               << (unsigned short)tuser.aff_ind2
	       << (unsigned short)(strlen(tuser.aff_key2)+1)
	       << tuser.aff_key2
	       << (unsigned short)tuser.aff_ind3
	       << (unsigned short)(strlen(tuser.aff_key3)+1)
               << tuser.aff_key3
	       << (unsigned short)0x0000
	       << (unsigned short)0x0001
	       << (char)0x00;

   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending affilations info meta-reply to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);
}

