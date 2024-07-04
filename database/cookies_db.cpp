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
/* This unit implements cookie table insert/delete 			  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* Inserts cookie into table		 				  */
/**************************************************************************/
int db_cookie_insert(unsigned long uin, char *cookie, unsigned short type)
{
   PGresult *res;

   cstring dbcomm_str;

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
              "INSERT INTO login_cookies values (%lu, %lu, \'%s\', 0, %d)",
	       uin, time(NULL), cookie, type);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[DB_COOKIE_INSERT]");
      return(-1);
   }
   else
   {
      PQclear(res);
   }

   return(0);
}


/**************************************************************************/
/* Delete cookie with specified uin from table      			  */
/**************************************************************************/
int db_cookie_delete(unsigned long uin)
{
   PGresult *res;

   fstring dbcomm_str;
   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "DELETE FROM login_cookies WHERE uin=%lu", uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[COOKIE DELETE BY UIN]");
      return(-1);
   }

   if (strcmp(PQcmdTuples(res), "") != 0)
   {
      PQclear(res);
      return(0);
   }
   else
   {
      PQclear(res);
      return(-1);
   }
}


/**************************************************************************/
/* Delete specified cookie from table					  */
/**************************************************************************/
int db_cookie_delete(char *cookie)
{
   PGresult *res;

   cstring dbcomm_str;
   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "DELETE FROM login_cookies WHERE cookie like \'%s\'", cookie);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[COOKIE DELETE BY VALUE]");
      return(-1);
   }

   if (strcmp(PQcmdTuples(res), "") != 0)
   {
      PQclear(res);
      return(0);
   }
   else
   {
      PQclear(res);
      return(-1);
   }
}


/**************************************************************************/
/* Return uin number by its cookie. If specified cookie doesn't exist 	  */
/* function return 0, on database error function return 1	 	  */
/**************************************************************************/
unsigned long db_cookie_check(char *cookie)
{
   PGresult *res;
   unsigned long uin;

   cstring dbcomm_str;
   db_cookie_check_expired();

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT * FROM login_cookies WHERE (cookie like \'%s\') AND type=%d",
	    cookie, TYPE_COOKIE);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[USER COOKIE LOOKUP]");
      return(1);
   }

   if (PQnfields(res) != LOGC_TBL_FIELDS)
   {
      LOG_SYS(0, ("Corrupted table structure in login_cookie table: \n"));
      exit(EXIT_ERROR_DB_STRUCTURE);
   }

   if (PQntuples(res) > 0)
   {
      uin = atoul(PQgetvalue(res, 0, 0));
      PQclear(res);
      return (uin);
   }
   else
   {
      PQclear(res);
      return(0);
   }
}


/**************************************************************************/
/* Return cookie for specified uin number. If specified cookie 	  	  */
/* doesn't exist function return -1, on database error function return -2 */
/**************************************************************************/
int db_cookie_get(unsigned long uin, char *cookie, unsigned short &used,
                  unsigned short type)
{
   PGresult *res;
   char *cook = cookie;

   fstring dbcomm_str;
   db_cookie_check_expired();

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT * FROM login_cookies WHERE (uin=%lu) AND (type=%d)",
	    uin, type);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[USER COOKIE GET]");
      return(-2);
   }

   if (PQnfields(res) != LOGC_TBL_FIELDS)
   {
      LOG_SYS(0, ("Corrupted table structure in login_cookie table: \n"));
      exit(EXIT_ERROR_DB_STRUCTURE);
   }

   if (PQntuples(res) > 0)
   {
      used = (unsigned short)atol(PQgetvalue(res, 0, 3));
      strncpy(cook, PQgetvalue(res, 0, 2), 254);

      PQclear(res);
      return (0);
   }
   else
   {
      PQclear(res);
      return(-1);
   }
}


/**************************************************************************/
/* Increment field "used" in cookie with specified uin			  */
/**************************************************************************/
void db_cookie_use(unsigned long uin)
{
   PGresult *res;

   fstring dbcomm_str;
   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "UPDATE login_cookies SET used=used+1 WHERE uin=%lu", uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[COOKIE UPDATE BY UIN]");
      return;
   }

   if (strcmp(PQcmdTuples(res), "") != 0)
   {
      PQclear(res);
      return;
   }
   else
   {
      PQclear(res);
      return;
   }
}


/**************************************************************************/
/* Increment field "used" in cookie 					  */
/**************************************************************************/
void db_cookie_use(char *cookie)
{
   PGresult *res;

   cstring dbcomm_str;
   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "UPDATE login_cookies SET used=used+1 WHERE cookie like \'%s\'", cookie);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[COOKIE UPDATE BY VALUE]");
      return;
   }

   if (strcmp(PQcmdTuples(res), "") != 0)
   {
      PQclear(res);
      return;
   }
   else
   {
      PQclear(res);
      return;
   }
}


/**************************************************************************/
/* Deletes expired unused cookies from table				  */
/**************************************************************************/
void db_cookie_check_expired()
{
   PGresult *res;

   fstring dbcomm_str;
   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "DELETE FROM login_cookies WHERE %lu = (%lu - cdate)",
	    lp_v7_cookie_timeout(), time(NULL));

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[COOKIE DELETE EXPIRED]");
      return;
   }

   if (strcmp(PQcmdTuples(res), "") != 0)
   {
      PQclear(res);
      return;
   }
   else
   {
      PQclear(res);
      return;
   }
}

