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
/* Socket processor (SP) routines (FreeBSD kqueue version)		  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#ifdef USE_KQUEUE

/* for FreeBSD version < 4.3 */
#ifndef EV_SET
#define EV_SET(kevp, a, b, c, d, e, f) do {     \
        (kevp)->ident = (a);                    \
        (kevp)->filter = (b);                   \
        (kevp)->flags = (c);                    \
        (kevp)->fflags = (d);                   \
        (kevp)->data = (e);                     \
        (kevp)->udata = (f);                    \
} while(0)
#endif

int nsockets = 0;
int nchanges = 0;
int kernq    = 0;
struct kevent *sock_kev = NULL;
struct kevent *sock_chg = NULL;
void kernel_queue_init();

/**************************************************************************/
/* func, managing udp, unix and tcp connections using kqueue function 	  */
/**************************************************************************/
void process_socket()
{
   int i;
   unsigned int udata_index = 0;

   pack_processed = 0;		/* number of processed by server packets  */
   Packet upacket;		/* udp socket processor temporal packet   */
   Packet wpacket;		/* unix wwp socket temporal packet	  */
   Packet tpacket;		/* tcp socket temporal packet		  */

   flap_init();
   watchdog_init();
   poll_table_init();
   kernel_queue_init();

   LOG_SYS(10, ("Init: Ready to serve requests on sockets [kqueue]\n"));

   while(1)
   {
      nsockets = kevent(kernq, sock_chg, nchanges, sock_kev, KQNEVENTS, 0);
      nchanges = 0;

      if ((nsockets <= 0) && (errno != EINTR))
      {
         LOG_SYS(0, ("Kernel queue error: %s\n", strerror(errno)));
      }

      while (tog_process(upacket) > 0) {}; /* tog needs special handling */

      for(i = 0; i < nsockets; i++)
      {
         udata_index = (unsigned int)sock_kev[i].udata;

         /* socket errors handler */
         if(sock_kev[i].flags & EV_ERROR)
	 {
            if ((int)sock_kev[i].ident == sock_fds[udata_index].fd)
	    {
	       if (udata_index >= RES_SLOTS)
	       {
	          DEBUG(10, ("[kq] Closing socket (%d/%d) on error\n",
	                      udata_index, tcp_sock_count));

	          close_socket_index(udata_index, sock_inf[udata_index].rnd_id);
               }
	       else
	       {
	          LOG_SYS(0, ("[kq] Sys-socket error (!) Closing socket %d\n", udata_index));
		  close(sock_kev[i].ident);
	       }
	    }

	    continue;
	 }

         /* events handler */
         switch(sock_kev[i].filter)
	 {
	    case EVFILT_READ:

	         if ((int)sock_kev[i].ident == sock_fds[udata_index].fd)
		 {
		    if (udata_index == SUDP) { udp_process(upacket); }
		    if (udata_index == SAIM) { aim_accept_connect(); }
		    if (udata_index == SMSN) { msn_accept_connect(); }
		    if (udata_index == SWWP) { wwp_process(wpacket); }
		    if (udata_index >= RES_SLOTS) { tcp_process(udata_index); }

                    break;
	         }

	    case EVFILT_TIMER:

	         curr_time = (unsigned long)time(NULL);

	         watchdog_check();
                 check_sockets_timeout();
                 prchilds_check();
                 if (process_role != ROLE_SOCKET) return;
                 old_time = curr_time;

		 break;
         }
      }

      while (tog_process(upacket) > 0) {}; /* check for tog packets again */
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
	     inet_ntoa(sock_inf[index].cli_addr.sin_addr),
	     ntohs(sock_inf[index].cli_addr.sin_port),
	     index, tcp_sock_count, sock_inf[index].rnd_id,
	     sock_inf[index].aim_srv_seq));

   tcp_sock_count--;

   /* we should delete it by hands - this fd may be cloned by fork */
   EV_SET(sock_chg+nchanges, sock_fds[index].fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
   nchanges++;

   close(sock_fds[index].fd);

   /* delete socket from index */
   if (index != tcp_sock_count)
   {
      sock_fds[index] = sock_fds[tcp_sock_count];
      sock_inf[index] = sock_inf[tcp_sock_count];

      /* now we should update kqueue object */
      EV_SET(sock_chg+nchanges, sock_fds[index].fd, EVFILT_READ, EV_ADD, 0, 0, (void *)index);

      nchanges++;
   }

   sock_fds[tcp_sock_count].fd = -1;
}


/**************************************************************************/
/* FreeBSD kernel queue initialization	 				  */
/**************************************************************************/
void kernel_queue_init()
{
   sock_kev  = new struct kevent[KQNEVENTS*2];
   sock_chg  = new struct kevent[KQNEVENTS*2];
   nchanges  = 0;

   /* Create a new kernel event queue and init descriptor */
   if ((kernq = kqueue()) == -1)
   {
      LOG_SYS(0, ("Error: Can't create kernel queue: %s\n", strerror(errno)));
      exit(EXIT_ERROR_FATAL);
   }

   /* Add initialized system sockets to kernel queue */
   for (int i=0; i<RES_SLOTS; i++)
   {
      if (sock_fds[i].fd != -1)
      {
         EV_SET(sock_chg+nchanges, sock_fds[i].fd, EVFILT_READ,
	        EV_ADD, 0, 0, (void *)i);

	 nchanges++;
      }
   }

   /* setup socket service timer: ident=1000, delay=1001ms */
   EV_SET(sock_chg+nchanges, 1000, EVFILT_TIMER, EV_ADD, 0, 1001, 0);
   nchanges++;
}


/**************************************************************************/
/* Added accepted socket to kernel queue (for common accept functions) 	  */
/**************************************************************************/
void accept_add_object(int index)
{
   if (sock_fds[index].fd != -1)
   {
      EV_SET(sock_chg+nchanges, sock_fds[index].fd,
             EVFILT_READ, EV_ADD, 0, 0, (void *)index);

      nchanges++;
   }
}

#endif

