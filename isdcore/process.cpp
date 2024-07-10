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
/* Server processing loop implemetation					  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#include "v3_proto/v3_defines.h"

extern unsigned long increm_num;

/**************************************************************************/
/* Determine who we are and do specific work 				  */
/**************************************************************************/
void process_server()
{
   while(1)
   {
      set_ps_display(process_role, "");

      switch (process_role)
      {
         case ROLE_SOCKET : process_socket();  break;  /* socket-processor  */
         case ROLE_PACKET : process_packet();  break;  /* packet-processor  */
         case ROLE_EPACKET: process_epacket(); break;  /* epacket-processor */
         case ROLE_ETIMER : process_etimer();  break;  /* etimer-processor  */
         case ROLE_EUSER  : process_euser();   break;  /* euser-processor   */
         case ROLE_BUSY   : process_busy();    break;  /* busy_processor    */
         case ROLE_DEFRAG : process_defrag();  break;  /* defragmenter      */
	 case ROLE_ACTIONS: process_actions(); break;  /* actions processor */
         case ROLE_NONE   : exit(EXIT_NORMAL); break;  /* we haven't role   */
         default: LOG_SYS(20, ("ERROR(!): UNKNOWN ROLE %%%d in server switch\n", process_role));
      }
   }
}

/**************************************************************************/
/* Packet processor main loop 						  */
/**************************************************************************/
void process_packet()
{
   Packet ppacket;

   usersdb_connect();
   parse_config_file(lp_v7_proto_config(), CONFIG_TYPE_AIM);
   init_all_protocols();
   init_random();

   for(;;)
   {
      pipe_receive_packet(ppacket);	/* receive packet from pipe  */
      handle_icq_packet(ppacket);	/* handle packet 	     */
   }

   return;
}


/**************************************************************************/
/* func, working with unnamed pipe. It receive packets 			  */
/* and send to socket "busy" message. It used to avoid pipe		  */
/* overflow on heavy server load (overload)				  */
/* Warning: live-time of this processor = 60 sec			  */
/**************************************************************************/
void process_busy()
{
   DEBUG(0,("BUSY: Not implemented...\n"));
   exit(EXIT_NORMAL);
}


/**************************************************************************/
/* Childs list  WARNING: don't use after list creation. This can	  */
/* cause a memory leak				 			  */
/**************************************************************************/
void init_childs_list()
{
   cl_begin = NULL;
   cl_list =  NULL;
   cl_last =  NULL;
}


/**************************************************************************/
/* Insert new child to list 						  */
/**************************************************************************/
void child_insert(pid_t child_pid, int role)
{
   cl_last = (child_record *)calloc(1, sizeof(*cl_last));

   DEBUG(100,("Insert child: %d (role: %d)\n",child_pid, role));

   cl_last->child.child_pid  = child_pid;
   cl_last->child.child_role = role;

   if (cl_begin == NULL) 		/* list is empty */
   {
     cl_last->prior = NULL;
     cl_begin = cl_last;
     cl_last->next = NULL;
   }
   else					/* insert before first */
   {
     cl_list = cl_begin;
     cl_begin = cl_last;
     cl_begin->next = cl_list;
     cl_begin->next->prior = cl_begin;
     cl_begin->prior = NULL;

   }
}


/**************************************************************************/
/* Delete child with specified pid from childs list			  */
/**************************************************************************/
void child_delete(pid_t child_pid)
{
   struct child_record * ltemp = NULL;
   cl_list = cl_begin;

   while(cl_list)
   {
      if (cl_list->child.child_pid == child_pid)
      {
         /* delete from middle of list */
         if ((cl_list->prior != NULL) & (cl_list->next != NULL))
         {
            cl_list->prior->next = cl_list->next;
 	    cl_list->next->prior = cl_list->prior;
	    ltemp = cl_list;
	    cl_list = cl_list->next;
	    free(ltemp);
	    return;
         }

         /* list contain only one record */
         if ((cl_list->prior == NULL) & (cl_list->next == NULL))
         {
            cl_begin = NULL;
 	    free(cl_list);
	    return;
         }

         /* delete record from end of the list */
         if (cl_list->next == NULL)
         {
            cl_list->prior->next = NULL;
	    free(cl_list);
	    return;
         }

         /* delete record from begin of the list */
         if (cl_list->prior == NULL)
         {
            cl_begin = cl_list->next;
 	    cl_list->next->prior = NULL;
	    free(cl_list);
	    return;
         }
      }
      cl_list = cl_list->next;
   }
}


/**************************************************************************/
/* Search child with specified pid and return it's role 		  */
/**************************************************************************/
int child_lookup(pid_t child_pid)
{
   cl_list = cl_begin;
   while(cl_list)
   {
     if (cl_list->child.child_pid == child_pid)
             return (cl_list->child.child_role);
     cl_list = cl_list->next;
   }
   return(-1);
}


/**************************************************************************/
/* Return number of childs with determined role. If role= -1 		  */
/* it return number of all childs 					  */
/**************************************************************************/
int childs_num(int role)
{
   int child_num = 0;
   cl_list = cl_begin;

   while(cl_list)
   {
      if (role == -1)	/* user want total number of childs */
      {
          child_num++;
      }
      else
      {
          if (cl_list->child.child_role == role) child_num++;
      }
      cl_list = cl_list->next;
   }
   return(child_num);
}


/**************************************************************************/
/* Return pid of event-processor 					  */
/**************************************************************************/
int geteppid()
{
    int child_pid = 0;
    cl_list = cl_begin;

    while(cl_list)
    {
       if (cl_list->child.child_role == ROLE_EPACKET)
       {
           child_pid = cl_list->child.child_pid;
 	   break;
       }
       cl_list = cl_list->next;
    }
    return(child_pid);
}


/**************************************************************************/
/* Check number of packet processors and fork new if need		  */
/**************************************************************************/
void check_packet_processors()
{
   int cnum = childs_num(ROLE_PACKET);
   int conf_num = lp_min_childs();

   if (cnum < conf_num)
   {
      fork_packet_processors(conf_num-cnum);
      childs_check = 0;
   }
}


/**************************************************************************/
/* Check if a event processors are alive and fork 			  */
/* new if they are dead							  */
/**************************************************************************/
void check_event_processors()
{
   if ((childs_num(ROLE_EPACKET) == 0) &&
       (process_role == ROLE_SOCKET))
   {
      fork_epacket_processor();
      childs_check = 0;
      reload_in_progress = False;
   }

   if ((childs_num(ROLE_EUSER) == 0) &&
       (process_role == ROLE_SOCKET))
   {
      fork_euser_processor();
      childs_check = 0;
      reload_in_progress = False;
   }

   if ((childs_num(ROLE_ETIMER) == 0) &&
       (process_role == ROLE_SOCKET))
   {
      fork_etimer_processor();
      childs_check = 0;
      reload_in_progress = False;
   }
}


/**************************************************************************/
/* Check if defrag processor is alive and fork 				  */
/* new if it is dead							  */
/**************************************************************************/
void check_defrag_processor()
{
   if (childs_num(ROLE_DEFRAG) == 0)
   {
      fork_defrag_processor();
      childs_check = 0;
      reload_in_progress = False;
   }
}


/**************************************************************************/
/* Check if actions processor is alive and fork 			  */
/* new if it is dead and actions enabled in config file			  */
/**************************************************************************/
void check_actions_processor()
{
   if ((childs_num(ROLE_ACTIONS) == 0) && (lp_actions_enabled() == True))
   {
      fork_actions_processor();
      childs_check = 0;
      reload_in_progress = False;
   }
}


/**************************************************************************/
/* Check if specified socket have new data for reading			  */
/**************************************************************************/
BOOL isready_data(int number)
{
   if (sock_fds[number].revents & POLLIN) return(True);
   return(False);
}


/**************************************************************************/
/* Check if specified socket have error or disconnected			  */
/**************************************************************************/
BOOL isready_error(int number)
{
   if (sock_fds[number].revents & POLLERR) return(True);
   if (sock_fds[number].revents & POLLHUP) return(True);
   return(False);
}


/**************************************************************************/
/* Check if specified socket have new data for reading 			  */
/**************************************************************************/
BOOL isready_data2(int number)
{
   if (evnt_fds[number].revents & POLLIN) return(True);
   return(False);
}


/**************************************************************************/
/* Server cleanup. Close files, kill childs, remove pidfile, etc 	  */
/**************************************************************************/
void Server_cleanup()
{
   union semun ss;
   struct shmid_ds sm;
   memset(&sm, 0, sizeof(sm));
   memset(&ss, 0, sizeof(ss));

   /* do it only if we are parent */
   if (process_role == ROLE_SOCKET)
   {
      /* close main udp socket */
      shutdown(msockfd, 2);
      close(msockfd);
      shutdown(aimsockfd, 2);
      close(aimsockfd);
      shutdown(msnsockfd, 2);
      close(msnsockfd);

      /* close interprocess pipes */
      close(incoming_pipe_fd[0]);
      close(incoming_pipe_fd[1]);

      close(outgoing_pipe_fd[0]);
      close(outgoing_pipe_fd[1]);

      close(epacket_pipe_fd[0]);
      close(epacket_pipe_fd[1]);

      close(euser_pipe_fd[0]);
      close(euser_pipe_fd[1]);

      close(defrag_pipe_fd[0]);
      close(defrag_pipe_fd[1]);

      close(toutgoing_pipe_fd[0]);
      close(toutgoing_pipe_fd[1]);

      if (lp_actions_enabled())
      {
         close(actions_pipe_fd[0]);
	 close(actions_pipe_fd[1]);
      }

      /* kill all childs - send SIGINT to process group */
      no_death_messages = True;
      signal(SIGINT, SIG_IGN);   /* we don't need this signal here */
      kill(0, SIGINT);

      /* remove IPC objects (semaphores, msg queues, shared mem, etc) */
      shmdt((char *)ipc_vars);
      shmdt((char *)usr_shm);
      semctl(semaphore_id, 0, IPC_RMID, ss);
      shmctl(sharedmem_id, IPC_RMID, &sm);
      shmctl(usr_shm_id, IPC_RMID, &sm);

      if (unlink(wp_serv_addr.sun_path) < 0)
      {
         LOG_SYS(0, ("CLEANUP: Can't remove [%s]\n", wp_serv_addr.sun_path));
         LOG_SYS(0, ("Error: [%s]\n", strerror(errno)));
      }

      /* remove lock file */
      if (pidFP != NULL) { Unlock(pidFP); fclose(pidFP); unlink(lp_pid_path()); }
      LOG_SYS(0, ("EXIT: Cleanup finished. Bye...\n"));
   }
}


/**************************************************************************/
/* Handle webpager messages from wwp-socket 			 	  */
/**************************************************************************/
void handle_wwp_mess(Packet &pack)
{
   unsigned  long signature;
   unsigned  long tuin;
   unsigned  long from_ip;
   unsigned short str_len;
   int i;

   char email[64];
   char name[32];
   char mess[450];
   char mess1[550];
   char mess2[550];
   pack.reset();

   /* extract data from packet */
   pack >> signature >> from_ip >> tuin >> str_len;

   if (signature != 0x01020709)
   {
      DEBUG(100, ("Wrong signature in wwp packet...\n"));
   }

   if (str_len > sizeof(email)+1)
   {
      DEBUG(10, ("WWP Email string len overflow...\n"));
      return;
   }
   for (i=0; i<str_len; i++) pack >> email[i];

   pack >> str_len;
   if (str_len > sizeof(name)+1)
   {
      DEBUG(10, ("WWP Name string len overflow...\n"));
      return;
   }

   for (i=0; i<str_len; i++) pack >> name[i];

   pack >> str_len;
   if (str_len > sizeof(mess)+1)
   {
      DEBUG(10, ("WWP Message string len overflow...\n"));
      return;
   }

   for (i=0; i<str_len; i++) pack >> mess[i];

   snprintf(mess1, sizeof(mess1)+1, "Sender IP: %s\x0d\x0a",
            inet_ntoa(icqToIp(from_ip)));

   snprintf(mess2, sizeof(mess2)-1, "%s%s", mess1, mess);

   /* now we should generate non-standart v3-packet
      and send it to packet-processor thru pipe */
   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxRCV_WWP_MSG
              << (unsigned short)0x0000
	      << (unsigned short)0x0000
	      << (unsigned  long)0x00000001
	      << (unsigned  long)0x00000000
	      << (unsigned  long)tuin
	      << (unsigned short)(strlen(name)+1)
	      << name
	      << (unsigned short)(strlen(email)+1)
	      << email
	      << (unsigned short)(strlen(mess2)+1)
	      << mess2;

   increm_num++;

   reply_pack.from_local = 1;
   spipe_send_packet(reply_pack);
}


