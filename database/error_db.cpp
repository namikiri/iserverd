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
/* This file contain database error handler 			 	  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"


/**************************************************************************/
/* Main database error handler 						  */
/**************************************************************************/
int handle_database_error(PGresult *res, char where[40])
{
   ExecStatusType ErrorType;

   ErrorType = PQresultStatus(res);

   /* first of all I should check if there is error exist */
   if ((ErrorType != PGRES_BAD_RESPONSE)   &&
       (ErrorType != PGRES_EMPTY_QUERY)    &&
       (ErrorType != PGRES_NONFATAL_ERROR) &&
       (ErrorType != PGRES_FATAL_ERROR))
   {
      LOG_SYS(0, ("One of the child processes tell me about error\n"));
      LOG_SYS(0, ("But there is no error (%d). Please report to developers.\n", ErrorType));
      LOG_SYS(0, ("Reported place: %s\n", where));
      PQclear(res);
      return(0);
   }

   /* Now time to write log messages about error */
   LOG_SYS(0, ("Database query failed (%d). Place: %s\n", ErrorType, where));
   PQclear(res);

   if ((ErrorType == PGRES_FATAL_ERROR) ||
       (ErrorType == PGRES_NONFATAL_ERROR))
   {
      PQreset( users_dbconn );
      if (PQstatus(users_dbconn) == CONNECTION_BAD)
      {
         LOG_SYS(0, ("RDBMS Reconnection failed. PPs are shuting down...\n"));
	 
	 /* send signal to actions processor */
	 send_event2ap(papack, ACT_RDBMSFAIL, 0, 0, 0, 0, longToTime(time(NULL)), "");
	 sleep(2);
         exit(EXIT_ERROR_DB_CONNECT);
      }
      
      /* if reconnect is ok this was only small RDBMS error */
      send_event2ap(papack, ACT_RDBMSERR, ErrorType, 0, 0, 0, longToTime(time(NULL)), where);
      return(0);
   }
   
   return(1);
}


/**************************************************************************/
/* Notice processor (to detect database connections and tranz errors	  */
/**************************************************************************/
void uNoticeStub(void * arg, const char * message)
{
   char stripped[32];
   strncpy(stripped, message, 31);
   string_truncate(stripped, 28);

   DEBUG(50, ("NOTICEPROCESSOR: %s\n", stripped));

   /* This happens when one of the backends die badly              */
   /* this can cause Postgres shared IPC memory corrupt            */
   if (strequal(stripped, "NOTICE:  Message from Postgr"))
   {
      /* something really bad happens - psql shmem corrupted       */
      /* here we can do only one thing - exit with special         */
      /* exit code to inform parent that it should kill all        */
      /* processes and sleep a little */
      LOG_SYS(50, ("ERROR: Database IPC corrupted, exiting...\n"));
      exit(EXIT_ERROR_DB_FAILURE);
   }
   
   /* There are problem occupied inside transaction. And postgres   */
   /* move it to abort state. We should abort current transaction   */
   if ((strequal(stripped, "NOTICE:  current transaction")) &&
       (block_nprocessor != True))
   {
      LOG_ALARM(10, ("ERROR: Transaction aborted... Reconnecting...\n"));
      
      block_nprocessor = True;
      PQclear( PQexec( users_dbconn, "ABORT" ) );
      
/* COMMENTED_OUT: don't reconnect to save cpu time */
#if 0
      PQreset( users_dbconn );
      if (PQstatus(users_dbconn) == CONNECTION_BAD)
      {
         LOG_SYS(0, ("ERROR: Reconnection failed....\n"));
         exit(EXIT_ERROR_DB_CONNECT);
      }
#endif
   }
      
   block_nprocessor = False;
   return;
}


/**************************************************************************/
/* This function wait for database                                        */
/**************************************************************************/
void  wait_for_database()
{
   PGconn   *temp_dbconn  = NULL;  /* database connection structure	  */
   
   char	    *t_pghost, *t_pgport,  *t_pgoptions, *t_pgtty;
   char	    *t_dbName, *t_dblogin, *t_dbpassw;	    

   t_pghost  = lp_db_addr();
   t_pgport  = lp_db_port();
   t_dbName  = lp_db_users();
   t_dblogin = lp_db_user();
   t_dbpassw = lp_db_pass();
   
   t_pgoptions = NULL;
   t_pgtty     = NULL;


   temp_dbconn = PQsetdbLogin(t_pghost, t_pgport, t_pgoptions, t_pgtty, 
                               t_dbName, t_dblogin, t_dbpassw);
   
   /* Well, if there is problem - we should check db in cycle every 5 sec */
   if (PQstatus(temp_dbconn) == CONNECTION_BAD)
   {
      set_ps_display(ROLE_PAUSED, "(RDBMS is down)");
      LOG_SYS(0, ("Init: RDBMS is down. Waiting for working database...\n"));
      while (PQstatus(temp_dbconn) == CONNECTION_BAD)
      {
         PQreset( temp_dbconn );
         if (PQstatus(temp_dbconn) == CONNECTION_BAD) sleep(5);
      }
   }
      
   LOG_SYS(0, ("Init: RDBMS is online. Firing up processing...\n"));
   set_ps_display(ROLE_SOCKET, "");

   PQfinish( temp_dbconn );
}

