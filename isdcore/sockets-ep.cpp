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
/* Socket processor (SP) routines (Linux epoll version)			  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#ifdef USE_EPOLL

struct epoll_event ev;
struct epoll_event *sock_kev = NULL;
int kernq    = 0;
int nsockets = 0;
void kernel_queue_init();


/**************************************************************************/
/* func, managing udp, unix and tcp connections using poll function 	  */
/* also it is manage server child processes (watchdog, restart)		  */
/**************************************************************************/
void process_socket()
{
   int i;
   unsigned short udata_index = 0;
   
   pack_processed = 0;		/* number of packets processed by server  */
   Packet upacket;		/* udp socket processor receiver packet   */
   Packet wpacket;		/* unix wwp socket receiver packet	  */
   Packet tpacket;		/* tcp socket receiver packet		  */
   
   flap_init();
   watchdog_init();
   poll_table_init();
   kernel_queue_init();

   LOG_SYS(10, ("Init: Ready to serve requests on sockets [epoll]\n"));
   
   while(1)
   {
      nsockets = epoll_wait(kernq, sock_kev, KQNEVENTS, -1);
      curr_time = (unsigned long)time(NULL);
      
      if ((nsockets <= 0) && (errno != EINTR))
      { 
         LOG_SYS(0, ("Error: epoll_wait problem: %s\n", strerror(errno)));
      }

      while (tog_process(upacket) > 0) {}; /* tog needs special handling */
      
      for(i=0; i<nsockets; i++)
      {
         udata_index = (unsigned short)sock_kev[i].data.fd;
	 
         /* socket errors handler */
         if(sock_kev[i].events & EPOLLERR)
	 {
	    if (udata_index >= RES_SLOTS)
	    {
	       DEBUG(10, ("[ep] Closing socket (%d/%d) on error (fd=%d)\n", 
	                   udata_index, tcp_sock_count, sock_fds[udata_index].fd));
               close_socket_index(udata_index, sock_inf[udata_index].rnd_id);
    
            }
            else
	    {
	       LOG_SYS(0, ("[ep] Sys-socket error (!) Closing socket %d\n", udata_index));
	       close(sock_fds[i].fd);
	    }
	    
	    continue;
	 }

	 if(sock_kev[i].events & EPOLLIN)
	 {
	    if (udata_index == SUDP) { udp_process(upacket); }
	    if (udata_index == SAIM) { aim_accept_connect(); }
	    if (udata_index == SMSN) { msn_accept_connect(); }
	    if (udata_index == SWWP) { wwp_process(wpacket); }
	    if (udata_index >= RES_SLOTS) { tcp_process(udata_index); }
	 }
      }

      /* check for tog packets again */
      while (tog_process(upacket) > 0) {};
      
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
/* Close socket, remove it from array and defragment it (array)		  */
/**************************************************************************/
void close_socket_index_r(int index, unsigned long rnd_id)
{
   if ((index + 1) >  tcp_sock_count) return;
   if (rnd_id != sock_inf[index].rnd_id) return;   

   DEBUG(100, ("Removing socket: [%s:%d],[i=%d/%d],[rnd=%lu],[ssec=%d]\n", 
	     inet_ntoa(sock_inf[index].cli_addr.sin_addr), ntohs(sock_inf[index].cli_addr.sin_port), 
	     index, tcp_sock_count, sock_inf[index].rnd_id, sock_inf[index].aim_srv_seq));

   tcp_sock_count--;
   
   /* we should delete it by hands - this fd may be cloned by fork */
   epoll_ctl(kernq, EPOLL_CTL_DEL, sock_fds[index].fd, &ev);
   close(sock_fds[index].fd);

   /* delete socket from index */   
   if ((index) != tcp_sock_count) 
   {
      sock_fds[index] = sock_fds[tcp_sock_count];
      sock_inf[index] = sock_inf[tcp_sock_count];
      
      /* now we should update epoll event object */
      ev.events = EPOLLIN | EPOLLERR;
      ev.data.fd = index;

      if (epoll_ctl(kernq, EPOLL_CTL_MOD, sock_fds[index].fd, &ev) < 0)
      {
         DEBUG(10, ("Error: epoll_ctl mod - %s\n", strerror(errno)));
	 return;
      }
      
      /* correct unprocessed events data */
      for(int i=0; i<nsockets; i++)
      {
         if (sock_kev[i].data.fd == tcp_sock_count)
	 {
	    sock_kev[i].data.fd = index;
	 }
      }
   }

  /* it is the last index - we can remove last and dec counter */
  sock_fds[tcp_sock_count].fd = -1;
      
}


/**************************************************************************/
/* FreeBSD kernel queue initialization	 				  */
/**************************************************************************/
void kernel_queue_init()
{
   sock_kev = new struct epoll_event[KQNEVENTS];

   /* Create a new epoll kernel queue and init descriptor */
   if ((kernq = epoll_create(lp_max_tcp_connections()/2 + RES_SLOTS)) == -1)
   {
      LOG_SYS(0, ("Error: Can't create epoll queue: %s\n", strerror(errno)));
      exit(EXIT_ERROR_FATAL);
   }

   /* Add initialized system sockets to epoll */
   for (int i=0; i<RES_SLOTS; i++)
   {
      if (sock_fds[i].fd != -1)
      {
         ev.events = EPOLLIN | EPOLLERR;
         ev.data.fd = i;

         if (epoll_ctl(kernq, EPOLL_CTL_ADD, sock_fds[i].fd, &ev) < 0) 
         {
            DEBUG(10, ("Error: epoll_ctl can't init socket %s\n", strerror(errno)));
	    return;
         }
      }
   }
}


/**************************************************************************/
/* Added accepted socket to kernel queue (for common accept functions) 	  */
/**************************************************************************/
void accept_add_object(int index)
{
   if (sock_fds[index].fd != -1)
   {
      ev.events = EPOLLIN | EPOLLERR;
      ev.data.fd = index;

      if (epoll_ctl(kernq, EPOLL_CTL_ADD, sock_fds[index].fd, &ev) < 0) 
      {
         DEBUG(10, ("Error: epoll_ctl can't add socket %s\n", strerror(errno)));
	 return;
      }
   }
}


#endif

