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
/* TCP server starting, stopping, restarting. Packets receiving/sending   */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* AIM protocol TCP server port creation 				  */
/**************************************************************************/
int aim_tcp_server_start(int port_number)
{
   struct sockaddr_in a_server;
   int bindcnt = 0;
   BOOL bindFailed = False;
   int val = 1;
  
   if ((aimsockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      LOG_SYS(0, ("WARNING: Can't create aim/v7 tcp socket...\n"));
   }

   memset(&a_server, 0, sizeof(a_server));
   a_server.sin_family = AF_INET;
   a_server.sin_addr = bind_interface;
   a_server.sin_port = htons(port_number);

   setsockopt(aimsockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));   
   
   while (bindcnt < MAX_BIND_TRY)
   {
     if (bind(aimsockfd, (struct sockaddr *)&a_server, 
	      sizeof(struct sockaddr)) < 0)
     {
        bindFailed = True;
        sleep(1);
        bindcnt++;
     }
     else
     {
        bindFailed = False;
        break;
     }			
   }
  		
   if (bindFailed)
   {
      LOG_SYS(0, ("ERROR: Can't bind aim/v7 tcp socket...\n"));
      LOG_SYS(3, ("       problem: \"%s\"\n",   strerror(errno)));
      LOG_SYS(3, ("       bind addr: [%s], bind port [%d]\n", 
			  inet_ntoa(bind_interface),  port_number)); 
      close(aimsockfd);
      aimsockfd = -1;  
      
      /* exit(EXIT_ERROR_ANOTHER_PROCESS); */
   }

   if (listen(aimsockfd, -1) < 0) 
   {
      LOG_SYS(0, ("ERROR: Can't listen aim tcp socket, exiting...\n"));
      LOG_SYS(3, ("       problem: \"%s\"\n",   strerror(errno)));
      close(aimsockfd);
      aimsockfd = -1;
      
   }
   else
   {   
      LOG_SYS(3, ("Init: TCP server (OSCAR) started on [%s:%d]\n", 
                  inet_ntoa(a_server.sin_addr), port_number));
   }

   return(aimsockfd);
}


/**************************************************************************/
/* MSN protocol TCP server port creation 				  */
/**************************************************************************/
int msn_tcp_server_start(int port_number)
{
   struct sockaddr_in m_server;
   int bindcnt = 0;
   BOOL bindFailed = False;
  
   if ((msnsockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      LOG_SYS(0, ("WARNING: Can't create MSN tcp socket...\n"));
   }

   memset(&m_server, 0, sizeof(m_server));
   m_server.sin_family = AF_INET;
   m_server.sin_addr = bind_interface;
   m_server.sin_port = htons(port_number);
   
   while (bindcnt < MAX_BIND_TRY)
   {
     if (bind(msnsockfd, (struct sockaddr *)&m_server, 
	      sizeof(struct sockaddr)) < 0)
     {
        bindFailed = True;
        sleep(1);
        bindcnt++;
     }
     else
     {
        bindFailed = False;
        break;
     }			
   }
  		
   if (bindFailed)
   {
      LOG_SYS(0, ("ERROR: Can't bind MSN tcp socket...\n"));
      LOG_SYS(3, ("       problem: \"%s\"\n",   strerror(errno)));
      LOG_SYS(3, ("       bind addr: [%s], bind port [%d]\n", 
			  inet_ntoa(bind_interface),  port_number)); 
      close(msnsockfd);
      msnsockfd = -1;  
      
      /* exit(EXIT_ERROR_ANOTHER_PROCESS); */
   }

   if (listen(msnsockfd, -1) < 0) 
   {
      LOG_SYS(0, ("ERROR: Can't listen MSN tcp socket, exiting...\n"));
      LOG_SYS(3, ("       problem: \"%s\"\n",   strerror(errno)));
      close(msnsockfd);
      msnsockfd = -1;
      
   }
   else
   {   
      LOG_SYS(3, ("Init: Listening for MSN protocol on [%s:%d]\n", 
                  inet_ntoa(m_server.sin_addr), port_number));
   }

   return(aimsockfd);
}


/**************************************************************************/
/* this used by socket processor to receive arrived data 		  */
/**************************************************************************/
void tcp_receive_packet(int index, Packet &pack)
{
   pack.clearPacket();

   /* Here we receiving client data */
   pack.sizeVal      = recv(sock_fds[index].fd, pack.buff, MAX_PACKET_SIZE, 0);
   pack.sock_evt     = SOCK_DATA;
   pack.sock_rnd     = sock_inf[index].rnd_id;
   pack.sock_type    = sock_inf[index].sock_type;
   pack.from_ip      = sock_inf[index].cli_addr.sin_addr;
   pack.from_port    = ntohs(sock_inf[index].cli_addr.sin_port);
   pack.flap_channel = 0;
   pack.net_order    = 0; /* default value */
   
   /* sanity check, also we don't care about REFUSED, INTR & BLOCK errors */
   if (pack.sizeVal == -1) pack.sizeVal = 0;
}


/**************************************************************************/
/* Function that check in non-blocking mode if there is data arrived	  */
/* used in flap protocol parser to avoid dos attack based on mailformed   */
/* flap packet (correct header without data block)			  */
/**************************************************************************/
BOOL ready_data(int sock_hdl)
{
   struct pollfd sock_chk;
   
   sock_chk.fd     = sock_hdl;
   sock_chk.events = POLLIN;
   
   if (poll(&sock_chk, 1, 0) != 1) return False;
   
   return True;
}


/**************************************************************************/
/* Function to wait on socket until we get POLLOUT			  */
/**************************************************************************/
int wait4write(int sock_hdl, int time)
{
   struct pollfd sock_chk;
   
   sock_chk.fd     = sock_hdl;
   sock_chk.events = POLLOUT;
   return(poll(&sock_chk, 1, time));
}


/**************************************************************************/
/* Function to wait on socket until we get POLLIN			  */
/**************************************************************************/
int wait4read(int sock_hdl, int time)
{
   struct pollfd sock_chk;
   
   sock_chk.fd     = sock_hdl;
   sock_chk.events = POLLIN;
   return(poll(&sock_chk, 1, time));
}


