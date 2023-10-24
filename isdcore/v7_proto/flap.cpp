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
/* Handle AIM/OSCAR low-level transport (FLAP) user packets		  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
int last_ind = 0;               /* last sent packet socket index     */


/**************************************************************************/
/* This function add flap header and send result packet to client	  */
/* Warning: called from socket processor 				  */
/**************************************************************************/
void flap_send_packet(Packet &pack)
{
   int sresult;
   int from_ind = tcp_sock_count-1;

   /* optimization for search results/userinfo */
   if ((pack.sock_hdl == sock_fds[last_ind].fd) && 
       (pack.sock_rnd == sock_inf[last_ind].rnd_id))
   {
      from_ind = last_ind;
   }
   
   /* optimization for login contacts notifications */
   for (int i=from_ind; i>=11; i--)
   {
      if ((pack.sock_hdl == sock_fds[i].fd) && 
          (pack.sock_rnd == sock_inf[i].rnd_id))
      {
	 last_ind = i;
	 
         if (pack.sock_evt == SOCK_RDY) sock_inf[i].aim_started = 0;
	 pipe_pack.clearPacket();
	 pipe_pack.network_order();

	 sock_inf[i].aim_srv_seq++;
	 pipe_pack << (char)FLAP_ID_BYTE
	           << (char)pack.flap_channel
		   << (unsigned short)sock_inf[i].aim_srv_seq
		   << (unsigned short)pack.sizeVal;

	 memcpy(pipe_pack.nextData, pack.buff, pack.sizeVal);
	 pipe_pack.sizeVal += pack.sizeVal;

         /* we know index so we should now send packet to this socket */
	 sresult = sendto(sock_fds[i].fd, pipe_pack.buff, pipe_pack.sizeVal, 0, NULL, 0);
	
	 if (sresult < 0)
	 {
	    if ((errno == EAGAIN) || (errno == EWOULDBLOCK) ||
	        (errno == EINTR)  || (errno == ENOMEM) || 
		(errno == ENOBUFS))
	    {
	       wait4write(sock_fds[i].fd, 5); /* 5ms delay, grrrrr */
	       sendto(sock_fds[i].fd, pipe_pack.buff, pipe_pack.sizeVal, 0, NULL, 0);
	       return;
	    }
	 
	    DEBUG(10, ("FLAP: Closing socket on send error... (%s)\n", strerror(errno)));
            close_socket_index(i, sock_inf[i].rnd_id); 
	 }
	 
	 return;
      }
   }
}


/**************************************************************************/
/* This function strip flap header and get all data from it		  */
/* Warning: called from socket processor 				  */
/**************************************************************************/
void flap_recv_packet(int index, Packet &pack)
{
   char flap_id;
   char flap_channel;
   unsigned short flap_cli_seq;
   unsigned short flap_buf_len;
   
   /* we have index so we can read flap header from socket */
   pack.sizeVal = recv(sock_fds[index].fd, pack.buff, FLAP_HRD_SIZE, 0);

   if (pack.sizeVal < 0)
   {
      DEBUG(50, ("Error at recv flap-header (%d)(%d - %s)\n", pack.sizeVal, 
                  errno, strerror(errno)));
   }
   
   pack.reset();
   pack.network_order(); /* AIM uses network byteorder */
   pack.sock_rnd = sock_inf[index].rnd_id;
   pack.flap_channel = 3;
   
   if (pack.sizeVal != 6) { pack.sizeVal = 0; return; }
   
   pack >> flap_id
        >> flap_channel
        >> flap_cli_seq
        >> flap_buf_len;
   
   /* we need this here because of keep_alive flaps */
   pack.flap_channel = flap_channel;
   
   /* if it is first client packet - we should take cli_seq from it      */
   if (sock_inf[index].aim_started == 2) 
   {
      sock_inf[index].aim_started = 1; /* started, but client not ready  */
      sock_inf[index].aim_cli_seq = flap_cli_seq - 1;
   }
   
   /* check for flap header marker */
   if (flap_id != FLAP_ID_BYTE) { pack.sizeVal = 0; return; }   

   sock_inf[index].aim_cli_seq++;

   /* check if sequence match saved for this socket */
   if (flap_cli_seq != sock_inf[index].aim_cli_seq)
   {
      DEBUG(50, ("Client flap sequence error, shuting down connection\n"));
      pack.sizeVal = 0;
      return;
   }
   
   /* check for packet length */
   if (flap_buf_len == 0)
   { 
      pack.sizeVal = 0; return; 
   }
   
   if (flap_buf_len > MAX_PACKET_SIZE) 
   { 
      pack.sizeVal = 0; return; 
   }
   
   /* so flap header is correct. good. now we should check if there  */
   /* is data in the socket to avoid blocking when an attacker write */
   /* 6 bytes to socket with flap_buf_len > 0 */
   if (ready_data(sock_fds[index].fd))
   {
      pack.sizeVal = recv(sock_fds[index].fd, pack.buff, flap_buf_len, 0);
      if (pack.sizeVal != flap_buf_len) { pack.sizeVal = 0; return; }
      
      /* now we have ready packet without FLAP header, lets fill info */
      pack.flap_channel = flap_channel;
      pack.sock_rnd     = sock_inf[index].rnd_id;
      pack.sock_type    = SAIM;
      pack.sock_evt	= SOCK_DATA;
      pack.from_port    = ntohs(sock_inf[index].cli_addr.sin_port);
      pack.from_ip	= sock_inf[index].cli_addr.sin_addr;
      pack.sock_hdl     = sock_fds[index].fd;
      
   }
   else
   {
      /* it seems to me that somebody trying to hack us... :( */
      LOG_SYS(0, ("Warning: mailformed FLAP packet from %s:%d\n", 
                  inet_ntoa(sock_inf[index].cli_addr.sin_addr), 
		  ntohs(sock_inf[index].cli_addr.sin_port)));
		  
      pack.sizeVal = 0;
      return;
   }
}


/**************************************************************************/
/* This function init flap parser 					  */
/* Warning: called at the starting up	 				  */
/**************************************************************************/
void flap_init()
{
   /* not used yet */
}

