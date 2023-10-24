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
/* V5 packets assembling						  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#include "v5_defines.h"

/**************************************************************************/
/* End of system messages packet				   	  */
/**************************************************************************/
void v5_send_depslist(unsigned short seq2, struct online_user &user)
{
   reply_pack.clearPacket();
   
   reply_pack  << (unsigned short)V3_PROTO
               << (unsigned short)0x0032
               << (unsigned short)0x0000
               << (unsigned short)seq2
               << (unsigned  long)user.uin
	       << (unsigned  long)0x8FFCACBF;

   DEBUG(100, ("Sending depricated deps reply to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* Send packet to client */
   udp_send_direct_packet(reply_pack);

}


/**************************************************************************/
/* Invalid uin requested          				   	  */
/**************************************************************************/
void v5_send_invalid_uin(struct online_user &user, unsigned long uin_num)
{
   reply_pack.clearPacket();
   
   reply_pack << (unsigned short)V5_PROTO
              << (char)0x00
              << (unsigned  long)user.session_id
	      << (unsigned short)ICQ_CMDxSND_USERxINVALIDxUIN
              << (unsigned short)user.servseq
              << (unsigned short)0x0000
              << (unsigned  long)user.uin
	      << (unsigned  long)0x0000
              << (unsigned  long)uin_num;

   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending old-style invalid uin message to %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirmation! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}



/**************************************************************************/
/* End of system messages packet				   	  */
/**************************************************************************/
void v5_send_lmeta(struct online_user &user, unsigned short seq2)
{
   reply_pack.clearPacket();
   
   reply_pack << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)user.session_id
	       << (unsigned short)ICQ_CMDxSND_USERxLMETA
               << (unsigned short)user.servseq
               << (unsigned short)seq2
               << (unsigned  long)user.uin
	       << (unsigned  long)0x0000
	       << (unsigned short)0x07D0
	       << (unsigned short)0x0000
	       << (unsigned short)0x0046;

   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending login meta reply to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* Send packet to client, we need confirmation! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}



/**************************************************************************/
/* Old style extended basic info for user			   	  */
/**************************************************************************/
void v5_send_old_style_info_ext(struct online_user &user, 
                            struct full_user_info &tuser,
			    struct notes_user_info &notes)
{
   reply_pack.clearPacket();
   
   /* BUGBUG I don't know if i should check notes length here */
   /* it is mean buffer overflow possibility */
   
   reply_pack << (unsigned short)V5_PROTO
              << (char)0x00
              << (unsigned  long)user.session_id
	      << (unsigned short)ICQ_CMDxSND_OLD_INFO_EXT
              << (unsigned short)user.servseq
              << (unsigned short)0x0000
              << (unsigned  long)user.uin
	      << (unsigned  long)0x0000
              << (unsigned  long)tuser.uin
	      << (unsigned short)(strlen(tuser.hcity)+1)
	      << tuser.hcity
	      << (unsigned short)tuser.hcountry
	      << (char)0x01  /* I don't know what is it */
	      << (unsigned short)(strlen(tuser.hstate)+1)
	      << tuser.hstate
	      << (unsigned short)tuser.age
	      << (char)tuser.gender
	      << (unsigned short)(strlen(tuser.hphone)+1)
	      << tuser.hphone
	      << (unsigned short)(strlen(tuser.hpage)+1)
	      << tuser.hpage
	      << (unsigned short)(strlen(notes.notes)+1)
	      << notes.notes;

   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending old-style extended info to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirmation! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}



/**************************************************************************/
/* Old style basic info for user 				   	  */
/**************************************************************************/
void v5_send_old_style_info(struct online_user &user, 
                            struct full_user_info &tuser)
{
   reply_pack.clearPacket();
   
   reply_pack << (unsigned short)V5_PROTO
              << (char)0x00
              << (unsigned  long)user.session_id
	      << (unsigned short)ICQ_CMDxSND_USERxINFO_SINGLE
              << (unsigned short)user.servseq
              << (unsigned short)0x0000
              << (unsigned  long)user.uin
	      << (unsigned  long)0x0000
              << (unsigned  long)tuser.uin
              << (unsigned short)(strlen(tuser.nick)+1)
              << tuser.nick
              << (unsigned short)(strlen(tuser.first)+1)
              << tuser.first
              << (unsigned short)(strlen(tuser.last)+1)
              << tuser.last
              << (unsigned short)(strlen(tuser.email2)+1)
              << tuser.email2
              << (char)tuser.auth;

   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending old-style basic info to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirmation! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}



/**************************************************************************/
/* End of system messages packet				   	  */
/**************************************************************************/
void v5_send_end_sysmsg(struct online_user &user)
{
   reply_pack.clearPacket();
   
   reply_pack << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)user.session_id
	       << (unsigned short)ICQ_CMDxSND_ENDxSYSTEMxMESSAGES
               << (unsigned short)user.servseq
               << (unsigned short)0x0000
               << (unsigned  long)user.uin
	       << (unsigned  long)0x0000;

   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(100, ("Sending end of sys messages to user %lu\n", 
                user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirmation! */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}



/**************************************************************************/
/* User change status packet					   	  */
/**************************************************************************/
void v5_send_user_status(struct online_user &to_user, struct online_user &user)
{
   reply_pack.clearPacket();
   
   reply_pack << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)to_user.session_id
	       << (unsigned short)ICQ_CMDxSND_USERxSTATUS
               << (unsigned short)to_user.servseq
               << (unsigned short)0x0000
               << (unsigned  long)to_user.uin
	       << (unsigned  long)0x0000
	       << (unsigned  long)user.uin
	       << (unsigned short)user.status
	       << (unsigned short)user.estat;

   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(450, ("Online: Sending alert about %lu to user %lu\n", 
                user.uin, to_user.uin));

   reply_pack.from_ip   = to_user.ip;
   reply_pack.from_port = to_user.udp_port;
   
   /* send packet to client, we need confirmation! */
   v5_send_indirect(reply_pack, to_user.uin, to_user.shm_index);

}



/**************************************************************************/
/* User online packet						   	  */
/**************************************************************************/
void v5_send_user_online(struct online_user &to_user, struct online_user &user)
{
   reply_pack.clearPacket();
   
   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)to_user.session_id
	       << (unsigned short)ICQ_CMDxSND_USERxONLINE
               << (unsigned short)to_user.servseq
               << (unsigned short)0x0000
               << (unsigned  long)to_user.uin
	       << (unsigned  long)0x0000
	       << (unsigned  long)user.uin;
	       
   /* V3 clients TCP protection from other (99a, 99b, 2000, 98) */
   if ((user.protocol == V3_PROTO) ||
      ((user.protocol == V7_PROTO) && (!lp_v7_direct_v5_connect())))
   {
      reply_pack << (unsigned  long)0x00000000
                 << (unsigned  long)0x00000000
	         << (unsigned  long)0x00000000
                 << (char)0x00;
   }
   else
   {
      reply_pack << (unsigned  long)ipToIcq(user.ip)
                 << (unsigned  long)user.tcp_port
	         << (unsigned  long)ipToIcq(user.int_ip)
                 << (char)user.dc_type;
   }
	       
   reply_pack  << (unsigned short)user.status
	       << (unsigned short)user.estat
	       << (unsigned  long)user.tcpver
	       << (unsigned  long)user.dc_cookie
	       << (unsigned  long)user.web_port
	       << (unsigned  long)user.cli_futures
	       << (unsigned  long)user.info_utime
	       << (unsigned  long)user.more_utime
	       << (unsigned  long)user.stat_utime;

   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(450, ("Online: Sending alert about %lu to user %lu\n", 
                user.uin, to_user.uin));

   reply_pack.from_ip   = to_user.ip;
   reply_pack.from_port = to_user.udp_port;
   
   /* send packet to client, we need confirmation! */
   v5_send_indirect(reply_pack, to_user.uin, to_user.shm_index);
}


/**************************************************************************/
/* User offline packet						   	  */
/**************************************************************************/
void v5_send_user_offline(struct online_user &to_user, unsigned long uin)
{
   reply_pack.clearPacket();
   
   reply_pack << (unsigned short)V5_PROTO
               << (char)0x00  /* gap byte */
               << (unsigned  long)to_user.session_id
	       << (unsigned short)ICQ_CMDxSND_USERxOFFLINE
               << (unsigned short)to_user.servseq
               << (unsigned short)0x0000
               << (unsigned  long)to_user.uin
	       << (unsigned  long)0x0000
	       << (unsigned  long)uin;

   PutKey(reply_pack, calculate_checkcode(reply_pack));
   
   DEBUG(450, ("Online: Sending alert about %lu to user %lu\n", 
                uin, to_user.uin));

   reply_pack.from_ip   = to_user.ip;
   reply_pack.from_port = to_user.udp_port;
   
   /* send packet to client, we need confirmation! */
   v5_send_indirect(reply_pack, to_user.uin, to_user.shm_index);
}


/**************************************************************************/
/* Server should confirm *every* received packet (except client ack). 	  */
/**************************************************************************/
void v5_send_ack(Packet &pack)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num, junk, session_id;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> junk >> uin_num >> session_id 
        >> pcomm >> seq1 >> seq2 ;
        
   reply_pack.clearPacket();
   
   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)session_id
  	       << (unsigned short)ICQ_CMDxSND_ACK
               << (unsigned short)seq1
               << (unsigned short)seq2
               << (unsigned  long)uin_num
  	       << (unsigned  long)0x0000;
  	    
   PutKey(reply_pack, calculate_checkcode(reply_pack));
    
   /* we do not need to encrypt packet */
   reply_pack.from_ip   = pack.from_ip;
   reply_pack.from_port = pack.from_port;

   /* send packets to client */
   udp_send_direct_packet(reply_pack);
 
}


/**************************************************************************/
/* This packet sent if client can't be logged in for some reason   	  */
/**************************************************************************/
void v5_send_login_err(Packet &pack, char *errmessage)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num, junk, session_id;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> junk >> uin_num >> session_id 
        >> pcomm >> seq1 >> seq2 ;
   
   reply_pack.clearPacket();

   reply_pack << (unsigned short)V5_PROTO
              << (char)0x00
              << (unsigned  long)session_id
  	      << (unsigned short)ICQ_CMDxSND_LOGIN_ERR
  	      << (unsigned short)0x0000
              << (unsigned short)seq2
              << (unsigned  long)uin_num
  	      << (unsigned  long)0x0000
  	      << (unsigned short)(strlen(errmessage)+1)
  	      << errmessage
  	      << (unsigned  long)0x0000;

   PutKey(reply_pack, calculate_checkcode(reply_pack));

   reply_pack.from_ip   = pack.from_ip;
   reply_pack.from_port = pack.from_port;

   /* send packet to client */
   udp_send_direct_packet(reply_pack);
}


/**************************************************************************/
/* This packet sent if client can't be logged in for some reason   	  */
/**************************************************************************/
void v5_send_test_err(Packet &pack, unsigned short commd, char *errmessage)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num, junk, session_id;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> junk >> uin_num >> session_id 
        >> pcomm >> seq1 >> seq2 ;
   
   reply_pack.clearPacket();

   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)session_id
  	       << (unsigned short)commd
  	       << (unsigned short)0x0000
               << (unsigned short)seq2
               << (unsigned  long)uin_num
  	       << (unsigned  long)0x0000
  	       << (unsigned short)(strlen(errmessage)+1)
  	       << errmessage;

   PutKey(reply_pack, calculate_checkcode(reply_pack));

   reply_pack.from_ip   = pack.from_ip;
   reply_pack.from_port = pack.from_port;

   /* send packet to client */
   udp_send_direct_packet(reply_pack);
}


/**************************************************************************/
/* This packet sent if client's password wrong or it doesn't exist 	  */ 
/* in user database 						   	  */
/**************************************************************************/
void v5_send_pass_err(Packet &pack)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num, junk, session_id;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> junk >> uin_num >> session_id 
        >> pcomm >> seq1 >> seq2 ;
   
   reply_pack.clearPacket();
   reply_pack  << (unsigned short)V3_PROTO
  	       << (unsigned short)ICQ_CMDxSND_WRONGxPASSWD
               << (unsigned short)0x0000
               << (unsigned short)seq2
               << (unsigned  long)uin_num
               << (unsigned  long)0x0000
  	       << (unsigned short)0x0000;

   reply_pack.from_ip   = pack.from_ip;
   reply_pack.from_port = pack.from_port;

   /* send packet to client */
   udp_send_direct_packet(reply_pack);

}


/**************************************************************************/
/* Send busy packet to user					   	  */
/**************************************************************************/
void v5_send_busy(Packet &pack)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num, junk, session_id;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> junk >> uin_num >> session_id 
        >> pcomm >> seq1 >> seq2 ;
        
   reply_pack.clearPacket();
   
   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)session_id
  	       << (unsigned short)ICQ_CMDxSND_BUSY
               << (unsigned short)0x0000
               << (unsigned short)seq2
               << (unsigned  long)uin_num
  	       << (unsigned  long)0x0000;
  	    
   PutKey(reply_pack, calculate_checkcode(reply_pack));
    
   /* we do not need to encrypt packet */
   reply_pack.from_ip   = pack.from_ip;
   reply_pack.from_port = pack.from_port;
   
   /* send packet to client */
   udp_send_direct_packet(reply_pack);

}


/**************************************************************************/
/* Login reply packet						   	  */
/**************************************************************************/
void v5_send_login_reply(Packet &pack, struct login_user_info &userinfo, 
		         struct online_user &user)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num, junk, session_id;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> junk >> uin_num >> session_id 
        >> pcomm >> seq1 >> seq2;
   
   reply_pack.clearPacket();

   reply_pack << (unsigned short)V5_PROTO	       /* proto number	    */
              << (char)0x00			       /* gap?		    */
              << (unsigned  long)session_id  	       /* session_id number */
  	      << (unsigned short)ICQ_CMDxSND_HELLO
  	      << (unsigned short)0x0000		       /* sequence 1 num    */
              << (unsigned short)seq2		       /* sequence 2 num    */
              << (unsigned  long)uin_num	       /* client uin number */
  	      << (unsigned  long)0x0000	     
  	      << (unsigned short)0x008C
  	      << (unsigned short)0x0000
  	      << (unsigned short)(lp_v5_pingtime()-10) /* ping time	    */
  	      << (unsigned short)(lp_v5_timeout())     /* timeout	    */
  	      << (unsigned short)0x000a		       /* unknown	    */
  	      << (unsigned short)(lp_v5_retries())     /* retries	    */
  	      << (unsigned  long)ipToIcq(reply_pack.from_ip)
  	      << (unsigned  long)0x80CDC19B;	       /* server ID ?	    */
  	     
   PutKey(reply_pack, calculate_checkcode(reply_pack));

   /* we do not need to encrypt packet */

   reply_pack.from_ip   = pack.from_ip;
   reply_pack.from_port = pack.from_port;
   
   /* send packet to client */
   v5_send_indirect(reply_pack, uin_num, user.shm_index);

}


/**************************************************************************/
/* Non-logged client tried to work with server 			   	  */
/**************************************************************************/
void v5_send_not_connected(Packet &pack)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num, junk, session_id;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> junk >> uin_num >> session_id 
        >> pcomm >> seq1 >> seq2 ;
        
   reply_pack.clearPacket();
   
   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00		
               << (unsigned  long)session_id
  	       << (unsigned short)ICQ_CMDxSND_ERR_NOT_CONNECTED
               << (unsigned short)0x0000
               << (unsigned short)seq2
               << (unsigned  long)uin_num
  	       << (unsigned  long)0x0000;
  	    
   PutKey(reply_pack, calculate_checkcode(reply_pack));
    
   /* we do not need to encrypt packet */

   reply_pack.from_ip   = pack.from_ip;
   reply_pack.from_port = pack.from_port;
   
   /* send packet to client */
   udp_send_direct_packet(reply_pack);

}


/**************************************************************************/
/* Send busy packet to user					   	  */
/**************************************************************************/
void v5_send_end_contact(struct online_user &user)
{
   reply_pack.clearPacket();
   
   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)user.session_id
  	       << (unsigned short)ICQ_CMDxSND_USERxLISTxDONE
               << (unsigned short)user.servseq
               << (unsigned short)0x0000
               << (unsigned  long)user.uin
  	       << (unsigned  long)0x0000
  	       << (unsigned  long)user.uin;
  	    
   PutKey(reply_pack, calculate_checkcode(reply_pack));
    
   /* we do not need to encrypt packet */
   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}


/**************************************************************************/
/* OLD Search mode result packet 				   	  */
/**************************************************************************/
void v5_send_old_search_found(unsigned short seq2, struct online_user &user, 
			      struct full_user_info &tuser)
{
   reply_pack.clearPacket();

   reply_pack << (unsigned short)V5_PROTO
              << (char)0x00
              << (unsigned  long)user.session_id
  	      << (unsigned short)ICQ_CMDxSND_OLDxSEARCHxFIND
              << (unsigned short)user.servseq
              << (unsigned short)seq2
              << (unsigned  long)user.uin
  	      << (unsigned  long)0x0000
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
  	      << (char)0x00;

   PutKey(reply_pack, calculate_checkcode(reply_pack));
    
   /* we do not need to encrypt packet */
   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client */  
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}


/**************************************************************************/
/* Old-style search end marker packet					  */
/**************************************************************************/
void v5_send_old_search_end(unsigned short seq2, struct online_user &user, 
			    BOOL more)
{
   reply_pack.clearPacket();
   
   reply_pack << (unsigned short)V5_PROTO
              << (char)0x00
              << (unsigned  long)user.session_id
 	      << (unsigned short)ICQ_CMDxSND_OLDxSEARCHxEND
              << (unsigned short)user.servseq
              << (unsigned short)seq2
              << (unsigned  long)user.uin
 	      << (unsigned  long)0x0000;
   if (more)
   {
      reply_pack << (char)0x01;
   }
   else
   {
      reply_pack << (char)0x00;
   }

   PutKey(reply_pack, calculate_checkcode(reply_pack));
    
   /* we do not need to encrypt packet */
   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* Send packet to client */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);

}


/**************************************************************************/
/* Send user disconnect packet to user				   	  */
/**************************************************************************/
void v5_send_srv_disconnect(struct online_user &user)
{
   reply_pack.clearPacket();
   
   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)user.session_id
  	       << (unsigned short)ICQ_CMDxSND_BUSY
               << (unsigned short)0x0000
               << (unsigned short)user.servseq2
               << (unsigned  long)user.uin
  	       << (unsigned  long)0x0000;
  	    
   /* we do not need to encrypt packet */
   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client */
   v5_send_indirect(reply_pack, user.uin, user.shm_index);
}

