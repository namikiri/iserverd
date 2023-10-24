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
/* This unit implements functions related tracking user timeouts.         */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* EUser process main loop                                                */
/**************************************************************************/
void process_euser()
{
   struct event_pack epacket;
  
   usersdb_connect();	/* connect to database server  */
   db_online_clear();	/* clear online_users table    */
   db_contacts_clear(); /* clear online_contacts table */
      
   evnt_fds[0].fd      = euser_pipe_fd[P_READ];
   evnt_fds[0].events  = POLLIN;
   evnt_fds[0].revents = 0;

   LOG_SYS(10, ("Init: EUser processor initialization success\n"));


   /* main loop for event-processor*/
   while(1)
   {
      /* block on sockets array for infinity time */
      if (poll(evnt_fds, 1, -1) < 1)
      {
         if (errno != EINTR) 
         {
            LOG_SYS(10, ("We have problem with poll(): %s\n", strerror(errno)));
	    exit(EXIT_ERROR_FATAL);
         }
      }
    
      /* check if we have event notification in pipe */
      if (isready_data2(0))
      {
          euser_receive_event(epacket);	/* receive event from pipe	*/
          euser_process_event(epacket);	/* parse event record	 	*/
      }
   }
}


/**************************************************************************/
/* EUser event packets handler                                            */
/**************************************************************************/
void euser_process_event(event_pack &inpack)
{
    unsigned long ip;
    
    DEBUG(200, ("EUSER EVENT: mtype: %lu, uin: %lu, stamp: %lu, ttl: %d\n", 
	     inpack.mtype, inpack.uin_number, inpack.ack_stamp, inpack.ttl));

    switch (inpack.mtype)
    {
	case MESS_ONLINE:
	     DEBUG(300, ("EUser got message: MESS_ONLINE\n"));
	     break;
	case MESS_OFFLINE:
	     DEBUG(300, ("EUser got message: MESS_OFFLINE\n"));

	     /* first we should find out client ip address */
	     ip = cache_ip_lookup(inpack.uin_number);
	     send_event2ap(papack, ACT_OFFLINE, inpack.uin_number, 0, ip, 0, 
		           longToTime(time(NULL)), "");
	     break;
	case MESS_UPDATE:
	     DEBUG(200, ("EUser got message: MESS_UPDATE (ttl=%d)\n",inpack.ttl));
	     online_cache_update(inpack.uin_number, inpack.ttl);
	     break;
	case MESS_TIMEOUT:
	     DEBUG(200, ("EUser got message: MESS_HEARTBEAT\n"));
	     online_cache_decrement();
	     scheduler_timer();
	     break;
	     
	default: break;
    }
}


/**************************************************************************/
/* Use this func in event processor after online database change to 	  */
/* update online cache							  */
/**************************************************************************/
void online_cache_update(unsigned long uin, unsigned short ttl)
{
   shm_user_update(uin, ttl);
}


/**************************************************************************/
/* Search client ip address in online cache			          */
/**************************************************************************/
unsigned long cache_ip_lookup(unsigned long uin)
{
   return(shm_get_ip(uin));
}


/**************************************************************************/
/* Decr counters in all cache records and drop offline expired users      */
/**************************************************************************/
void online_cache_decrement()
{
   struct online_user *temp_user;
   unsigned long temp_uin;
   unsigned long overhead, processed_num;
   unsigned long uptime;
   
   /* shm array may contain holes (!) */
   overhead = max_user_cnt;
   processed_num = 0;

   DEBUG(300, ("Decrement user's cache called...\n"));
  
   for (int i=0; i<(int)overhead; i++)
   {      
      temp_user = &(usr_shm[i]);

      if (processed_num >= ipc_vars->online_usr_num) break;

      /* check for hole */
      if (temp_user->uin != 0) 
      {
         processed_num++;
      }
      else
      {
         continue;
      }
      
      if (temp_user->ttl != 0)
      {
         temp_user->ttlv--;
	 
         DEBUG(450, ("Decrementing ttl: uin=%lu, ttl=%lu, idx=%d\n", 
	   	      temp_user->uin, temp_user->ttlv, i));
      }
       
      if ((temp_user->ttlv < 1) && 
          (temp_user->uin != 0) &&
	  (temp_user->ttl != 0))
      {
	   
         temp_uin = temp_user->uin;
	   
         /* calculate approx user uptime */
	 uptime = (unsigned long)(time(NULL) - longToTime(temp_user->uptime));

         /* we should expire user only if its uptime > 0 */
         if ((uptime < 0x7fffffff) && (uptime > 0))
	 {
            LOG_USR(0, ("User %lu expired & moved offline. Online time was: %lu seconds\n", temp_uin, uptime));
            DEBUG(50, ("SHM user record expired. User %lu deleted\n", temp_uin));
            move_user_offline(temp_uin);
         }
	 else
	 {
	    if (uptime > 0x7fffffff)
	    {
	       LOG_SYS(0, ("SHM record has negative uptime, please report to developer (uin=%lu)\n", temp_uin));
	    }
	 }
      }
   }
}


/**************************************************************************/
/* This function used to move user online. It insert new record in	  */
/* cache and database and init lastupdate value in online user record	  */
/**************************************************************************/
void move_user_online(struct online_user &user)
{  
    send_online2cache( user.uin, user.ttl, ipToIcq(user.ip) );
    db_users_touch( user );
    broadcast_online( user );
    db_defrag_delete( user.uin );
    
    DEBUG(100, ("User %lu moved online: [ttl=%ld,ltime=%s]\n", 
		user.uin, user.ttl, time2str(longToTime(user.lutime))));
    
    send_event2ap(papack, ACT_ONLINE, user.uin, user.status, ipToIcq(user.ip), 
                  user.protocol, longToTime(user.lutime), "");

}


/**************************************************************************/
/* This function used to move user offline. It delete cache record and	  */
/* wipe out it from database						  */
/**************************************************************************/
void move_user_offline(unsigned long uin)
{
    send_offline2cache(uin);
    db_contact_delete(uin);
    db_defrag_delete(uin);
    db_online_delete(uin, 1);
    broadcast_offline(uin);
}


/**************************************************************************/
/* This function send message to online cache. It user by packet 	  */
/* processors to inform cache about database update.			  */
/**************************************************************************/
void send_offline2cache(unsigned long uin)
{
   struct event_pack pmessage;
  
   pmessage.mtype      = MESS_OFFLINE;
   pmessage.uin_number = uin;
   pmessage.ack_stamp  = 0;
   pmessage.ttl	       = 0;
   pmessage.ip	       = 0;
  
   euser_send_event(pmessage);
}


/**************************************************************************/
/* This function send message to online cache. It user by packet 	  */
/* processors to inform cache about database update.			  */
/**************************************************************************/
void send_online2cache(unsigned long uin, unsigned short ttl, unsigned long ip)
{
   struct event_pack pmessage;
  
   pmessage.mtype      = MESS_ONLINE;
   pmessage.uin_number = uin;
   pmessage.ack_stamp  = 0;
   pmessage.ttl	       = ttl;
   pmessage.ip	       = ip;
   
   euser_send_event(pmessage);
}


/**************************************************************************/
/* This function send message to online cache. It user by packet 	  */
/* processors to inform cache about database update.			  */
/**************************************************************************/
void send_update2cache(unsigned long uin, unsigned short ttl)
{
   struct event_pack pmessage;
   
   pmessage.mtype      = MESS_UPDATE;
   pmessage.uin_number = uin;
   pmessage.ack_stamp  = 0;
   pmessage.ttl	       = ttl;
   pmessage.ip	       = 0;
  
   DEBUG(300, ("Trying to send update cmd to pipe: ttl=%d\n", pmessage.ttl));
  
   euser_send_event(pmessage);
}

