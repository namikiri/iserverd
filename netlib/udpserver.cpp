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
/* UDP server starting, stopping, restarting. Packets receiving/sending   */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* this used by socket processor to receive arrived data 		  */
/**************************************************************************/
void udp_receive_packet(Packet &pack)
{
   /* This we need to obtain sender addr */
   struct sockaddr_in client_addr;

   /* clear structure */
   memset(&client_addr, 0, sizeof(struct sockaddr_in));
   socklen_t caddr_len = sizeof(client_addr);

   /* Here we receiving client data */
   pack.sizeVal = recvfrom(msockfd, pack.buff, MAX_PACKET_SIZE, 0,
                          (sockaddr *)&client_addr, &caddr_len);

   /* sanity check, also we don't care about REFUSED, INTR & BLOCK errors */
   if ((pack.sizeVal == -1) &&
       (errno != ECONNREFUSED) &&
       (errno != EINTR) &&
       (errno != EWOULDBLOCK) &&
       (errno != EHOSTUNREACH))
   {
      LOG_SYS(0, ("READSOCK: error code: [%s]\n", strerror(errno)));
      exit(EXIT_ERROR_FATAL);
   }

   /* fill packet with system data */
   pack.from_ip      = client_addr.sin_addr;
   pack.from_port    = ntohs(client_addr.sin_port);
   pack.sock_evt     = SOCK_DATA;
   pack.sock_type    = SUDP;
   pack.sock_hdl     = 0;
   pack.sock_rnd     = 0;
   pack.net_order    = 0;
   pack.from_local   = 0;
   pack.flap_channel = 0;
   pack.asciiz       = 1;

}


/**************************************************************************/
/* this used by utils in non-blocking call		 		  */
/**************************************************************************/
BOOL udp_recv_pack(Packet &pack)
{
   /* This we need to obtain sender addr */
   struct sockaddr_in client_addr;

   /* clear structure */
   memset(&client_addr, 0, sizeof(struct sockaddr_in));
   socklen_t caddr_len = sizeof(client_addr);

   pack.clearPacket();

   /* Here we receiving client data */
   pack.sizeVal = recvfrom(msockfd, pack.buff, MAX_PACKET_SIZE, 0,
                          (sockaddr *)&client_addr, &caddr_len);

   /* sanity check, also we don't care about REFUSED, INTR & BLOCK errors */
   if (pack.sizeVal == -1)
   {
      return (False);
   }

   /* fill packet with client addr */
   pack.from_ip   = client_addr.sin_addr;
   pack.from_port = ntohs(client_addr.sin_port);

   return (True);
}


/**************************************************************************/
/* This used by packet processors and event processor to send reply 	  */
/* packets to client 							  */
/**************************************************************************/
void udp_send_direct_packet(Packet &pack)
{
   struct sockaddr_in cli_addr;

   cli_addr.sin_addr = pack.from_ip;
   cli_addr.sin_port = htons(pack.from_port);
   cli_addr.sin_family = AF_INET;

   if (sendto(msockfd, pack.buff, pack.sizeVal, 0,
             (sockaddr *)&cli_addr, sizeof(struct sockaddr_in)) == -1)
   {
      if (errno == EACCES)
          { LOG_ALARM(0, ("WARNING: broadcast address [ %s ]\n", inet_ntoa(cli_addr.sin_addr) ));}
      if (errno == EHOSTUNREACH)
          { LOG_ALARM(0, ("WARNING: address unreachable [ %s ]\n", inet_ntoa(cli_addr.sin_addr) ));}
   }
}


/**************************************************************************/
/* Make new socket with new port and interface				  */
/**************************************************************************/
int udpserver_start(int sock_port, int log=3)
{
   struct sockaddr_in i_server;
   int bindcnt = 0;
   BOOL bindFailed = True;

   old_sock_port = sock_port;

   if ((msockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
   {
      LOG_SYS(0, ("ERROR: Can't create socket, exiting...\n"));
      exit(EXIT_ERROR_FATAL);
   }

   memset(&i_server, 0, sizeof(i_server));
   i_server.sin_family = AF_INET;
   i_server.sin_addr = bind_interface;
   i_server.sin_port = htons(sock_port);

   bindcnt = 0;

   while (bindcnt < MAX_BIND_TRY)
   {
     if (bind(msockfd, (struct sockaddr *)&i_server,
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
      LOG_SYS(0, ("ERROR: Can't bind main socket, exiting...\n"));
      LOG_SYS(log, ("       problem: \"%s\"\n",   strerror(errno)));
      LOG_SYS(log, ("       bind addr: [%s], bind port [%d]\n",
			  inet_ntoa(bind_interface),  sock_port));
      exit(EXIT_ERROR_ANOTHER_PROCESS);
   }

   LOG_SYS(log, ("Init: UDP server (S/ICQ) started on [%s:%d]\n",
                inet_ntoa(i_server.sin_addr), sock_port));

   return(msockfd);
}


/**************************************************************************/
/* Close socket								  */
/**************************************************************************/
int udpserver_stop()
{
   shutdown(msockfd, 2);
   close(msockfd);
   return(0);
}


/**************************************************************************/
/* Close socket and make new one with new port and interface		  */
/**************************************************************************/
int udpserver_restart(int sock_port)
{
   if ((ipToIcq(old_interface) != ipToIcq(bind_interface)) |
       (sock_port != old_sock_port))
   {
      udpserver_stop();
      udpserver_start(sock_port);
      return(0);
   }

   LOG_SYS(0, ("There is no need to restart UDP server....\n"));
   return(1);
}


/**************************************************************************/
/* this used by socket processor to receive wwp messages from socket	  */
/**************************************************************************/
void wwp_receive_packet(Packet &pack)
{
   /* This we need to obtain sender addr */
   struct sockaddr_in client_addr;

   /* clear structure */
   memset(&client_addr, 0, sizeof(struct sockaddr_in));
   socklen_t caddr_len = sizeof(client_addr);
   pack.clearPacket();

   /* Here we receiving client data */
   pack.sizeVal = recvfrom(wwpsockfd, pack.buff, MAX_PACKET_SIZE, 0,
                          (sockaddr *)&client_addr, &caddr_len);

   /* sanity check, also we don't care about REFUSED, INTR & BLOCK errors */
   if ((pack.sizeVal == -1) &&
       (errno != ECONNREFUSED) &&
       (errno != EINTR) &&
       (errno != EWOULDBLOCK))
   {
      DEBUG(0, ("WWPSOCK: error code: [%s]\n", strerror(errno)));
      pack.sizeVal = 0;
   }

   /* fill packet with client addr */
   pack.from_ip   = icqToIp(0);
   pack.from_port = 0;

}

