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
/* Socket processor (SP) routines (generic poll() version)		  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#ifdef USE_POLL

int nsockets = 0;
int csockets = 0;

/**************************************************************************/
/* func, managing udp, unix and tcp connections using poll function 	  */
/* also it is manage server child processes (watchdog, restart)		  */
/**************************************************************************/
void process_socket()
{
   pack_processed = 0;		/* number of processed by server packets  */
   Packet upacket;		/* udp socket processor temporal packet   */
   Packet wpacket;		/* unix wwp socket temporal packet	  */
   Packet tpacket;		/* tcp socket temporal packet		  */
   
   /* initialization */
   flap_init();
   watchdog_init();
   poll_table_init();

   LOG_SYS(10, ("Init: Ready to serve requests on sockets [poll]\n"));
   
   while(1)
   {
      csockets = 0;

      /* poll sockets array waiting for network events */
      if (((nsockets = poll(sock_fds, tcp_sock_count, -1)) < 1) && (errno != EINTR))
      {
         LOG_SYS(10, ("Problem with poll(): %s\n", strerror(errno)));
      }

      curr_time = (unsigned long)time(NULL);

      /* check if we have incoming client udp/wwp messages */
      if (isready_data(STOG)) 
      {
         while (tog_process(upacket) > 0) {};
	 csockets++;
      }

      if ((csockets < nsockets) && isready_data(SUDP)) 
         { udp_process(upacket); csockets++; }

      if ((csockets < nsockets) && isready_data(SAIM))
         { aim_accept_connect(); csockets++; }	 

      if ((csockets < nsockets) && isready_data(SMSN))
         { msn_accept_connect(); csockets++; }

      if ((csockets < nsockets) && isready_data(SWWP))
         { wwp_process(wpacket); csockets++; }
	 
      if (csockets < nsockets) { check_accepted_connections(); }
      
      /* check for dead and hung childs */
      if ((curr_time - old_time) > 0)
      {
         watchdog_check();
         check_sockets_timeout();
         prchilds_check();
         if (process_role != ROLE_SOCKET) return;
         old_time = curr_time;
      }
   }
}


/**************************************************************************/
/* Check all accepted sockets for data/errors after poll function	  */
/**************************************************************************/
void check_accepted_connections()
{
   /* Begin from end of matrix to reduce cpu load during login sequence   */
   for (int i=tcp_sock_count-1; i>=RES_SLOTS; i--)
   {
      if (csockets >= nsockets) return;
      if (sock_fds[i].revents)
      {
         /* check if we can read data from socket */
         if (isready_data(i))
         {
            csockets++;
	    tcp_process(i);
         }
         else
         {
            /* check if there is error occupied on socket */
            if (isready_error(i))
	    {
   	       csockets++;
	       DEBUG(10, ("Closing socket (%d/%d) on error [rev=%04X]\n", 
	                   i, tcp_sock_count, sock_fds[i].revents));
			   
	       close_socket_index(i, sock_inf[i].rnd_id);
            }
         }
      }
   }
}


/**************************************************************************/
/* Close socket, remove it from array and defragment it (array)		  */
/**************************************************************************/
void close_socket_index_r(int index, unsigned long rnd_id)
{
   if ((index + 1) >  tcp_sock_count) return;
   if (rnd_id != sock_inf[index].rnd_id) return;   

   DEBUG(100, ("Removing socket: [%s:%d],[i=%d/%d],[rnd=%lu],[ssec=%d]\n", 
	     inet_ntoa(sock_inf[index].cli_addr.sin_addr), ntohs(sock_inf[index].cli_addr.sin_port), 
	     index, tcp_sock_count, sock_inf[index].rnd_id, sock_inf[index].aim_srv_seq));

   close(sock_fds[index].fd);
   tcp_sock_count--;
   
   /* delete socket from index */   
   if ((index) != tcp_sock_count) 
   {
      /* well, we should move last socket data to this socket and dec cnt */
      sock_fds[index] = sock_fds[tcp_sock_count];
      sock_inf[index] = sock_inf[tcp_sock_count];
   }

   sock_fds[tcp_sock_count].fd = -1;
}


#endif 

