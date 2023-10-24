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
/* This module contain functions that create reply packets for AIM proto  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* This func create and send connection acknownledge packet to client     */
/**************************************************************************/
void send_connection_accepted(Packet &pack)
{
   pack.clearPacket();
   pack.network_order();
   pack.sock_evt = SOCK_DATA;
   pack << (unsigned long)FLAP_VERSION;
   
   tcp_writeback_packet(pack);
}


/**************************************************************************/
/* This func create and send authorize fail packet to client     	  */
/**************************************************************************/
void send_authorize_fail(Packet &pack, char *error_text, char *scr_name, 
			 unsigned short error_code)
{
   pack.clearPacket();
   pack.setup_aim();
   
   pack << (unsigned short)0x0001       /* TLV type 1 - screen name */ 
        << (unsigned short)(strlen(scr_name))
	<< scr_name
   
        << (unsigned short)0x0008	/* TLV type 8 - error code  */
        << (unsigned short)sizeof(error_code)
	<< (unsigned short)error_code;
	
   if (strlen(error_text) > 0)
   {
      pack << (unsigned short)0x0004	/* TLV type 4 - error url */
   	   << (unsigned short)(strlen(error_text))
	   << error_text;
   }
	
   pack << (unsigned short)0x000C	/* TLV type C - unknown   */
	<< (unsigned short)0x0002
	<< (unsigned short)0x0001;
	
   pack.flap_channel = 0x04;
   pack.sock_evt     = SOCK_DATA;
   tcp_writeback_packet(pack);
   close_connection(pack);
}


/**************************************************************************/
/* This func create and send auth cookie / BOS address to client     	  */
/**************************************************************************/
void send_auth_cookie(Packet &pack, char *screen_name, char *srv_addr, 
                      char *cookie, unsigned short cookie_len)
{
   pack.clearPacket();
   pack.setup_aim();
   
   pack << (unsigned short)0x0001       /* TLV type 1 - screen name */ 
        << (unsigned short)(strlen(screen_name))
	<< screen_name;   
      
   pack << (unsigned short)0x0005       /* TLV type 5 - BOS addr:port */ 
        << (unsigned short)(strlen(srv_addr))
	<< srv_addr;
   
   pack << (unsigned short)0x0006	/* TLV type 6 - auth cookie */
	<< (unsigned short)cookie_len;

   /* put all cookie to packet byte for byte (it is not a string) */
   for (int i=0; i<cookie_len; i++) pack << (char)cookie[i];

   pack.sock_evt     = SOCK_DATA;   
   pack.flap_channel = 0x04;
   tcp_writeback_packet(pack);
}


/**************************************************************************/
/* This func create and send session fail packet			  */
/**************************************************************************/
void send_session_fail(Packet &pack, char *error_text, unsigned short error_code)
{
   pack.clearPacket();
   pack.network_order();
   pack.no_null_terminated();
   
   pack << (unsigned short)0x0009	/* TLV type 9 - error code  */
        << (unsigned short)0x0002
	<< (unsigned short)error_code;
	
   if (strlen(error_text) > 0)	
   {
      pack << (unsigned short)0x000B	/* TLV type B - error text */
   	   << (unsigned short)(strlen(error_text))
	   << error_text;
   }
   
   DEBUG(10, ("Sending session_error packet with code=%d\n", error_code));   
   pack.flap_channel = 0x04;
   pack.sock_evt     = SOCK_DATA;
   tcp_writeback_packet(pack);
   close_connection(pack);
}


/**************************************************************************/
/* This func create and send srv families packet			  */
/**************************************************************************/
void send_srv_families(Packet &pack, struct online_user &user)
{
   struct aim_family *bam = aim_root.aim_families;
   
   pack.clearPacket();
   pack.network_order();
   pack.no_null_terminated();

   pack << (unsigned short)SN_TYP_GENERIC
        << (unsigned short)SN_GEN_SERVERxFAMILIES
	<< (unsigned short)0x0000
	<< (unsigned  long)(user.servseq2-1);
	
   /* Ok. Now we should itterate thru V7 config tree and gather families */
   while (bam)
   {
      if (bam->number != SN_TYP_REGISTRATION)
      {
         pack << (unsigned short)bam->number;
      }

      bam = bam->next;
   }
     
   DEBUG(200, ("Sending server supported families packet\n"));
   pack.flap_channel = 0x02;
   pack.sock_evt     = SOCK_RDY;
   
   tcp_writeback_packet(pack);
}


/**************************************************************************/
/* This func create and send srv pause packet (migration start)		  */
/* Migration: first server send srv_pause packet, client should respond   */
/* with groups he wants for this connection. After that server send       */
/* new BOS address and cookie. Client should disconnect and connect again */
/* to the new BOS server with given cookie and login to it as ussual      */
/**************************************************************************/
void send_srv_pause(struct online_user *user)
{
   reply_pack.clearPacket();
   reply_pack.network_order();
   reply_pack.no_null_terminated();
   
   reply_pack << (unsigned short)SN_TYP_GENERIC
              << (unsigned short)SN_GEN_PAUSE
              << (unsigned short)0x0000
              << (unsigned  long)user->servseq2++;
	
   reply_pack.from_ip      = user->ip;
   reply_pack.from_port    = 0x0000;
   reply_pack.sock_hdl     = user->sock_hdl;
   reply_pack.sock_rnd     = user->sock_rnd;
   reply_pack.sock_evt     = SOCK_DATA;
   reply_pack.sock_type    = SAIM;
   reply_pack.flap_channel = 0x02;

   tcp_writeback_packet(reply_pack);

}


/**************************************************************************/
/* This func create and send server migrate request to client     	  */
/* Client should disconnect and connect to new server specified in TLV(5) */
/**************************************************************************/
void send_srv_migrate(struct online_user *user, char *srv_addr, 
                      char *cookie, unsigned short cookie_len,
		      unsigned short *families, unsigned short fam_number)
{
   reply_pack.clearPacket();
   reply_pack.setup_aim();

   reply_pack << (unsigned short)SN_TYP_GENERIC
              << (unsigned short)SN_GEN_MIGRATION
	      << (unsigned short)0x0000
	      << (unsigned  long)user->servseq2++;
      
   reply_pack << (unsigned short)fam_number;
   
   for (int i=0; i<fam_number; i++)
   {
      reply_pack << (unsigned short)families[i];   
   }
      
   reply_pack << (unsigned short)0x0005
              << (unsigned short)(strlen(srv_addr))
	      << srv_addr;
   
   reply_pack << (unsigned short)0x00006
	      << (unsigned short)cookie_len;

   /* put all cookie to packet byte for byte (it is not a string) */
   for (int i=0; i<cookie_len; i++) reply_pack << (char)cookie[i];

   reply_pack.sock_evt     = SOCK_DATA;
   reply_pack.from_ip      = user->ip;
   reply_pack.from_port    = 0x0000;
   reply_pack.sock_hdl     = user->sock_hdl;
   reply_pack.sock_rnd     = user->sock_rnd;
   reply_pack.sock_type    = SAIM;
   reply_pack.flap_channel = 0x02;
   
   tcp_writeback_packet(reply_pack);
}


/**************************************************************************/
/* This func create and send srv motd packet				  */
/**************************************************************************/
void send_srv_motd(struct snac_header &snac, Packet &pack)
{
   pack.clearPacket();
   pack.network_order();
   pack.no_null_terminated();
   
   pack << (unsigned short)SN_TYP_GENERIC
        << (unsigned short)SN_GEN_MOTD
	<< (unsigned short)0x0000
	<< (unsigned  long)(snac.id+1)
	
	/* Unknown values (not explored yet) */
	<< (unsigned short)0x0005
	
	<< (unsigned short)0x0002
	<< (unsigned short)0x0002
	<< (unsigned short)0x001E
	
	<< (unsigned short)0x0003
	<< (unsigned short)0x0002
	<< (unsigned short)0x04B0;
	
   DEBUG(200, ("Sending server motd packet\n"));   
   pack.sock_evt     = SOCK_DATA;
   pack.flap_channel = 0x02;
   tcp_writeback_packet(pack);
}


/**************************************************************************/
/* This func create and send srv_disconnect packet			  */
/**************************************************************************/
void v7_send_srv_disconnect(struct online_user &user, unsigned short err_code, 
                            char *err_string)
{
   reply_pack.clearPacket();
   reply_pack.setup_aim();

   reply_pack << (unsigned short)0x0009 /* TLV 0x09 */
              << (unsigned short)sizeof(err_code)
   	      << (unsigned short)err_code
	
	      << (unsigned short)0x000b /* TLV 0x0B */
	      << (unsigned short)strlen(err_string)
	      << err_string;
	
   LOG_SYS(0, ("Online user %lu (oscar v7) was kicked off by new one...\n", user.uin));
   
   reply_pack.flap_channel = 0x04;
   reply_pack.sock_evt     = SOCK_DATA;
   reply_pack.from_ip      = user.ip;
   reply_pack.from_port    = 0;
   reply_pack.sock_hdl     = user.sock_hdl;
   reply_pack.sock_rnd     = user.sock_rnd;
   reply_pack.sock_type    = SAIM;
   
   tcp_writeback_packet(reply_pack);
}


/**************************************************************************/
/* This func create and send srv error packet (non-auth reply)		  */
/**************************************************************************/
void send_snac_error(unsigned short family, unsigned short errcode, 
                     unsigned long reqid, Packet &pack)
{
   pack.clearPacket();
   pack.network_order();
   pack.no_null_terminated();

   pack << (unsigned short)family
        << (unsigned short)0x0001
	<< (unsigned short)0x0000
	<< (unsigned  long)reqid
	<< (unsigned short)errcode;
	
   DEBUG(50, ("Sending error snac(%02X,%02X) - errcode %04X\n", 
              family, 1, errcode));

   pack.sock_evt = SOCK_DATA;
   tcp_writeback_packet(pack);
}


/**************************************************************************/
/* This func create and send srv error packet (auth reply)		  */
/**************************************************************************/
void send_snac_error(unsigned short family, unsigned short errcode, 
                     unsigned long reqid, struct online_user *user)
{
   arpack.clearPacket();
   arpack.network_order();
   arpack.no_null_terminated();

   arpack << (unsigned short)family
          << (unsigned short)0x0001
  	  << (unsigned short)0x0000
	  << (unsigned  long)reqid
	  << (unsigned short)errcode;
	
   DEBUG(50, ("Sending error snac(%02X,%02X) to %lu - errcode %04X\n", 
              family, 1, user->uin, errcode));

   arpack.sock_evt     = SOCK_DATA;
   arpack.from_ip      = user->ip;
   arpack.from_port    = 0x0000;
   arpack.sock_hdl     = user->sock_hdl;
   arpack.sock_rnd     = user->sock_rnd;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 0x02;

   tcp_writeback_packet(arpack);
}


