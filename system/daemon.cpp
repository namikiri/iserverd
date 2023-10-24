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
/* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE   */
/* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 	  */
/* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.			  */
/*									  */
/* IServerd daemonization :)						  */
/*									  */
/**************************************************************************/


#include "includes.h"

struct rlimit flimit;

/**************************************************************************/
/* return the pid in a pidfile. return 0 if the process (or pidfile)      */
/* does not exist 							  */
/**************************************************************************/
pid_t pidfile_pid()
{
   FILE *fd;
   char pidstr[20];
   unsigned int ret;

   fd = fopen(lp_pid_path(), "r");
   if (fd == NULL) 
   {
      return 0;
   }

   ZERO_ARRAY(pidstr);

   fread(pidstr, sizeof(pidstr)-1, 1, fd ); 
   ret = atoi(pidstr);
	
   if (!process_exists((pid_t)ret)) 
   {
      goto noproc;
   }

   if (Lock(fd) == 0) 
   {
      /* we could get the lock - it can't be a IServerd process */
      goto noproc;
   }
   
   fclose(fd);
   return (pid_t)ret;

 noproc:
   
   fclose(fd);
   return 0;
}


/**************************************************************************/
/* This proc write pid file to lp_pid_path and lock it			  */
/**************************************************************************/
BOOL write_pid()
{
   FILE    *fd;
   pid_t   pid;

   pid = pidfile_pid();
   
   if (pid != 0) 
   {
      LOG_SYS(0,("ERROR: IServerd running. Pidfile & process %d exists.\n", (int)pid));
      return(False);
   }

   unlink(lp_pid_path()); /* We can do this.. WE SHOULD DO THIS TO AVOID SYMLINKS */
   
   fd = fopen(lp_pid_path(), "w");
   if (fd == NULL) 
   {
      LOG_SYS(0,("ERROR: can't open %s.\n", lp_pid_path()));
      LOG_SYS(0,("ERROR: %s\n", strerror(errno)));
      return(False);
   }

   
   if (Lock(fd)!=0) 
   {
      LOG_SYS(0,("ERROR: can't get lock on pidfile. Error was %s\n", strerror(errno)));
      return(False);
   }

   rewind(fd);
   (void)fprintf(fd, "%ld", (long int)getpid());
   (void)fflush(fd);
   DEBUG(3,("Wrote pid file...\n"));

   return(True);

   /* Leave pid file open & locked for the duration... */
}


/**************************************************************************/
/* Go to daemon mode							  */
/**************************************************************************/
int become_daemon(void)
{

#ifndef HAVE_SETSID
   int ttyfd;
#endif

   if (sys_fork()) _exit(EXIT_FORK);

   LOG_SYS(4, ("Init: Detaching from control terminal\n"));    
#ifdef HAVE_SETSID
  /* setsid() is the preferred way to disassociate from the */
  /* controlling terminal                                   */
  setsid();
#else
  /* Open /dev/tty to access our controlling tty (if any) */
  if( (ttyfd = open("/dev/tty",O_RDWR)) != -1)
  {
    if(ioctl(ttyfd,TIOCNOTTY,NULL) == -1) 
    {
      LOG_SYS(0, ("Can't detach from controling terminal (ioctl error).\n")); 
      exit(EXIT_CONFIG);
    }

    close(ttyfd);
  }
#endif /* HAVE_SETSID */
   
   /* we don't need signals from terminal and pipes */    
   if (getppid() != 1)	   /* check if we are running by init */
   {
      signal(SIGTTOU, SIG_IGN);
      signal(SIGTTIN, SIG_IGN);
      signal(SIGTSTP, SIG_IGN);
      signal(SIGPIPE, SIG_IGN);
   }
    
   /* close stdio streams */
   close(fileno(stdin));
   close(fileno(stdout));
   close(fileno(stderr));
    
   if (write_pid() != True) 
   {
      exit(EXIT_ERROR_ANOTHER_PROCESS);
   }
   else
   {
      LOG_SYS(0, ("Init: Wrote & lock server pidfile\n"));
   }

   LOG_SYS(4, ("Init: Creating new process group\n"));

#ifdef HAVE_SETPGID
   setpgid(0,getpid());
#else
# ifdef SETPGRP_VOID
   setpgrp();
# else
   setpgrp(0,getpid());
# endif
#endif

   return(0);
}


/**************************************************************************/
/* Init signals handlers for socket processor				  */
/**************************************************************************/
BOOL Signals_Init(void)
{
   if ( (rsignal(SIGHUP,  (void(*)(int))mySIGHUPHandler)  == SIG_ERR)  ||
        (rsignal(SIGINT,  (void(*)(int))mySIGINTHandler)  == SIG_ERR)  ||
        (rsignal(SIGQUIT, (void(*)(int))mySIGINTHandler)  == SIG_ERR)  ||
        (rsignal(SIGTERM, (void(*)(int))mySIGINTHandler)  == SIG_ERR)  ||
        (rsignal(SIGCHLD, (void(*)(int))mySIGCHLDHandler) == SIG_ERR)  ||
        (rsignal(SIGSEGV, (void(*)(int))mySIGSEGVHandler) == SIG_ERR)  ||
	(rsignal(SIGPIPE, (void(*)(int))mySIGALRMHandler) == SIG_ERR))
   {
       return(False);
   }

   return(True);
}


/**************************************************************************/
/* Init signals handlers for childs					  */
/**************************************************************************/
void childSignals_Init(void)
{
   rsignal(SIGHUP,  (void(*)(int))childSIGINTHandler);
   rsignal(SIGALRM, (void(*)(int))childSIGINTHandler);
   rsignal(SIGINT,  (void(*)(int))childSIGINTHandler);
   rsignal(SIGQUIT, (void(*)(int))childSIGINTHandler);
   rsignal(SIGTERM, (void(*)(int))childSIGINTHandler);
   rsignal(SIGCHLD, (void(*)(int))childSIGINTHandler);
   rsignal(SIGSEGV, (void(*)(int))mySIGSEGVHandler);
}

