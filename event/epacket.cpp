/**************************************************************************/
/*									  */
/* Copyright (c) 2000-2005 by Alexandr V. Shutko, Khabarovsk, Russia	  */
/* All rights reserved.							  */
/*                                                                        */
/* Redistribution and use in source and binary forms, with or without	  */
/* modification, are permitted provided that the following conditions	  */
/* are met:								  */
/* 1. Redistributions of source code must retain the above copyright	  */
/*    notice, this list of conditions and the following disclaimer.	  */
/* 2. Redistributions in binary form must reproduce the above copyright	  */
/*    notice, this list of conditions and the following disclaimer in 	  */
/*    the documentation and/or other materials provided with the 	  */
/*    distribution.							  */
/*                                                                        */
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
/* This unit implements functions related packet confirming and tracking  */
/* non-confirmed packets.                                                 */
/*									  */
/**************************************************************************/

#include "includes.h"

/* for user pqueue */
struct usr_queue_s *uqueue_root = NULL;
struct usr_queue_s *uqueue_last = NULL;

/**************************************************************************/
/* main epacket processing loop 					  */
/**************************************************************************/
void process_epacket()
{
   struct indirect_pack *ppacket = NULL;
   struct event_pack epacket;

   ppacket = (struct indirect_pack *)calloc(1, sizeof(struct indirect_pack));
   if (!ppacket) return;
  
   LOG_SYS(10, ("Init: EPacket processor initialization success\n"));
  
   init_packets_list();
   usersdb_connect();
      
   /* we should create fd matrix for poll function */
   evnt_fds[SOUT].fd 	  = outgoing_pipe_fd[P_READ];
   evnt_fds[SOUT].events  = POLLIN;
   evnt_fds[SOUT].revents = 0;

   evnt_fds[SEVT].fd 	  = epacket_pipe_fd[P_READ];
   evnt_fds[SEVT].events  = POLLIN;
   evnt_fds[SEVT].revents = 0;

   /* now main loop for epacket-processor */
   while(1)
   {
      /* block on sockets array for infinity time */
      if (poll(evnt_fds, 2, -1) < 1)
      {
         if (errno != EINTR) 
         {
            LOG_SYS(10, ("We have problem with poll(): %s\n", strerror(errno)));
	    exit(EXIT_ERROR_FATAL);
         }
      }
    
      /* check if we have outgoing packet in pipe  */
      if (isready_data2(SOUT)) 
      {
  	  epacket_receive_packet(*ppacket);	/* receive packet from pipe 	*/
	  epacket_process_packet(*ppacket);	/* send it & add to list 	*/
      }

      /* check if we have event notification in pipe */
      if (isready_data2(SEVT))
      {
          epacket_receive_event(epacket);	/* receive event from pipe	*/
          epacket_process_event(epacket);	/* parse event record	 	*/
      }
   }
}


/**************************************************************************/
/* Processing event packet: send packet to network socket, add		  */
/* it to list, check its timeout & retry_nums each 1 sec and 		  */
/* resend it if need.							  */
/**************************************************************************/
void epacket_process_packet(indirect_pack &inpack)
{
   unsigned long proto_ver  = 0;
   struct online_user *user = NULL;

   /* using shm_index to speedup search */
   user = shm_iget_user(inpack.to_uin, inpack.shm_index);
   if (user == NULL) return;
   
   memcpy(arpack.buff, inpack.buff, inpack.sizeVal);
   
   arpack.sizeVal   = inpack.sizeVal;
   arpack.from_ip   = inpack.to_ip;
   arpack.from_port = inpack.to_port;
   
   /* OK :) So here we should check protocol version, find sseq1 in shared */
   /* memory, increment it, generate timestamp, calculate checkcode for v5 */
   proto_ver = inpack.ack_stamp;   
   inpack.ack_stamp = generate_ack_number(inpack.to_uin, user->servseq, 0);

   switch (proto_ver)
   {
      case V3_PROTO:  PutSeq3(arpack, user->servseq);
    	              break;

      case V5_PROTO:  PutSeq(arpack, user->servseq);
                      PutKey(arpack, calculate_checkcode(arpack));
                      break;

      case V7_PROTO:  /* V7 retrans handled by tcp/ip stack */
                      return;

      default: return;
   }

   user->servseq++;

   add_packet_to_list(inpack, user);
   udp_send_direct_packet(arpack);
}


/**************************************************************************/
/* Processing event: if we have ack event - we should remove confirmed	  */
/* packet from list, if it is db_change event - we should check if online */
/* cache data for this uin is valid.					  */
/**************************************************************************/
void epacket_process_event(event_pack &inpack)
{
    DEBUG(200, ("EPACKET EVENT: mtype: %lu, uin: %lu, stamp: %lu, ttl: %d\n", 
	     inpack.mtype, inpack.uin_number, inpack.ack_stamp, 
	     inpack.ttl));

    switch (inpack.mtype)
    {
	case MESS_ACK:
	     DEBUGADD(200, ("EPacket got message: MESS_ACK\n"));
	     remove_packet_from_list(inpack.uin_number, inpack.ack_stamp);
	     break;
	case MESS_TIMEOUT:
	     DEBUGADD(200, ("EPacket got message: MESS_HEARTBEAT\n"));
	     check_ack_list();
	     break;

	default: break;
    }
}


/**************************************************************************/
/* Packets list  initialization.					  */
/* WARNING: don't use after list creation. It can cause a memory leak	  */
/**************************************************************************/
void init_packets_list()
{
   uqueue_root = NULL;
   uqueue_last = NULL;
}


/**************************************************************************/
/* Send packet to user and add it to list				  */
/**************************************************************************/
void add_packet_to_list(indirect_pack &inpack, struct online_user *user)
{
   struct usr_queue_s *uqueue     = uqueue_root;
   struct usr_queue_s *uqueue_new = NULL;

   DEBUG(200,("Ins pack: to:%lu, size:%d, stamp:%lu, retry_num:%d timeout:%d\n",
              inpack.to_uin, inpack.sizeVal, inpack.ack_stamp, 
  	      inpack.retry_num, inpack.time_out));
  
   /* First we should find user in usr_queue */
   while (uqueue)
   {
      if (uqueue->uin == user->uin) break;
      uqueue = uqueue->next;
   }

   /* check if we should add this user to queue list */   
   if (!uqueue)
   {
      uqueue_new = create_user_queue(user, inpack);
      if (!uqueue_new) return;
         
      if (uqueue_last)
      {
         /* usr queue is not empty  */
	 uqueue_last->next = uqueue_new;
	 uqueue_new->prev  = uqueue_last;
	 uqueue_last       = uqueue_new;
	 uqueue            = uqueue_new;
      }
      else
      {
         /* no records in usr queue */
         uqueue_root       = uqueue_new;
         uqueue_last       = uqueue_new;
	 uqueue            = uqueue_new;
      }
   }
   else
   {
      /* Check if saved session was terminated and we have   */
      /* packet from new session. So we should delete this   */
      /* user record and its queue and create new one        */
      if (uqueue->usid != user->usid)
      {
         /* Well, we can save time because we already have   */
	 /* uqueue record, we should just change it          */
	 if (uqueue->pqueue)
	    delete_user_pqueue(uqueue);
	 
	 uqueue->uin      = user->uin;
	 uqueue->usid     = user->usid;
	 uqueue->ishm     = user->shm_index;
	 uqueue->ip       = user->ip;
	 uqueue->port     = user->udp_port;
	 uqueue->time_out = inpack.time_out;
      }
   }
   
   /* Now we have user queue root record with correct usid   */
   /* And we can add new packet to this user queue           */
   user_queue_add_packet(uqueue, inpack);
}


/**************************************************************************/
/* Remove packet from list						  */
/**************************************************************************/
void remove_packet_from_list(unsigned long ack_uin, unsigned long ack_stamp)
{
   struct usr_queue_s *uqueue = uqueue_root;

   while (uqueue)
   {
      if (uqueue->uin == ack_uin) break;
      uqueue = uqueue->next;
   }

   /* check found uqueue for packet */
   if (uqueue)
   {
      user_queue_del_packet(uqueue, ack_stamp);
   
      /* check if uqueue is empty and delete it */
      if (!uqueue->pqueue) delete_user_queue(uqueue);
   }
}


/**************************************************************************/
/* Check user's packet queues for timeouts/expiration			  */
/**************************************************************************/
void check_ack_list()
{
   struct usr_queue_s *uqueue = uqueue_root;
   struct usr_queue_s *cuqueue;

   while (uqueue)
   {
      cuqueue = uqueue;
      uqueue = uqueue->next;
      
      user_pqueue_check(cuqueue);
      if (!cuqueue->pqueue) delete_user_queue(cuqueue);
   }
}


/**************************************************************************/
/* This func create user packets queue					  */
/**************************************************************************/
struct usr_queue_s *create_user_queue(struct online_user *user, indirect_pack &inpack)
{
   struct usr_queue_s *uqueue_new = NULL;
   
   uqueue_new = (struct usr_queue_s *)calloc(1, sizeof(struct usr_queue_s));
   if (uqueue_new == NULL) return(NULL);

   uqueue_new->pqueue   = NULL;
   uqueue_new->next     = NULL;
   uqueue_new->prev	= NULL;
   uqueue_new->uin      = user->uin;
   uqueue_new->usid     = user->usid;
   uqueue_new->ishm     = user->shm_index;
   uqueue_new->time_out = inpack.time_out;
   uqueue_new->ip       = user->ip;
   uqueue_new->port     = user->udp_port;
   
   return(uqueue_new);
}


/**************************************************************************/
/* This func delete user queue record					  */
/**************************************************************************/
void delete_user_queue(struct usr_queue_s *uqueue)
{
   /* 1. remove uqueue - relink pointers */
   if (uqueue->next)
      uqueue->next->prev = uqueue->prev;
   else
      uqueue_last = uqueue->prev;
      
   if (uqueue->prev)
      uqueue->prev->next = uqueue->next;
   else
      uqueue_root = uqueue->next;

   /* 2. del uqueue->pqueue, del uqueue  */
   delete_user_pqueue(uqueue);
   free(uqueue);
}


/**************************************************************************/
/* This func delete user packets queue					  */
/**************************************************************************/
void delete_user_pqueue(struct usr_queue_s *uqueue)
{
   struct ipack_s *pqueue = uqueue->pqueue;
   struct ipack_s *cpqueue = NULL;
   
   while (pqueue)
   {
      cpqueue = pqueue;      
      pqueue  = pqueue->next;
      
      free(cpqueue->buff);
      free(cpqueue);
   }
   
   uqueue->pqueue = NULL;
}


/**************************************************************************/
/* Add incoming packet to user personal queue 				  */
/**************************************************************************/
void user_queue_add_packet(struct usr_queue_s *uqueue, indirect_pack &inpack)
{
   struct ipack_s *pqueue_record = NULL;
   
   pqueue_record = (struct ipack_s *)calloc(1, sizeof(struct ipack_s));
   if (!pqueue_record) return;
   
   pqueue_record->buff = (char *)calloc(1, inpack.sizeVal);
   
   if (!pqueue_record->buff) 
   {
      free(pqueue_record);
      return;
   }
   
   pqueue_record->sizeVal   = inpack.sizeVal;
   pqueue_record->retry_num = inpack.retry_num;
   pqueue_record->ack_stamp = inpack.ack_stamp;
   pqueue_record->ctime_out = uqueue->time_out;
   memcpy(pqueue_record->buff, inpack.buff, pqueue_record->sizeVal);
   
   /* now link new packet to pqueue */
   if (uqueue->pqueue)
      uqueue->pqueue->prev = pqueue_record;
   
   pqueue_record->next  = uqueue->pqueue;
   pqueue_record->prev  = NULL;
   uqueue->pqueue       = pqueue_record;
}


/**************************************************************************/
/* Delete confirmed packet from user personal queue 			  */
/**************************************************************************/
void user_queue_del_packet(struct usr_queue_s *uqueue, unsigned long ack_stamp)
{
   struct ipack_s *pqueue = uqueue->pqueue;
   struct ipack_s *cpqueue = NULL;
   
   while(pqueue)
   { 
      cpqueue = pqueue;
      pqueue = pqueue->next;
      
      if (cpqueue->ack_stamp == ack_stamp)
      {
         DEBUG(200, ("Del pack: uin=%lu, stamp=%lu\n", uqueue->uin, cpqueue->ack_stamp));
      
         /* remove packet from queue */
	 if (cpqueue->next)
	    cpqueue->next->prev = cpqueue->prev;

         if (cpqueue->prev)
	    cpqueue->prev->next = cpqueue->next;
	 else
	    uqueue->pqueue = cpqueue->next;
	 
	 /* free allocated memory */
	 free(cpqueue->buff);
	 free(cpqueue);
      }
   }
}


/**************************************************************************/
/* Check user queue for expired packets 	 			  */
/**************************************************************************/
void user_pqueue_check(struct usr_queue_s *uqueue)
{
   struct ipack_s *pqueue = uqueue->pqueue;
   struct ipack_s *qpack  = NULL;
   
   while (pqueue)
   {
      qpack = pqueue;
      pqueue = pqueue->next;
      
      /* Check packet and retransmit on timeout */
      if (((qpack->ctime_out) == 0) & 
          ((qpack->retry_num) != 0)) 
      {
         DEBUG(200, ("Timeout. Resending pack [stamp=%lu]\n",  qpack->ack_stamp));
					   
         memcpy(arpack.buff, qpack->buff, qpack->sizeVal);
	 
         arpack.sizeVal    = qpack->sizeVal;
         arpack.from_ip    = uqueue->ip;
         arpack.from_port  = uqueue->port;
       
         udp_send_direct_packet(arpack);
       
         qpack->retry_num--;
         qpack->ctime_out = uqueue->time_out;
      }
      
      /* Check if packet expired and delete it */
      if (((qpack->ctime_out) == 0) & 
          ((qpack->retry_num) == 0)) 
      {
         packet_expire(uqueue, qpack);
	 
         /* remove packet from queue */
	 if (qpack->next)
	    qpack->next->prev = qpack->prev;

         if (qpack->prev)
	    qpack->prev->next = qpack->next;
	 else
	    uqueue->pqueue = qpack->next;
	 
	 /* free allocated memory */
	 free(qpack->buff);
	 free(qpack);
	 
	 continue;
      }

      if (qpack->ctime_out != 0) qpack->ctime_out--;
   }
}


/**************************************************************************/
/* Called when paket is expired in list	(maybe we'll want to force user	  */
/* go offline because of that)						  */
/**************************************************************************/
void packet_expire(struct usr_queue_s *uqueue, struct ipack_s *pack)
{
   DEBUG(100, ("Packet expired in confirm list [uin:%lu], [stamp:%lu]\n",
               uqueue->uin, pack->ack_stamp));
}

          
/**************************************************************************/
/* This func generate unique unsigned long number to identify packet	  */
/* and its ack packet. We get pack sequence numbers (two, but second 	  */
/* can be zero) recipient uin number, we should reproduce same number 	  */
/* from ack packet (any version) 					  */
/**************************************************************************/
unsigned long generate_ack_number(unsigned long puin, 
	      unsigned long pseq1, unsigned long pseq2)
{
   /* I simply sum them, but may be there are better algs exists  */
   /* This func can produce same numbers for different users	  */
   /* this is ok - we also check uin number in confirm procedure  */
   return (puin + pseq2 + pseq1);

}


/**************************************************************************/
/* This func send delivery confirm message (ack) to event processor.	  */
/* Called by packet processors						  */
/**************************************************************************/
void process_ack_event(unsigned long puin, unsigned long pstamp)
{
   struct event_pack pmessage;
  
   pmessage.mtype      = MESS_ACK;
   pmessage.uin_number = puin;
   pmessage.ack_stamp  = pstamp;
   pmessage.ttl	       = 0;
   pmessage.ip         = 0;
  
   epacket_send_event(pmessage);
}


