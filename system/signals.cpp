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
/* Signal handlers for IServerd						  */
/*									  */
/**************************************************************************/


#include "includes.h"


/**************************************************************************/
/* Func: reliable signal() function 					  */
/**************************************************************************/
void (*rsignal(int signo, void (*hndlr)(int)))(int)
{
  struct sigaction act, oact;
  
  act.sa_handler = hndlr;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_RESTART;
  
  if (sigaction(signo, &act, &oact) < 0) return (SIG_ERR);
  
  return (oact.sa_handler);
}


/**************************************************************************/
/* Func: reliable signal blocking function 				  */
/**************************************************************************/
void block_signal(int signo)
{
  sigset_t signal_set;
  sigset_t old_signal_set;
  
  sigemptyset(&signal_set);
  sigaddset(&signal_set, signo);
  sigprocmask(SIG_BLOCK, &signal_set, &old_signal_set);
 
}


/**************************************************************************/
/* Func: reliable signal unblocking function 				  */
/**************************************************************************/
void unblock_signal(int signo)
{
  sigset_t signal_set;
  sigset_t old_signal_set;
  
  sigemptyset(&signal_set);
  sigaddset(&signal_set, signo);
  sigprocmask(SIG_UNBLOCK, &signal_set, &old_signal_set);
 
}


/**************************************************************************/
/* Func: SIGHUP handler							  */
/**************************************************************************/
RETSIGTYPE mySIGHUPHandler(int param)
{
    BOOL ret;

    LOG_SYS(0,("SIGHUP catched: Reloading configuration...\n"));
    if (lp_file_list_changed()) 
    {
       exit_ok = False;
       reload_in_progress = True;
       ret = lp_load(ICQ_CONFIG_FILE, False, False, True);
       reopen_logs();
       old_interface = bind_interface;
       init_bindinterface();
       udpserver_restart(lp_udp_port());
       killpg(0, SIGINT);
       exit_ok = True;
    }
    
    return;
}


/**************************************************************************/
/* Func: parent SIGINT handler						  */
/**************************************************************************/
RETSIGTYPE mySIGINTHandler(int param)
{   
   if (exit_ok)
   {
      LOG_SYS(0, ("SIGINT catched: Shutting down...\n"));
      signal(SIGINT, SIG_IGN);
      exit(EXIT_NORMAL);
   }
}


/**************************************************************************/
/* Func: child SIGINT handler						  */
/**************************************************************************/
RETSIGTYPE childSIGINTHandler(int param)
{   
   DEBUG(10, ("[%d/%d] SIGINT catched: Shutting down...\n", getpid(), process_role));
   PQfinish(users_dbconn);
     
   exit(EXIT_NORMAL);
}


/**************************************************************************/
/* Func: child SIGALRM handler						  */
/**************************************************************************/
RETSIGTYPE mySIGALRMHandler(int param)
{
   return;
}


/**************************************************************************/
/* Function: SIGCHLD handler for action processor			  */
/**************************************************************************/
RETSIGTYPE myAPSIGCHLDHandler(int param)
{
   pid_t              cpid = 1;
   int                status;
    
   while(cpid > 0)
   {
      cpid = waitpid(-1, &status, WNOHANG);
      if (cpid < 0) { return; }
      else if (cpid == 0) return;
   }
}


/**************************************************************************/
/* Function    : parent SIGCHLD handler					  */
/**************************************************************************/
RETSIGTYPE mySIGCHLDHandler(int param)
{
   pid_t              cpid = 1;
   int                status;
    
   while(cpid > 0)
   {
      cpid = waitpid(0, &status, WNOHANG);
      if (cpid < 0) { return; }
      else if (cpid == 0) return;

      if (!no_death_messages)
      {
        if ((WIFEXITED(status)) && (!reload_in_progress))
        {
           LOG_SYS(50, ("SIGCHLD: child exited: pid=[%ld], ec=[%d]%s\n", (unsigned long)cpid,
                   WEXITSTATUS(status), (WCOREDUMP(status) ? " (core dumped)" : "")));

           /* here we should check child exit code - may be it try to */
           /* tell us something */
           if (WEXITSTATUS(status) == EXIT_ERROR_DB_FAILURE)
           {
              LOG_ALARM(0, ("RDBMS server failure detected. Recovering started...\n"));
	      
              exit_ok = False;
              reload_in_progress = True;
              killpg(0, SIGINT);

              /* wait for database recovering */
              sleep(3);
              exit_ok = True;
           }

           /* here we should if there is problem with database connection */
           if (WEXITSTATUS(status) == EXIT_ERROR_DB_CONNECT)
           {
              exit_ok = False;
              reload_in_progress = True;
              killpg(0, SIGINT);

              /* wait for database recovering   */
	      /* BUGBUG - signals disabled here */
              wait_for_database();
              exit_ok = True;
           }
        }
        else if ((WIFSIGNALED(status)) && (!reload_in_progress))
        {
            LOG_SYS(50, ("SIGCHLD: child exited: pid=[%ld], signal=[%d], ec=[%d]%s\n",
                   (unsigned long)cpid, WTERMSIG(status), WEXITSTATUS(status), 
		   (WCOREDUMP(status) ? " (core dumped)" : "")));
        }
        else
        {
	    if (!reload_in_progress)
	    {
               LOG_SYS(50, ("SIGCHLD: child died with no status information pid=[%ld]\n",
                        (unsigned long)cpid));
	    }
        }

        if (child_lookup(cpid) != ROLE_BUSY)
        {
	    if (!reload_in_progress)
	    {
               LOG_SYS(50, ("SIGCHLD: child role was [%d], starting new one.\n", child_lookup(cpid)));
	    }   
            childs_check = 1;
        }	
      }
      
      child_delete(cpid); 
    }
}


/**************************************************************************/
/* Function    : SIGSEGV handler					  */
/**************************************************************************/
RETSIGTYPE mySIGSEGVHandler(int param)
{
  if (param != SIGSEGV)
  {
    LOG_SYS(0, ("Unknown signal (%d) in SIGSEGV handler. Skipped\n", param));
    return;
  }

  FILE *ftrace = NULL;
  
  /* hmm... bad idea to write this to tmp dir */
  ftrace = fopen("/tmp/isd-crashdump.txt", "a");
  
  if (ftrace != NULL)
  {
     fprintf(ftrace, "============================================================\n");
     fprintf(ftrace, "[%s] SIGSEGV: IServerd Segmentation Fault\n", time2str(time(NULL)));
     fprintf(ftrace, "------------------------------------------------------------\n");
     fprintf(ftrace, "System name: %s, compiler: %s\n", SYSTEM_UNAME, COMPILER_NAME);
     fprintf(ftrace, "Process role: %d, online_users_num: %lu, max_online: %d\n", 
                      process_role, ipc_vars->online_usr_num, max_user_cnt);

     fprintf(ftrace, "Stime: [%s] \nHtime: [%s]\n", 
                      time2str(server_started), time2str(config_loaded));
		   
     fprintf(ftrace, "Debug level: %d, log level: %d\n", DEBUGLEVEL, LOGLEVEL);
     fprintf(ftrace, "------------------------------------------------------------\n");
     fprintf(ftrace, "Queue: Packs: %lu, bytes: %lu, pmax: %lu, bmax: %lu, errs: %lu\n\n", 
                      ipc_vars->pack_in_queue, ipc_vars->byte_in_queue, 
   		      ipc_vars->max_queue_pack, ipc_vars->max_queue_size, 
		      ipc_vars->queue_send_errors);  
		   
     fprintf(ftrace, "IServerd stack backtrace dump: \n\n");

#ifdef HAVE_BACKTRACE  
     /* trying to dump stack backtrace */
     void *addr_array[32];
     int addr_num = backtrace(addr_array, 32);
     char **sym_array = backtrace_symbols(addr_array, addr_num);
     for (int i=0; i<addr_num; i++) fprintf(ftrace, ">> %s\n", sym_array[i]);
#else
     fprintf(ftrace, "[ glibc backtrace(...) is not supported, dump skipped ] \n\n");
#endif
  
     fprintf(ftrace, "\n\nOK... This is some info about crash, but it not enought\n");
     fprintf(ftrace, "If you have iserverd core around - try to get backtrace from it\n");
     fprintf(ftrace, "You can found instructions in BUGS file located in source tree\n\n");
     fprintf(ftrace, "Please send this information to iserverd authors.\n\n\n");     

     fclose(ftrace);
   }
   
   abort();
}


