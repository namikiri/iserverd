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
/* Initialization stuff							  */
/*                                                                        */
/**************************************************************************/


#include "includes.h"

/* semaphore control data structures */

static struct sembuf inc_lock[2]    = { {0, 0, 0}, {0, 1, 0} };
static struct sembuf inc_unlock[1]  = { {0,-1, 0} };

static struct sembuf out_lock[2]    = { {1, 0, 0}, {1, 1, 0} };
static struct sembuf out_unlock[1]  = { {1,-1, 0} };

static struct sembuf incw_lock[2]   = { {2, 0, 0}, {2, 1, 0} };
static struct sembuf incw_unlock[1] = { {2,-1, 0} };

static struct sembuf shmr_lock[2]   = { {3, 0, 0}, {3, 1, 0} };
static struct sembuf shmr_unlock[1] = { {3,-1, 0} };

static struct sembuf ipcw_lock[2]   = { {4, 0, 0}, {4, 1, 0} };
static struct sembuf ipcw_unlock[1] = { {4,-1, 0} };


/*************************************************************************/
/* Create semaphore and increase it to 1. 1 - pipe free, 0 - pipe locked */
/*************************************************************************/
void ipc_objects_init()
{
  semun ss;
  memset(&ss, 0, sizeof(ss));
  
  semaphore_id = semget(IPC_PRIVATE, 5, 0600|IPC_CREAT|IPC_EXCL);

  if (semaphore_id == -1)
  {
    if (errno != ENOSPC)
    {
       LOG_SYS(0, ("Can't create semaphores. Error: [%s]\n", strerror(errno)));
    }
    else
    {
       LOG_SYS(0, ("Can't create semaphores. Maximum kernel number limit exceeded.\n"));
       LOG_SYS(0, ("You need to raise the respective kernel parameter.\n"));
    }
    
    exit(EXIT_ERROR_IPC);
  }

  semctl(semaphore_id, 0, SETVAL, ss);
  semctl(semaphore_id, 1, SETVAL, ss);
  semctl(semaphore_id, 2, SETVAL, ss);
  semctl(semaphore_id, 3, SETVAL, ss);
  semctl(semaphore_id, 4, SETVAL, ss);

  inc_lock[0].sem_flg    = SEM_UNDO;  inc_lock[1].sem_flg   = SEM_UNDO;
  inc_unlock[0].sem_flg  = SEM_UNDO;
  
  out_lock[0].sem_flg    = SEM_UNDO;  out_lock[1].sem_flg   = SEM_UNDO;
  out_unlock[0].sem_flg  = SEM_UNDO;

  incw_lock[0].sem_flg   = SEM_UNDO;  incw_lock[1].sem_flg   = SEM_UNDO;
  incw_unlock[0].sem_flg = SEM_UNDO;

  shmr_lock[0].sem_flg   = SEM_UNDO;  shmr_lock[1].sem_flg   = SEM_UNDO;
  shmr_unlock[0].sem_flg = SEM_UNDO;

  ipcw_lock[0].sem_flg   = SEM_UNDO;  ipcw_lock[1].sem_flg   = SEM_UNDO;
  ipcw_unlock[0].sem_flg = SEM_UNDO;

  sems_ready = 1;
  
  /* system shared memory structure initialization */
  sharedmem_id = shmget(IPC_PRIVATE, sizeof(ipc_structure), SHM_R | SHM_W);
  if (sharedmem_id == -1)
  {
    LOG_SYS(0, ("Can't allocate system shm structure.\nError: [%s]\n", strerror(errno)));
    exit(EXIT_ERROR_IPC);
  }

  /* user shared memory structure initialization */
  usr_shm_id = shmget(IPC_PRIVATE, lp_shm_size(), SHM_R | SHM_W);
  if (usr_shm_id == -1)
  {
    LOG_SYS(0, ("Can't allocate user shm structure.\nError: [%s]\n", strerror(errno)));
    exit(EXIT_ERROR_IPC);
  }

  usr_shm  = (online_user *)shmat(usr_shm_id, 0, 0);
  max_user_cnt = lp_shm_size() / sizeof(struct online_user);
  bzero(usr_shm, lp_shm_size());
  
  LOG_SYS(0, ("Init: Allocated shm segment (size=%d bytes)\n", lp_shm_size()));
  LOG_SYS(0, ("Init: Max %d concurent online users allowed\n", max_user_cnt));
  
  ipc_vars = (ipc_structure *)shmat(sharedmem_id, 0, 0);
  
  if (sharedmem_id == -1)
  {
    LOG_SYS(0, ("Can't attach shared memory segment.\nError: [%s]\n", strerror(errno)));
    exit(EXIT_ERROR_IPC);
  }

  /* init ipc_vars */  
  bzero(ipc_vars, sizeof(ipc_structure));
}


/**************************************************************************/
/* Lock userspace shared memory for read 				  */
/**************************************************************************/
void lock_ushm()
{
   /* Wait while write sem become 0 and then increment read sem by 1 */
   semop(semaphore_id, &shmr_lock[0], 2); 
}


/**************************************************************************/
/* Unlock userspace shared memory		 			  */
/**************************************************************************/
void unlock_ushm()
{
   /* decrease read sem by one */
   semop(semaphore_id, &shmr_unlock[0], 1);
}


/**************************************************************************/
/* Lock interprocess pipe for reading/writing 				  */
/**************************************************************************/
void lock_pipe(int pipe)
{
   /* waiting for unlock (when sem = 0) */
   if (pipe == INCOMING_PIPE) semop(semaphore_id, &inc_lock[0], 2);
   if (pipe == OUTGOING_PIPE) semop(semaphore_id, &out_lock[0], 2); 
}


/**************************************************************************/
/* Unlock interprocess pipe for reading/writing 			  */
/**************************************************************************/
void unlock_pipe(int pipe)
{
   if (pipe == INCOMING_PIPE) semop(semaphore_id, &inc_unlock[0], 1);
   if (pipe == OUTGOING_PIPE) semop(semaphore_id, &out_unlock[0], 1);
}


/**************************************************************************/
/* Lock incoming pipe for write 	 				  */
/**************************************************************************/
BOOL lock_incw()
{
   /* waiting for unlock (when sem = 0) and lock */
   semop(semaphore_id, &incw_lock[0], 2);
 
   return(True);
}


/**************************************************************************/
/* Unlock incoming pipe after writing to it				  */
/**************************************************************************/
BOOL unlock_incw()
{
   semop(semaphore_id, &incw_unlock[0], 1);
  
   return(True);
}


/**************************************************************************/
/* Lock ipc			 	 				  */
/**************************************************************************/
BOOL lock_ipcw()
{
   /* waiting for unlock (when sem = 0) and lock */
   semop(semaphore_id, &ipcw_lock[0], 2);
 
   return(True);
}


/**************************************************************************/
/* Unlock ipc								  */
/**************************************************************************/
BOOL unlock_ipcw()
{
   semop(semaphore_id, &ipcw_unlock[0], 1);
  
   return(True);
}


/**************************************************************************/
/* Set internal process_role variable to PACKET_ROLE, fork		  */
/* another process and add it to child_list 				  */
/**************************************************************************/
void fork_packet_processors(int num)
{
   pid_t child_pid;
   int i;
  
   /* we need to fork number of childs */
   for  (i=0; i<num; i++)
   {
      child_pid = fork();
      
      if (child_pid != 0)
      {
         /* Parent process, we should push pid to the list */  
         child_insert(child_pid, ROLE_PACKET);
      }
      else 
      {
         process_role = ROLE_PACKET;
         childSignals_Init();
         return;
      }
   }
}


/**************************************************************************/
/* Set internal process_role variable to EPACKET_ROLE, fork another 	  */
/* process and add it to child_list				  	  */
/**************************************************************************/
void fork_epacket_processor()
{
   pid_t child_pid;
  
   child_pid = fork();

   if (child_pid != 0)
   {
       /* Parent process, we should push to the list */
       child_insert(child_pid, ROLE_EPACKET); 
   }
   else
   {
       process_role = ROLE_EPACKET;
       childSignals_Init();
   }
}


/**************************************************************************/
/* Set internal process_role variable to EUSER_ROLE, fork another 	  */
/* process and add it to child_list				  	  */
/**************************************************************************/
void fork_euser_processor()
{
   pid_t child_pid;
  
   child_pid = fork();

   if (child_pid != 0)
   {
    /* Parent process, we should push pid to the list */
       child_insert(child_pid, ROLE_EUSER); 
   }
   else
   {
       process_role = ROLE_EUSER;
       childSignals_Init();
   }
}


/**************************************************************************/
/* Set internal process_role variable to EUSER_ROLE, fork another 	  */
/* process and add it to child_list				  	  */
/**************************************************************************/
void fork_etimer_processor()
{
   pid_t child_pid;
  
   child_pid = fork();

   if (child_pid != 0)
   {
       /* Parent process, we should push pid to the list */
       child_insert(child_pid, ROLE_ETIMER); 
   }
   else
   {
       process_role = ROLE_ETIMER;
       childSignals_Init();
   }
}


/**************************************************************************/
/* Set internal process_role variable to DEFRAG_ROLE, fork another 	  */
/* process and add it to child_list				  	  */
/**************************************************************************/
void fork_defrag_processor()
{
   pid_t child_pid;
  
   if (lp_v3_enabled())  
   {
      child_pid = fork();

      if (child_pid != 0)
      {
          /* Parent process, we should push pid to the list */
          child_insert(child_pid, ROLE_DEFRAG);
      }
      else
      {
          process_role = ROLE_DEFRAG;
          childSignals_Init();
      }
   }
}


/**************************************************************************/
/* Set internal process_role variable to ACTIONS_ROLE, fork another 	  */
/* process and add it to child_list				  	  */
/**************************************************************************/
void fork_actions_processor()
{
   pid_t child_pid;
  
   if (lp_actions_enabled())  
   {
      child_pid = fork();

      if (child_pid != 0)
      {
          /* Parent process, we should push pid to the list */
          child_insert(child_pid, ROLE_ACTIONS);
      }
      else
      {
          process_role = ROLE_ACTIONS;
          childSignals_Init();
      }
   }
}


/**************************************************************************/
/* Fork several packet-processors and one event processor	  	  */
/**************************************************************************/
void fork_childs()
{
   childs_check = 0;

   init_childs_list();
   
   /* set initial role */
   process_role = ROLE_SOCKET;
   
   fork_packet_processors(lp_min_childs());

   /* if we are SP - fork subprocesses */
   if (process_role == ROLE_SOCKET) fork_defrag_processor();
   if (process_role == ROLE_SOCKET) fork_euser_processor();
   if (process_role == ROLE_SOCKET) fork_epacket_processor();
   if (process_role == ROLE_SOCKET) fork_etimer_processor();
   if (process_role == ROLE_SOCKET) fork_actions_processor();
}


/**************************************************************************/
/* Switch to server directory.                             	  	  */
/**************************************************************************/
void switch_to_server_root(char *bin_path)
{

#ifdef ENABLE_SWITCH
  
   int slen = 0;
   char npath[128];
  
   /* we need to remove filename from command string */
   for (int i=strlen(bin_path); i>=0; i--)
      if (bin_path[i] == '/') { slen = i+1; break; }
      
   /* sanity check */
   if (slen > 128) slen = 128;
   snprintf(npath, slen+1, bin_path);
  
   /* chdir to server_root/bin */
   if (chdir(npath) != 0)
   {
      printf("Can't switch to server root: \"%s\"\n", strerror(errno));
      exit(EXIT_ERROR_FATAL);
   }
  
   /* chdir to server_root (../) */
   if (chdir("../") != 0)
   {
      printf("Can't switch to server root: \"%s\"\n", strerror(errno));
      exit(EXIT_ERROR_FATAL);
   }

   getcwd(npath, 128);

#else

   chdir("/");

#endif /* ENABLE_SWITCH */

}

