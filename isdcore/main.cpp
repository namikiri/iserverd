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
/* IServerd starting and initialization					  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

extern pstring debugf; 
extern pstring systemf; 
extern pstring alarmf;
extern pstring usersf;
extern BOOL append_log;


/**************************************************************************/
/* Program entry point 							  */
/**************************************************************************/
int main(int argc, char **argv)
{
   pid_t spid;
   pstring configf;

   real_argc = argc;
   real_argv = argv;
  
   /* First of all we need to switch to server root */
   switch_to_server_root(argv[0]);

  /* Process command line options */
   init_globals();
   process_command_line_opt(argc, argv);
   slprintf(configf, sizeof(configf)-1, lp_config_file());

   /* Global parameters initialization */ 
   slprintf(debugf,  sizeof(debugf),  lp_dbglog_path());
   umask(022); append_log = lp_append_logs(); reopen_logs(); 
   
   /* Load configuration file */
   if (!lp_load(configf,False,False,True))
   {
      printf("FATAL ERROR: Can't find config file: \"%s\"\n", configf);
      exit(EXIT_CONFIG);
   }

   /* Open log files */
   init_syslog_logs();
   
   spid = pidfile_pid();
   
   if (spid != 0) 
   {
      LOG_SYS(0,("ERROR: IServerd running. Pidfile & process %d exists\n", (int)spid));
      exit(EXIT_ERROR_ANOTHER_PROCESS);
   }
   
   LOG_SYS(0,("\n"));
   LOG_SYS(0,("Init: IServerd started: [Ver: %s]\n", Iversion));
   rvs_order = setup_byteorder();
   random_check();
 
   server_started = time(NULL);
  
   TimeInit();
   init_random();

   /* IPC Initialization (semaphores, shared memory, pipes) 	     */
   incoming_pipe_init();  /* pipe from parent to packet-processors   */
   outgoing_pipe_init();  /* pipe from PP's to event-processor 	     */
   toutgoing_pipe_init(); /* pipe from PP's to socket-processor	     */
   epacket_pipe_init();	  /* pipe from PP's to EPP for event signals */
   euser_pipe_init();     /* pipe from PP's to EUP for event signals */
   defrag_pipe_init();	  /* pipe from PP's to FP 		     */
   actions_pipe_init();	  /* pipe from PP's to AP 		     */
   
   /* Selecting server mode. If we are daemon - fork(), 
   close all files write pid file and detach from control 
   terminal */

   switch (lp_server_mode())
   {
      case MOD_STANDALONE: setup_syslogging("", True);   
                           setup_alrlogging("", True);
			   setup_usrlogging("", True);
			   setup_logging("", True);
			   break;
      case MOD_DAEMON:	   become_daemon();
      			   break;
      default:		   become_daemon();
      			   break;
   }

   init_ps_display(real_argc, real_argv);

   /* we need to determine interface to bind on. If there is no
   Bind interface line in config file this function setup a first
   interface of the machine */
   init_bindinterface();
  
   /* Init signals and connect handlers */
   if ( Signals_Init() != True )
   {
      LOG_SYS(0, ("FATAL ERROR: Signals handling problem, exiting\n"));
      exit(EXIT_CONFIG);
   }
  
   /* Register parent exit() cleaning up function */
   if (atexit((void(*)(void))Server_cleanup) != 0) 
   {
      LOG_SYS(0, ("WARNING: Can't set cleanup function\n"));
   }

   init_translate();      /* load translation table */

   /* IPC Initialization (semaphores, shared memory, pipes) */  
   ipc_objects_init();
   LOG_SYS(10, ("Init: IPC objects init success\n"));
   
   udpserver_start(lp_udp_port(), 3);

   aimsockfd = -1; msnsockfd = -1;
   
   if (lp_aim_port() > 0) aim_tcp_server_start(lp_aim_port());
   if (lp_msn_port() > 0) msn_tcp_server_start(lp_msn_port());
   
   wwp_socket_init();
   LOG_SYS(10, ("Init: WWP socket initialized successfully\n"));

   /* check if we should start without database */  
   if (lp_degradated_mode()) wait_for_database();
   
   /* Now we should check database and fix it if need */
   if (check_and_fix_database(lp_db_users(), lp_db_user(), 
       lp_db_pass(), lp_db_addr(), lp_db_port()) != True)
   {
      LOG_SYS(0, ("ERROR: Database problem... Exiting...\n"));
      exit(EXIT_ERROR_DB_STRUCTURE);
   }
  
   LOG_SYS(10, ("Init: Starting packet/event processors\n"));

   fork_childs();
   process_server();
}

