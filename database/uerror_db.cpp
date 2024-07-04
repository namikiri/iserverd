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

   /* Now time to write log messages about error */
   printf("Database query failed (%d). Place: %s\n", ErrorType, where);
   PQclear(res);

   if ((ErrorType == PGRES_FATAL_ERROR) ||
       (ErrorType == PGRES_NONFATAL_ERROR))
   {
      PQreset( users_dbconn );
      if (PQstatus(users_dbconn) == CONNECTION_BAD)
      {
         printf("Database connection terminated\n");
         exit(EXIT_ERROR_DB_CONNECT);
      }

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

   if (strequal(stripped, "NOTICE:  Message from Postgr"))
   {
      printf("ERROR: Database IPC corrupted, exiting...\n");
      exit(EXIT_ERROR_DB_FAILURE);
   }

   return;
}


/**************************************************************************/
/* This function wait for database                                        */
/**************************************************************************/
void  wait_for_database()
{
   return;
}

