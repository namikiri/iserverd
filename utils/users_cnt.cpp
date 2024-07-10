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
/* This program retrive total number of users from database to show 	  */
/* on www (yes, it is CGI program)					  */
/*									  */
/**************************************************************************/

#include "includes.h"

PGconn   *dbconn  = NULL;

/*************************************************************************/
/* Function to exit gracefully					 	 */
/*************************************************************************/
void cgi_exit()
{
  printf ("Content-type: text/html\n");  printf ("Pragma: No-cache\n\n");
  printf ("N/A\n");

  PQfinish(dbconn);
}


/*************************************************************************/
/* This function make connection to users database		 	 */
/*************************************************************************/
void usersdb_connect()
{
   char	    *pghost, *pgport,  *pgoptions, *pgtty;
   char	    *dbName, *dblogin, *dbpassw;

   pghost  = "127.0.0.1";	/* address of db server (NULL - unix) */
   pgport  = "5432";		/* port of the backend */
   dbName  = "users_db";	/* database name */
   dblogin = "iserverd";	/* database user */
   dbpassw = "default";		/* database password */

   pgoptions = NULL;		/* special options for backend server */
   pgtty     = NULL;		/* debugging tty for the backend server */

				/* make a connection to the database */
   dbconn = PQsetdbLogin(pghost, pgport, pgoptions, pgtty, dbName,
                               dblogin, dbpassw);

   if (PQstatus(dbconn) == CONNECTION_BAD) cgi_exit();
}


/**************************************************************************/
/* now call connect_db, select count(*) and display this on std 	  */
/**************************************************************************/
int main(int argc, char **argv)
{
   unsigned long number;
   PGresult *res;

   usersdb_connect();

   res = PQexec(dbconn, "SELECT count(*) FROM users_info_ext");
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      PQclear(res);
      cgi_exit();
   }

   if (PQntuples(res) > 0)
   {
      char **valid = NULL;
      number = strtoul(PQgetvalue(res, 0, 0), valid, 10);
      PQclear(res);
      printf ("Content-type: text/html\n");      printf ("Pragma: No-cache\n\n");
      printf ("%lu", number);

   } else { cgi_exit(); }

   PQfinish(dbconn);
}

