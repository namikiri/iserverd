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
/* Description: Interprocess (IPC) pipes via unix domain sockets 	  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

int incoming_pipe_fd[2];	 /* incoming pipe handler	*/
int outgoing_pipe_fd[2];	 /* outgoing pipe handler	*/
int epacket_pipe_fd[2];		 /* epacket pipe handler        */
int euser_pipe_fd[2];            /* euser pipe handler          */
int defrag_pipe_fd[2];		 /* defragmenter pipe handler	*/
int actions_pipe_fd[2];		 /* actions processor pipe hdl	*/
int toutgoing_pipe_fd[2];	 /* tcp outgoing pipe handler   */

/**************************************************************************/
/* Write size of packet to socket domain pipe, then write 		  */
/* packet,ip,port and other parameters					  */
/**************************************************************************/
int pipe_send_packet(Packet &pack)
{
   int snd_result;

   /* first write packet parameters to temp packet */
   spack.clearPacket();   
   spack << (unsigned  long)(ipToIcq(pack.from_ip))
         << (unsigned  long)(pack.from_port)
	 << (long)pack.sock_hdl
	 << (unsigned  long)pack.sock_rnd
	 << (unsigned short)pack.sock_type
	 << (unsigned short)pack.sock_evt
	 << (unsigned short)pack.from_local
	 << (unsigned short)pack.flap_channel
	 << (unsigned  long)pack.seq_number;
	     
   /* preparing temp packet for pipe socket */
   memcpy(spack.nextData, pack.buff, pack.sizeVal);
   spack.sizeVal += pack.sizeVal;

   lock_incw();
   lock_ipcw();

   /* send packet to pipe socket in non-blocking mode */
   snd_result = sendto(incoming_pipe_fd[P_WRIT], spack.buff, spack.sizeVal, 0, 0, 0);

   if (snd_result > 0) 
   {      
      ipc_vars->pack_in_queue++;
      ipc_vars->byte_in_queue += snd_result;

      /* ajust max queue sizes */
      if (ipc_vars->max_queue_size < ipc_vars->byte_in_queue)
            ipc_vars->max_queue_size = ipc_vars->byte_in_queue;

      if (ipc_vars->max_queue_pack < ipc_vars->pack_in_queue)
            ipc_vars->max_queue_pack = ipc_vars->pack_in_queue;
   }

   unlock_ipcw();
   unlock_incw();
   return(snd_result);
}


/**************************************************************************/
/* Write size of packet to socket domain pipe, then write 		  */
/* packet,ip,port and other parameters					  */
/**************************************************************************/
void tcp_writeback_packet(Packet &pack)
{
   DEBUG(400, ("Writeback packet: size=%d, from=%s:%d, sock_hdl=%lu, sock_rnd=%lu, sock_type=%d, flap_chan=%d\n", 
              pack.sizeVal, inet_ntoa(pack.from_ip), pack.from_port, pack.sock_hdl, pack.sock_rnd, 
	      pack.sock_type, pack.flap_channel));

   pipe_pack.clearPacket();
   
   /* first write packet parameters to temp packet */
   pipe_pack << (unsigned  long)(ipToIcq(pack.from_ip))
             << (unsigned  long)(pack.from_port)
	     << (long)pack.sock_hdl
	     << (unsigned  long)pack.sock_rnd
	     << (unsigned short)pack.sock_type
	     << (unsigned short)pack.sock_evt
	     << (unsigned short)pack.flap_channel;

   /* preparing temp packet for pipe socket */
   memcpy(pipe_pack.nextData, pack.buff, pack.sizeVal);
   pipe_pack.sizeVal += pack.sizeVal;

   /* send packet to pipe socket */
   sendto(toutgoing_pipe_fd[P_WRIT], pipe_pack.buff, pipe_pack.sizeVal, 0, 0, 0);

}


/**************************************************************************/
/* Pipe packet read procedure. Lock pipe and read packet		  */ 
/* into packet class							  */
/**************************************************************************/
void pipe_receive_packet(Packet &pack)
{
   unsigned long temp_ip;
   pack.clearPacket();
   pipe_pack.clearPacket();

   lock_pipe(INCOMING_PIPE);

   pipe_pack.sizeVal = recvfrom(incoming_pipe_fd[P_READ], pipe_pack.buff, 
        			MAX_PACKET_SIZE, 0, 0, 0);

   unlock_pipe(INCOMING_PIPE);
   lock_ipcw();
   
   if (pipe_pack.sizeVal > 0) 
   {
      if ((ipc_vars->pack_in_queue != 0) ||
          (ipc_vars->byte_in_queue != 0))
      {
         ipc_vars->pack_in_queue--;
         ipc_vars->byte_in_queue-=pipe_pack.sizeVal;
      }
      else
      {
         if (ipc_vars->pack_in_queue == 0) ipc_vars->byte_in_queue = 0;
      }
   }
   else
   {
      DEBUG(100, ("precv: error returned (\"%s\")\n", strerror(errno)));
      pack.sizeVal = 0; unlock_ipcw();
      return;
   }

   unlock_ipcw();

   pipe_pack.reset();   
   pipe_pack >> temp_ip
             >> pack.from_port
	     >> pack.sock_hdl
	     >> pack.sock_rnd
	     >> pack.sock_type
	     >> pack.sock_evt
	     >> pack.from_local
	     >> pack.flap_channel
	     >> pack.seq_number;
	     
   pack.from_ip = icqToIp(temp_ip);
   memcpy(pack.buff, pipe_pack.nextData, pipe_pack.sizeVal-28);
   pack.sizeVal = pipe_pack.sizeVal-28;

   return;
}


/**************************************************************************/
/* tcp outgoing pipe read procedure. Read packet from tog pipe 		  */ 
/* into packet class with all its paramaters				  */
/**************************************************************************/
int toutgoing_receive_packet(Packet &pack)
{
   unsigned long temp_ip;
   pack.clearPacket();

   pipe_pack.sizeVal = recvfrom(toutgoing_pipe_fd[P_READ], pipe_pack.buff, 
				MAX_PACKET_SIZE, 0, 0, 0);

   if (pipe_pack.sizeVal > 0)
   {
      pipe_pack.reset();   
      pipe_pack >> temp_ip
                >> pack.from_port
	        >> pack.sock_hdl
	        >> pack.sock_rnd
	        >> pack.sock_type
	        >> pack.sock_evt
	        >> pack.flap_channel;
   
      pack.from_ip = icqToIp(temp_ip);
      pack.sizeVal = pipe_pack.sizeVal - 22;
      memcpy(pack.buff, pipe_pack.nextData, pack.sizeVal);
   }
   else
   {
      pack.sizeVal = 0;
   }
   
   return(pipe_pack.sizeVal);
}


/**************************************************************************/
/* recv packet from actions pipe 					  */
/**************************************************************************/
int actions_receive_packet(Packet &pack)
{
   pack.clearPacket();

   pack.sizeVal = recvfrom(actions_pipe_fd[P_READ], pack.buff, 
        		   MAX_PACKET_SIZE, 0, 0, 0);
   
   return(pack.sizeVal);
}


/**************************************************************************/
/* this pipe should be readed only by one epacket processor, so   	  */
/* doesn't need lock&unlock it for reading. We should read 		  */
/* packet, ip, port, timeout, retry_num	and push them to inpack 	  */
/**************************************************************************/
void epacket_receive_packet(indirect_pack &inpack)
{
   unsigned long temp_ip;
   
   pipe_pack.sizeVal = recvfrom(outgoing_pipe_fd[P_READ], pipe_pack.buff, 
        			MAX_PACKET_SIZE, 0, 0, 0);

   pipe_pack.reset();
   
   pipe_pack >> temp_ip
             >> inpack.to_port
	     >> inpack.retry_num
             >> inpack.time_out
	     >> inpack.ack_stamp
	     >> inpack.shm_index
	     >> inpack.to_uin;

   inpack.to_ip = icqToIp(temp_ip);
   memcpy(inpack.buff, pipe_pack.nextData, pipe_pack.sizeVal-28);
   inpack.sizeVal = pipe_pack.sizeVal-28;

   return;
}


/**************************************************************************/
/* this used to receive epacket notifications from event pipe		  */
/**************************************************************************/
void epacket_receive_event(event_pack &inpack)
{
   pipe_pack.sizeVal = recvfrom(epacket_pipe_fd[P_READ], pipe_pack.buff, 
        			MAX_PACKET_SIZE, 0, 0, 0);

   DEBUG(400, ("Received epacket event record, length: %d\n", pipe_pack.sizeVal));

   pipe_pack.reset();
   
   pipe_pack >> inpack.mtype
             >> inpack.uin_number
	     >> inpack.ack_stamp
	     >> inpack.ttl;
  
   return;	        
}


/**************************************************************************/
/* this used to receive euser notifications from event pipe		  */
/**************************************************************************/
void euser_receive_event(event_pack &inpack)
{
   pipe_pack.sizeVal = recvfrom(euser_pipe_fd[P_READ], pipe_pack.buff, 
        			MAX_PACKET_SIZE, 0, 0, 0);

   DEBUG(400, ("Received euser event record, length: %d\n", pipe_pack.sizeVal));

   pipe_pack.reset();
     
   pipe_pack >> inpack.mtype
    	     >> inpack.uin_number
    	     >> inpack.ack_stamp
             >> inpack.ttl
    	     >> inpack.ip;

   return;
}


/**************************************************************************/
/* this used to receive defrag notifications from event pipe		  */
/**************************************************************************/
void defrag_receive_pipe(defrag_pack &inpack)
{
   pipe_pack.sizeVal = recvfrom(defrag_pipe_fd[P_READ], pipe_pack.buff, 
        			MAX_PACKET_SIZE, 0, 0, 0);

   pipe_pack.reset();
    
   pipe_pack >> inpack.mtype
    	     >> inpack.uin_num
    	     >> inpack.seq;

   DEBUG(100, ("Received defrag record, len:%d, mtype:%lu, uin:%lu, seq:%d\n", 
                pipe_pack.sizeVal, inpack.mtype, inpack.uin_num, inpack.seq));

   return;
}


/**************************************************************************/
/* Write packet to outgoing pipe for event processor. We need		  */
/* addr:port, max number of retries (from config), max confirm		  */
/* timeout and long unique number of packet to recognize ack on 	  */
/* any protocol (ack_timestamp)						  */
/**************************************************************************/
void event_send_packet(Packet &pack, unsigned long to_uin, 
     unsigned long shm_index, unsigned long retry_num, 
     unsigned long rtimeout, unsigned long ack_timestamp)
{

   lock_pipe(OUTGOING_PIPE);

   pipe_pack.clearPacket();
   
   pipe_pack << (unsigned long)(ipToIcq(pack.from_ip))
             << (unsigned long)(pack.from_port)
	     << (unsigned long)retry_num
	     << (unsigned long)rtimeout
	     << (unsigned long)ack_timestamp
	     << (unsigned long)shm_index
	     << (unsigned long)to_uin;

   /* preparing temp packet for pipe socket */
   memcpy(pipe_pack.nextData, pack.buff, pack.sizeVal);
   pipe_pack.sizeVal += pack.sizeVal;
   
   /* send packet to pipe socket */
   sendto(outgoing_pipe_fd[P_WRIT], pipe_pack.buff, pipe_pack.sizeVal, 0, 0, 0);

   unlock_pipe(OUTGOING_PIPE);
}


/**************************************************************************/
/* Write epacket event record to event pipe.			          */
/**************************************************************************/
void epacket_send_event(event_pack &pack) 
{
   /* preparing temp packet for pipe socket */
   pipe_pack.clearPacket();
   
   pipe_pack << (unsigned  long)pack.mtype
             << (unsigned  long)pack.uin_number
	     << (unsigned  long)pack.ack_stamp
	     << (unsigned short)pack.ttl;
   
   /* send packet to pipe socket */
   sendto(epacket_pipe_fd[P_WRIT], pipe_pack.buff, pipe_pack.sizeVal, 0, 0, 0);
}


/**************************************************************************/
/* Write euser event record to event pipe.			          */
/**************************************************************************/
void euser_send_event(event_pack &pack) 
{
   /* preparing temp packet for pipe socket */
   pipe_pack.clearPacket();
   
   pipe_pack << (unsigned  long)pack.mtype
             << (unsigned  long)pack.uin_number
	     << (unsigned  long)pack.ack_stamp
	     << (unsigned short)pack.ttl
	     << (unsigned  long)pack.ip;
   
   /* send packet to pipe socket */
   sendto(euser_pipe_fd[P_WRIT], pipe_pack.buff, pipe_pack.sizeVal, 0, 0, 0);
}


/**************************************************************************/
/* Write to defragment pipe.						  */
/**************************************************************************/
void defrag_send_pipe(defrag_pack &pack) 
{
   /* preparing temp packet for pipe socket */
   pipe_pack.clearPacket();
   
   pipe_pack << (unsigned  long)pack.mtype
             << (unsigned  long)pack.uin_num
	     << (unsigned short)pack.seq;
   
   /* send packet to pipe socket */
   sendto(defrag_pipe_fd[P_WRIT], pipe_pack.buff, pipe_pack.sizeVal, 0, 0, 0);
}


/**************************************************************************/
/* Send packet to actions pipe */
/**************************************************************************/
void actions_send_packet(Packet &pack)
{
   /* preparing temp packet for pipe socket */
   memcpy(pipe_pack.buff, pack.buff, pack.sizeVal);
   pipe_pack.sizeVal = pack.sizeVal;
   pipe_pack.append();
   
   /* send packet to pipe socket */
   sendto(actions_pipe_fd[P_WRIT], pipe_pack.buff, pipe_pack.sizeVal, 0, 0, 0);
}


/**************************************************************************/
/* WWP pipe initialization. It return socket descriptor 	 	  */
/**************************************************************************/
void wwp_socket_init()
{
  if ((wwpsockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
  {
     LOG_SYS(0,("ERROR: Can't create WWP unix domain socket...\n"));
     exit(EXIT_ERROR_FATAL);
  }
  
  bzero((char *)&wp_serv_addr, sizeof(wp_serv_addr));
  wp_serv_addr.sun_family = AF_UNIX;
  snprintf(wp_serv_addr.sun_path, sizeof(wp_serv_addr.sun_path)-1,
           lp_wwp_socketname());
	   
  wp_saddrlen = sizeof(wp_serv_addr.sun_family) + 
                       strlen(wp_serv_addr.sun_path)+1;

  if (unlink(wp_serv_addr.sun_path) == 0)
  {
     LOG_SYS(0, ("Init: WWP Unix socket exist: unclean shutdown?...\n"));
  }

  if (bind(wwpsockfd, (struct sockaddr *)&wp_serv_addr, wp_saddrlen) < 0)
  {
     LOG_SYS(0, ("ERROR: Can't bind WWP socket to special file...\n"));
     exit(EXIT_ERROR_FATAL);
  }
  
  chmod(wp_serv_addr.sun_path, 0666);
}


/**************************************************************************/
/* Incoming pipe initialization. It return socket descriptor 	 	  */
/**************************************************************************/
void incoming_pipe_init()
{
  if (socketpair(AF_UNIX, SOCK_DGRAM, 0, incoming_pipe_fd) != 0)
  {
     LOG_SYS(0,("ERROR: Can't create unix domain socket for IPC pipe...\n"));
     exit(EXIT_ERROR_FATAL);
  }
}


/**************************************************************************/
/* Outgoing pipe initialisation. It return pipe descriptors pair 	  */
/**************************************************************************/
void outgoing_pipe_init()
{
  if (socketpair(AF_UNIX, SOCK_DGRAM, 0, outgoing_pipe_fd) != 0)
  {
     LOG_SYS(0,("ERROR: Can't create unix domain socket for IPC pipe...\n"));
     exit(EXIT_ERROR_FATAL);
  }
}


/**************************************************************************/
/* Outgoing pipe initialisation. It return pipe descriptors pair 	  */
/**************************************************************************/
void actions_pipe_init()
{
  if (!lp_actions_enabled()) return;

  if (socketpair(AF_UNIX, SOCK_DGRAM, 0, actions_pipe_fd) != 0)
  {
     LOG_SYS(0,("ERROR: Can't create unix domain socket for IPC pipe...\n"));
     exit(EXIT_ERROR_FATAL);
  }
}


/**************************************************************************/
/* TCP outgoing pipe initialisation. It return pipe descriptor	 	  */
/**************************************************************************/
void toutgoing_pipe_init()
{
  if (socketpair(AF_UNIX, SOCK_DGRAM, 0, toutgoing_pipe_fd) != 0)
  {
     LOG_SYS(0,("ERROR: Can't create unix domain socket for IPC pipe...\n"));
     exit(EXIT_ERROR_FATAL);
  }
}


/**************************************************************************/
/* EPacket pipe initialization. It return socket descriptor 	 	  */
/**************************************************************************/
void epacket_pipe_init()
{
  if (socketpair(AF_UNIX, SOCK_DGRAM, 0, epacket_pipe_fd) != 0)
  {
     LOG_SYS(0,("ERROR: Can't create unix domain socket for IPC pipe...\n"));
     exit(EXIT_ERROR_FATAL);
  }
}


/**************************************************************************/
/* EUser pipe initialization. It return socket descriptor 	 	  */
/**************************************************************************/
void euser_pipe_init()
{
  if (socketpair(AF_UNIX, SOCK_DGRAM, 0, euser_pipe_fd) != 0)
  {
     LOG_SYS(0,("ERROR: Can't create unix domain socket for IPC pipe...\n"));
     exit(EXIT_ERROR_FATAL);
  }
}


/**************************************************************************/
/* Event pipe initialization. It return socket descriptor 	 	  */
/**************************************************************************/
void defrag_pipe_init()
{
  if (socketpair(AF_UNIX, SOCK_DGRAM, 0, defrag_pipe_fd) != 0)
  {
     LOG_SYS(0,("ERROR: Can't create unix domain socket for IPC pipe...\n"));
     exit(EXIT_ERROR_FATAL);
  }
}


