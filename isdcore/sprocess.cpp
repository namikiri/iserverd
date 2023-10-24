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
/* Socket processor const routines for all versions (poll/epoll/kqueue)	  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#include "v3_proto/v3_defines.h"

unsigned long old_counter;   	/* old counter for watchdog 		  */
unsigned long watchdog_cnt;  	/* queue hung seconds                     */

/**************************************************************************/
/* Receive and process packet from tcp outgoing socket			  */
/**************************************************************************/
int tog_process(Packet &tpacket)
{
   int rresult = 0;
   if ((rresult = toutgoing_receive_packet(tpacket)) > 0)
   {
      /* packet processors can ack to close particular connection */
      if (tpacket.sock_evt == SOCK_TERM) 
      { 
         DEBUG(50, ("tog_process, closing_connection (%lu, %lu)\n", 
                     tpacket.sock_hdl, tpacket.sock_rnd));
         close_socket(tpacket.sock_hdl, tpacket.sock_rnd);
         return(rresult);
      }
   
      if (tpacket.sock_type == SAIM) flap_send_packet(tpacket);
   }
   
   return(rresult);
}


/**************************************************************************/
/* Receive and process packet from udp socket 				  */
/**************************************************************************/
void udp_process(Packet &upacket)
{
   unsigned short pversion, pcomm, seq1, seq2;
   unsigned long uin_num, temp_stamp, lj;

   udp_receive_packet(upacket);
   upacket.reset(); 
   upacket >> pversion;

   /* well, ack packets we should handle separately    */
   /* because we can process them in non-blocking mode */
   switch (pversion)
   {
      case V3_PROTO: 

           upacket >> pcomm;

           if (pcomm == ICQ_CMDxRCV_ACK)
           {
              upacket >> seq1 >> seq2 >> uin_num;
              temp_stamp = generate_ack_number(uin_num, seq1, seq2);
              process_ack_event(uin_num, temp_stamp);
              return;
           }
              
	   break;
	      
      case V5_PROTO:
         
           V5Decrypt(upacket);
	   upacket >> lj >> uin_num >> lj >> pcomm;

           if (pcomm == ICQ_CMDxRCV_ACK)
	   {
	      upacket >> seq1 >> seq2;
              temp_stamp = generate_ack_number(uin_num, seq1, seq2);
              process_ack_event(uin_num, temp_stamp);
              return;
           }
              
           break;

      case SYS_PROTO:

           handle_sys_proto(upacket);
           return;

   }
   
   /* sanity check - to avoid spoofing */
   if (ipToIcq(upacket.from_ip) != 0)
   {
      increm_num++;
      spipe_send_packet(upacket);
      pack_processed++;
   }
}


/**************************************************************************/
/* Receive and process packet from wwp socket 				  */
/**************************************************************************/
void wwp_process(Packet &wpacket)
{
   wwp_receive_packet(wpacket);
   wpacket.from_local = 1;
   handle_wwp_mess(wpacket);
}


/**************************************************************************/
/* WATCHDOG: We should check packet queue each second. If packet queue	  */
/* doesn't decrease during several seconds that needed special handling   */
/**************************************************************************/
void watchdog_check()
{  
   /* if (lp_watchdog_enabled()) */
   if (False)
   {
      DEBUG(300, ("WATCHDOG called... tracking PPs activity\n"));

      /* note, that here we track only packet-processors    */
      /* activity but event-processors and other is free :( */
      if ((ipc_vars->pack_in_queue == (old_counter+increm_num)) &&
	  (ipc_vars->pack_in_queue != 0))
      {
         DEBUG(300, ("WATCHDOG increment (old: %lu, inc: %lu, ipc: %lu)\n",
       	            old_counter, increm_num, ipc_vars->pack_in_queue));

         watchdog_cnt++;
	 increm_num   =  0;
	 old_counter  =  ipc_vars->pack_in_queue;
      }
      else
      {
         DEBUG(300, ("WATCHDOG reset (old: %lu, inc: %lu, ipc: %lu)\n",
      	              old_counter, increm_num, ipc_vars->pack_in_queue));
 	     
         watchdog_cnt = 0;
         increm_num   = 0;
         old_counter  = ipc_vars->pack_in_queue;
      }

      /* check if timeout = lp_watchdog_timeout() */
      if ((int)watchdog_cnt > lp_watchdog_timeout())
      {
         /* well, it is pity but we have hung :( */
         LOG_ALARM(0, ("WATCHDOG: Packet-processors hung detected\n"));
         LOG_ALARM(0, ("WATCHDOG: restarting child processes...\n"));
	       
	 block_signal(SIGCHLD);
	 
         /* send signal to actions processor */
         send_event2ap(papack, ACT_PPHUNG, 0, 0, 0, 0, longToTime(time(NULL)), "");
         sleep(1);
			     
         exit_ok      = False;
         killpg(0, SIGINT); sleep(3);
         exit_ok      = True;
         increm_num   = 0;
         watchdog_cnt = 0;
	 old_counter  = ipc_vars->pack_in_queue;
	 old_time     = timeToLong(time(NULL));
	 
	 unblock_signal(SIGCHLD);
      }
   }
}


/***************************************************************************/
/* This function make watchdog variables initialization			   */
/***************************************************************************/
void watchdog_init()
{
   /* watchdog initialization */
   increm_num   = 0;
   watchdog_cnt = 0;
   old_counter  = ipc_vars->pack_in_queue;
   old_time     = timeToLong(time(NULL));
}


/***************************************************************************/
/* This function look into "childs_check" variable and start new childs    */
/* if they are dead							   */
/***************************************************************************/
void prchilds_check()
{
   /* check if we have lost any child */
   if (childs_check == 1)
   {
      /* check & fork new if need */
      check_packet_processors();
      if (process_role != ROLE_SOCKET) return;
      check_event_processors();
      if (process_role != ROLE_SOCKET) return;
      check_defrag_processor();
      if (process_role != ROLE_SOCKET) return;
      check_actions_processor();
      if (process_role != ROLE_SOCKET) return;
   }
}


/***************************************************************************/
/* Check sockets timeout						   */
/***************************************************************************/
void check_sockets_timeout()
{
   int v7_timeout = lp_v7_conn_timeout();
   for (int i=RES_SLOTS; i<tcp_sock_count; i++)
   {
      /* check if socket expired */
      if ((sock_inf[i].aim_started) &&
          ((int)(curr_time - sock_inf[i].lutime) > v7_timeout) &&
	  (sock_inf[i].sock_type == SAIM))
      {
         DEBUG(50, ("AIM: Socket index=%d closed on timeout...\n", i));
	 
	 /* we can use low-level socket shutdown routine because */
	 /* user still haven't hi-level connection               */
         close_socket_index_r(i, sock_inf[i].rnd_id);
	 continue;
      }   
   }
}



/***************************************************************************/
/* Get & process tcp socket data					   */
/***************************************************************************/
void tcp_process(int index)
{
   sock_inf[index].lutime = curr_time;
	 
   if (sock_inf[index].sock_type == SAIM)
   {
      flap_recv_packet(index, int_pack);

      /* keep-alive flap arrived. Keep alive packets used to */
      /* avoid disconnection on proxy/firewall timeout       */
      if (int_pack.flap_channel == 5) return;
   }
   else
   {
      tcp_receive_packet(index, int_pack);
   }

   if (int_pack.sizeVal == 0) 
   {
      /* there is a problem with socket.. (closing connection) */
      close_socket_index(index, int_pack.sock_rnd);
   }
   else
   {
      increm_num++;
      int_pack.from_local = 0;
      spipe_send_packet(int_pack);
      pack_processed++;
   }
}


/**************************************************************************/
/* Close socket, remove it from array and defragment it (array)		  */
/* this called only if pp acked us to close connection so we REALLY can   */
/* close connection and don't care about moving user offline              */
/**************************************************************************/
void close_socket(long sock_hdl, unsigned long sock_rnd)
{
   for (int i=RES_SLOTS; i<tcp_sock_count; i++)
   {
      if ((sock_hdl == sock_fds[i].fd) && 
          (sock_rnd == sock_inf[i].rnd_id))
      {
         DEBUG(100, ("Closing connection with index=%d\n", i));
         close_socket_index(i, sock_rnd);
	 return;
      }
   }
}


/**************************************************************************/
/* Notify PP about closed connection, and close it			  */
/**************************************************************************/
void close_socket_index(int index, unsigned long rnd_id)
{
	     
   if ((index + 1) >  tcp_sock_count) return;
   if (rnd_id != sock_inf[index].rnd_id) return;

   /* notify packet processors about terminated connection    */
   /* we shouldn't close connection unless one of the PP's    */
   /* ack about it because we can't drop owner of this socket */
   /* offline - we haven't database connection in SP          */
   int_pack.clearPacket();
   int_pack              << (unsigned long)0x00000000;
   
   int_pack.sock_hdl      = sock_fds[index].fd;
   int_pack.sock_evt 	  = SOCK_TERM;
   int_pack.sock_rnd      = rnd_id;
   int_pack.sock_type	  = sock_inf[index].sock_type;
   
   increm_num++;
   spipe_send_packet(int_pack);
   close_socket_index_r(index, rnd_id);
}


/**************************************************************************/
/* Receive and process packet from udp socket 				  */
/**************************************************************************/
void poll_table_init()
{
   /* Alloc memory for socket descriptors array */
   LOG_SYS(10, ("Init: Maximum tcp connections number = %d\n", lp_max_tcp_connections()));

   sock_fds  = new struct pollfd[lp_max_tcp_connections()+RES_SLOTS+5];
   sock_inf  = new struct sock_info[lp_max_tcp_connections()+RES_SLOTS+5];
   curr_time = (unsigned long)time(NULL);

   for (int i=0;i<(lp_max_tcp_connections()+RES_SLOTS);i++) 
   {
      sock_fds[i].fd          = -1;
      sock_fds[i].events      = 0;
      sock_fds[i].revents     = 0;
      sock_inf[i].aim_srv_seq = 0;
      sock_inf[i].aim_cli_seq = 0;
      sock_inf[i].rnd_id      = 0;
      sock_inf[i].sock_type   = 0;
      sock_inf[i].lutime      = 0;
   }
   
   /* num of accepted sockets =0, so array contain only RES_SLOTS reserved slots */
   tcp_sock_count = RES_SLOTS;

   /* Let's fill descriptors array for poll() function */
   sock_fds[SUDP].fd 	      = msockfd;
   sock_fds[SUDP].events      = POLLIN;
   sock_fds[SUDP].revents     = 0;
   sock_inf[SUDP].aim_srv_seq = 0;
   sock_inf[SUDP].aim_cli_seq = 0;
   sock_inf[SUDP].rnd_id      = 0;
   sock_inf[SUDP].sock_type   = SUDP;
   
   sock_fds[SWWP].fd 	      = wwpsockfd;
   sock_fds[SWWP].events      = POLLIN;
   sock_fds[SWWP].revents     = 0;
   sock_inf[SWWP].aim_srv_seq = 0;
   sock_inf[SWWP].aim_cli_seq = 0;
   sock_inf[SWWP].rnd_id      = 0;
   sock_inf[SWWP].sock_type   = SWWP;

   sock_fds[SAIM].fd 	      = aimsockfd;
   sock_fds[SAIM].events      = POLLIN;
   sock_fds[SAIM].revents     = 0;
   sock_inf[SAIM].aim_srv_seq = 0;
   sock_inf[SAIM].aim_cli_seq = 0;
   sock_inf[SAIM].rnd_id      = 0;
   sock_inf[SAIM].sock_type   = SAIM;

   sock_fds[SMSN].fd 	      = msnsockfd;
   sock_fds[SMSN].events      = POLLIN;
   sock_fds[SMSN].revents     = 0;
   sock_inf[SMSN].aim_srv_seq = 0;
   sock_inf[SMSN].aim_cli_seq = 0;
   sock_inf[SMSN].rnd_id      = 0;
   sock_inf[SMSN].sock_type   = SMSN;

   sock_fds[STOG].fd	      = toutgoing_pipe_fd[P_READ];
   sock_fds[STOG].events      = POLLIN;
   sock_fds[STOG].revents     = 0;
   sock_inf[STOG].aim_srv_seq = 0;
   sock_inf[STOG].aim_cli_seq = 0;
   sock_inf[STOG].rnd_id      = 0;
   sock_inf[STOG].sock_type   = STOG;

   setNonBlocking(sock_fds[STOG].fd);
}


/**************************************************************************/
/* Accept connection from aim/v7 based clients 				  */
/**************************************************************************/
void aim_accept_connect()
{   
   struct sockaddr_in caddr;
   socklen_t caddr_len = sizeof(caddr);
   
   /* check if we can't accept this connection */
   if ((tcp_sock_count+1) > (lp_max_tcp_connections()+RES_SLOTS))
   {
      close(accept(aimsockfd, (struct sockaddr *)&caddr, &caddr_len));
      LOG_SYS(0, ("WARN: Connection table full, connection from %s:%d dropped...\n", 
                   inet_ntoa(caddr.sin_addr), htons(caddr.sin_port)));
      return;
   }
   
   sock_fds[tcp_sock_count].fd = accept(aimsockfd, (struct sockaddr *)&caddr,
				        &caddr_len);

   if (sock_fds[tcp_sock_count].fd == -1)
   {
      DEBUG(0, ("Error: Can't accept socket: %s\n", strerror(errno)));
      return;
   }
					  
   /* now fill information about accepted socket into sock_fds & sock_inf */
   sock_fds[tcp_sock_count].events      = POLLIN | POLLERR;
   sock_fds[tcp_sock_count].revents     = 0;     
   sock_inf[tcp_sock_count].aim_srv_seq = (unsigned short)random();
   sock_inf[tcp_sock_count].aim_cli_seq = 0; /* undefined yet  */
   sock_inf[tcp_sock_count].aim_started = 2; /* just connected */
   sock_inf[tcp_sock_count].rnd_id      = (unsigned  long)random();
   sock_inf[tcp_sock_count].sock_type   = SAIM;
   sock_inf[tcp_sock_count].cli_addr    = caddr;
   sock_inf[tcp_sock_count].lutime      = curr_time;
   
   setNonBlocking(sock_fds[tcp_sock_count].fd);
   
   DEBUG(50, ("Accepted TCP AIM connection from %s:%d (rnd_id=%lu)\n",
              inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port), 
	      sock_inf[tcp_sock_count].rnd_id));

#ifndef DISABLE_ADD_OBJECT
   accept_add_object(tcp_sock_count);
#endif

   /* notify packet processors about new connection */
   int_pack.clearPacket();
   int_pack.sizeVal       = 40;
   int_pack.sock_hdl      = sock_fds[tcp_sock_count].fd;
   int_pack.sock_evt  	  = SOCK_NEW;
   int_pack.sock_rnd      = sock_inf[tcp_sock_count].rnd_id;
   int_pack.sock_type	  = sock_inf[tcp_sock_count].sock_type;
   int_pack.from_port	  = ntohs(sock_inf[tcp_sock_count].cli_addr.sin_port);
   int_pack.from_ip	  = sock_inf[tcp_sock_count].cli_addr.sin_addr;
   int_pack.flap_channel  = 1;
   
   increm_num++;
   spipe_send_packet(int_pack);
   tcp_sock_count++;
}


/**************************************************************************/
/* Accept and close connection from MSN clients (STUB) 			  */
/**************************************************************************/
void msn_accept_connect()
{
   struct sockaddr_in client_addr;
   socklen_t addr_len = sizeof(client_addr);
   
   close(accept(aimsockfd, (struct sockaddr *)&client_addr, &addr_len));

   LOG_SYS(0, ("Dropped TCP MSN connection from %s:%d\n",
                inet_ntoa(client_addr.sin_addr), 
	        ntohs(client_addr.sin_port)));
	    
   /* TODO: msn packets processing */
}


struct pollfd wfds[2];

/**************************************************************************/
/* Reliable pipe_send_packet version for socket processor 		  */
/**************************************************************************/
void spipe_send_packet(Packet &pack)
{
   int spres = -1;

   while (spres < 0)
   {
      spres = pipe_send_packet(pack);
      
      if (spres < 0)
      {
         if (errno == EMSGSIZE)
         {
            LOG_SYS(0, ("IPack was dropped because of size (%d)\n", pack.sizeVal));
	    LOG_SYS(0, ("Check the respective kernel parameter (FreeBSD: net.local.dgram.maxdgram)\n"));
	    ipc_vars->queue_send_errors++;
	    return;
         }
      
         if ((errno == EAGAIN)  || 
	     (errno == EINTR)   || 
	     (errno == ENOMEM)  || 
	     (errno == ENOBUFS) || 
	     (errno == EWOULDBLOCK))
         {
            /* Well, here we should wait while we can write data to this */
  	    /* pipe. But! There is tog pipe - we should control it too   */

            wfds[0].fd = incoming_pipe_fd[P_WRIT];
	    wfds[0].revents = 0;
	    wfds[0].events = POLLOUT;
	 
   	    wfds[1].fd = sock_fds[STOG].fd;
	    wfds[1].revents = 0;
	    wfds[1].events = POLLIN;
	 
	    while((wfds[0].revents & POLLOUT) == 0) 
	    {
	       poll(wfds, 2, -1);
	       while (tog_process(arpack) > 0) {};
	    }
	 }
	 else
	 {
	    LOG_SYS(0, ("SPIPE_SEND: Unexpected error: %s\n", strerror(errno)));
	    ipc_vars->queue_send_errors++;
	    return;
	 }
      }
   }
}


