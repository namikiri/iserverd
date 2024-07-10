/**************************************************************************/
/*                                                                        */
/* Copyright (c) 2000-2005 by Alexandr V. Shutko, Khabarovsk, Russia      */
/* All rights reserved.                                                   */
/*                                                                        */
/* Redistribution and use in source and binary forms, with or without     */
/* modification, are permitted provided that the following conditions     */
/* are met:                                                               */
/* 1. Redistributions of source code must retain the above copyright      */
/*    notice, this list of conditions and the following disclaimer.       */
/* 2. Redistributions in binary form must reproduce the above copyright   */
/*    notice, this list of conditions and the following disclaimer in     */
/*    the documentation and/or other materials provided with the          */
/*    distribution.                                                       */
/*                                                                        */
/* THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND */
/* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE  */
/* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR     */
/* PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS  */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,    */
/* OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT   */
/* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR     */
/* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,  */
/* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE   */
/* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,      */
/* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                     */
/*                                                                        */
/* This unit implement database management functions (check, init, etc)   */
/* table parameters for automatic database initialization and checks      */
/*                                                                        */
/**************************************************************************/

#include "includes.h"


/**************************************************************************/
/* This routine will check database structure and fix it in some cases    */
/**************************************************************************/
BOOL check_and_fix_database(char *dbname, char *dbuser, char *dbpass,
			    char *dbaddr, char *dbport)
{
   PGconn   *check_conn;
   PGresult *res;
   BOOL check_result[16];
   cstring dbcomm_str;
   int o_number, n_number;

   /* I found that people often use default "sicq" password for    */
   /* their databases. This is very stupid and I must do something */
   /* about to solve this REAL security problem                    */

   if (!strcmp(dbpass, "sicq") || (!strcmp(dbpass, "DEFAULT")))
   {
      LOG_SYS(0, ("Hey man, ARE YOU CRAZY ? You are trying to use default database password\n"));
      LOG_SYS(0, ("Sorry, I'm not so crazy as you... Try to change password first\n"));
      exit(EXIT_ERROR_DB_CONNECT);
   }

   LOG_SYS(0, ("Init: %s:%s db=%s , user=%s\n\tChecking DB integrity...\n", dbaddr, dbport, dbname, dbuser));
   /* First mark all tests failed */
   for(int i=0; i<8; i++) check_result[i] = False;

   /* database template1 connect */
   char     *pgoptions, *pgtty;

   pgoptions = NULL;    /* special options for backend server   */
   pgtty     = NULL;    /* debugging tty for the backend server */

   /* check if specified database exist & mark result */
   /* make a connection to the database */
   check_conn = PQsetdbLogin(dbaddr, dbport, pgoptions, pgtty,
			     "template1", dbuser, dbpass);

   if (PQstatus(check_conn) == CONNECTION_BAD)
   {
      LOG_SYS(0, ("Can't check database... Connection failed.\n%s", PQerrorMessage(check_conn)));
      exit(EXIT_ERROR_DB_CONNECT);
   }

   PQsetNoticeProcessor(check_conn,
   (void (*)(void *, const char *))uNoticeStub, NULL);

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
	   "SELECT COUNT(*) FROM pg_database WHERE datname like '%s'",
	    dbname);

   res = PQexec(check_conn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[TEST DATABASE]");
      return (False);
   }

   if (PQntuples(res) > 0)
   {
      if (atoi(PQgetvalue(res, 0, 0)) > 0)
      {
	 check_result[DB_EXIST] = True;
	 PQclear(res);
      }
      else
      {
	 LOG_ALARM(0, ("Database %s not found, please create it with createdb tool\n", dbname));
	 PQclear(res);
	 exit(EXIT_ERROR_DB_NOT_FOUND);
      }
   }
   else
   {
      LOG_SYS(0, ("Can't fetch information about database %s\n", dbname));
      PQclear(res);
      exit(EXIT_ERROR_DB_NOT_FOUND);
   }

   PQfinish(check_conn);

   /* now it is time to connect to specified database and check all tables */
   check_conn = PQsetdbLogin(dbaddr, dbport, pgoptions, pgtty,
				dbname, dbuser, dbpass);

   if (PQstatus(check_conn) == CONNECTION_BAD)
   {
      LOG_SYS(0, ("Can't connect db to check tables...\n"));
      LOG_SYS(0, ("%s", PQerrorMessage(check_conn)));
      exit(EXIT_ERROR_DB_CONNECT);
   }

   PQsetNoticeProcessor(check_conn,
   (void (*)(void *, const char *))uNoticeStub, NULL);

   /* check if mess table exist and correct */
   check_result[MESS_TBL_OK] = check_user_tbl(check_conn,
			       MESS_TBL_NAME, MESS_TBL_FIELDS);
   /* check if lock table exist and correct */
   check_result[LOCK_TBL_OK] = check_user_tbl(check_conn,
			       LOCK_TBL_NAME, LOCK_TBL_FIELDS);
   /* check if deps table exist and correct */
   check_result[DEPS_TBL_OK] = check_user_tbl(check_conn,
			       DEPS_TBL_NAME, DEPS_TBL_FIELDS);
   /* check if cont table exist and correct */
   check_result[CONT_TBL_OK] = check_user_tbl(check_conn,
			       CONT_TBL_NAME, CONT_TBL_FIELDS);
   /* check if onln table exist and correct */
   check_result[ONLN_TBL_OK] = check_user_tbl(check_conn,
			       ONLN_TBL_NAME, ONLN_TBL_FIELDS);
   /* check if dfrg table exist and correct */
   check_result[DFRG_TBL_OK] = check_user_tbl(check_conn,
			       DFRG_TBL_NAME, DFRG_TBL_FIELDS);

   check_result[LOGC_TBL_OK] = check_user_tbl(check_conn,
			       LOGC_TBL_NAME, LOGC_TBL_FIELDS);

   /* check if ssi table exist and correct */
   check_result[USSI_TBL_OK] = check_user_tbl(check_conn,
			       USSI_TBL_NAME, USSI_TBL_FIELDS);

   /* check if ssi table exist and correct */
   check_result[UPRM_TBL_OK] = check_user_tbl(check_conn,
			       UPRM_TBL_NAME, UPRM_TBL_FIELDS);

   /* check if online_profiles table exist and correct */
   check_result[OPRF_TBL_OK] = check_user_tbl(check_conn,
			       OPRF_TBL_NAME, OPRF_TBL_FIELDS);

   /* delete old corrupted tables and create new one */
   if (check_result[MESS_TBL_OK] != True)
   {
       delete_user_tbl(check_conn, MESS_TBL_NAME);
       create_user_tbl(check_conn, MESS_TBL_CREATE, MESS_TBL_NAME);
   }

   if (check_result[LOCK_TBL_OK] != True)
   {
       delete_user_tbl(check_conn, LOCK_TBL_NAME);
       create_user_tbl(check_conn, LOCK_TBL_CREATE, LOCK_TBL_NAME);
   }


   if (check_result[DEPS_TBL_OK] != True)
   {
       delete_user_tbl(check_conn, DEPS_TBL_NAME);
       create_user_tbl(check_conn, DEPS_TBL_CREATE, DEPS_TBL_NAME);
   }

   if (check_result[CONT_TBL_OK] != True)
   {
       delete_user_tbl(check_conn, CONT_TBL_NAME);
       create_user_tbl(check_conn, CONT_TBL_CREATE, CONT_TBL_NAME);
   }

   if (check_result[ONLN_TBL_OK] != True)
   {
       delete_user_tbl(check_conn, ONLN_TBL_NAME);
       create_user_tbl(check_conn, ONLN_TBL_CREATE, ONLN_TBL_NAME);
   }

   if (check_result[DFRG_TBL_OK] != True)
   {
       delete_user_tbl(check_conn, DFRG_TBL_NAME);
       create_user_tbl(check_conn, DFRG_TBL_CREATE, DFRG_TBL_NAME);
   }

   if (check_result[LOGC_TBL_OK] != True)
   {
       delete_user_tbl(check_conn, LOGC_TBL_NAME);
       create_user_tbl(check_conn, LOGC_TBL_CREATE, LOGC_TBL_NAME);
   }

   if (check_result[USSI_TBL_OK] != True)
   {
       delete_user_tbl(check_conn, USSI_TBL_NAME);
       create_user_tbl(check_conn, USSI_TBL_CREATE, USSI_TBL_NAME);
   }

   if (check_result[UPRM_TBL_OK] != True)
   {
       delete_user_tbl(check_conn, UPRM_TBL_NAME);
       create_user_tbl(check_conn, UPRM_TBL_CREATE, UPRM_TBL_NAME);
   }

   if (check_result[OPRF_TBL_OK] != True)
   {
       delete_user_tbl(check_conn, OPRF_TBL_NAME);
       create_user_tbl(check_conn, OPRF_TBL_CREATE, OPRF_TBL_NAME);
   }

   /* several checks on users database we should detect old and */
   /* new formats of users info table and convert it to new     */
   /* format if should */

   check_result[USER_TBL1_OK] = check_user_tbl2(check_conn,
				USER_TBL1_NAME, USER_TBL1_FIELDS);
   check_result[USER_TBL2_OK] = check_user_tbl(check_conn,
				USER_TBL2_NAME, USER_TBL2_FIELDS);

   /* check if we haven't both tables */
   if ((check_result[USER_TBL1_OK] != True) &&
       (check_result[USER_TBL2_OK] != True))
   {
       delete_user_tbl(check_conn, USER_TBL2_NAME);
       create_user_tbl(check_conn, USER_TBL2_CREATE, USER_TBL2_NAME);
       create_user_tbl(check_conn, USER_TBL2_CREATE1, USER_TBL2_NAME);
       delete_user_tbl(check_conn, USER_TBL1_NAME);
       create_user_tbl(check_conn, USER_TBL1_CREATE, USER_TBL1_NAME);
   }

   /* check if we have table1 and haven't table2 */
   if ((check_result[USER_TBL2_OK] == True) &&
       (check_result[USER_TBL1_OK] != True))
   {
       delete_user_tbl(check_conn, USER_TBL1_NAME);
       create_user_tbl(check_conn, USER_TBL1_CREATE, USER_TBL1_NAME);
   }

   /* add column martial routine */
   if ((check_result[USER_TBL1_OK] == True) &&
       (check_result[USER_TBL2_OK] != True))
   {
       slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
    	   "ALTER TABLE Users_Info_Ext ADD COLUMN martial int2 DEFAULT 0;");

       res = PQexec(check_conn, dbcomm_str);

       if (PQresultStatus(res) == PGRES_COMMAND_OK)
       {

	  handle_database_error(res, "[ADD COLUMN martial]");

	  check_result[USER_TBL2_OK] = check_user_tbl(check_conn,
				USER_TBL2_NAME, USER_TBL2_FIELDS);
       }

       if (check_result[USER_TBL2_OK] != True)
       {
          slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
    	    "ALTER TABLE Users_Info_Ext ADD COLUMN wzip2 text;");

	  res = PQexec(check_conn, dbcomm_str);

	  if (PQresultStatus(res) == PGRES_COMMAND_OK)
	  {

	    handle_database_error(res, "[ADD COLUMN wzip2]");

	    check_result[USER_TBL2_OK] = check_user_tbl(check_conn,
				USER_TBL2_NAME, USER_TBL2_FIELDS);
	  }

       }

       if (check_result[USER_TBL2_OK] != True)
       {
          slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
    	    "ALTER TABLE Users_Info_Ext ADD COLUMN hzip2 text;");

	  res = PQexec(check_conn, dbcomm_str);

	  if (PQresultStatus(res) == PGRES_COMMAND_OK)
	  {

	    handle_database_error(res, "[ADD COLUMN hzip2]");

	    check_result[USER_TBL2_OK] = check_user_tbl(check_conn,
				USER_TBL2_NAME, USER_TBL2_FIELDS);
	  }

       }

       if (check_result[USER_TBL2_OK] != True)
       {
          slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
    	    "ALTER TABLE Users_Info_Ext ADD COLUMN bcountry int4;");

	  res = PQexec(check_conn, dbcomm_str);

	  if (PQresultStatus(res) == PGRES_COMMAND_OK)
	  {

	    handle_database_error(res, "[ADD COLUMN bcountry]");

	    check_result[USER_TBL2_OK] = check_user_tbl(check_conn,
				USER_TBL2_NAME, USER_TBL2_FIELDS);
	  }

       }

       if (check_result[USER_TBL2_OK] != True)
       {
          slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
    	    "ALTER TABLE Users_Info_Ext ADD COLUMN bstate text;");

	  res = PQexec(check_conn, dbcomm_str);

	  if (PQresultStatus(res) == PGRES_COMMAND_OK)
	  {

	    handle_database_error(res, "[ADD COLUMN bstate]");

	    check_result[USER_TBL2_OK] = check_user_tbl(check_conn,
				USER_TBL2_NAME, USER_TBL2_FIELDS);
	  }

       }

       if (check_result[USER_TBL2_OK] != True)
       {
          slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
    	    "ALTER TABLE Users_Info_Ext ADD COLUMN bcity text;");

	  res = PQexec(check_conn, dbcomm_str);

	  if (PQresultStatus(res) == PGRES_COMMAND_OK)
	  {

	    handle_database_error(res, "[ADD COLUMN bcity]");

	    check_result[USER_TBL2_OK] = check_user_tbl(check_conn,
				USER_TBL2_NAME, USER_TBL2_FIELDS);
	  }

       }

   }

   /* check if we haven't both tables */
   if ((check_result[USER_TBL1_OK] == True) &&
       (check_result[USER_TBL2_OK] != True))
   {
       delete_user_tbl(check_conn, USER_TBL2_NAME);
       create_user_tbl(check_conn, USER_TBL2_CREATE, USER_TBL2_NAME);
       create_user_tbl(check_conn, USER_TBL2_CREATE1, USER_TBL2_NAME);

       /* now we should move all users_info from table1 */
       /* to table2 with insert (select statement)      */
       slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
	   "INSERT INTO Users_Info_Ext (SELECT * FROM Users_Info);");

       res = PQexec(check_conn, dbcomm_str);

       if (PQresultStatus(res) != PGRES_COMMAND_OK)
       {
	  handle_database_error(res, "[MOVE2NEW TABLE]");
	  return(False);
       }

       /* now we should check if all data are moved */
       slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
	   "SELECT COUNT(*) FROM Users_Info", dbname);

       res = PQexec(check_conn, dbcomm_str);
       if (PQresultStatus(res) != PGRES_TUPLES_OK)
       {
	  handle_database_error(res, "[COMPARE OLD & NEW]");
	  return (False);
       }

       o_number = atoi(PQgetvalue(res, 0, 0));
       PQclear(res);

       /* now we should check if all data are moved */
       slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
	   "SELECT COUNT(*) FROM Users_Info_Ext", dbname);

       res = PQexec(check_conn, dbcomm_str);
       if (PQresultStatus(res) != PGRES_TUPLES_OK)
       {
	  handle_database_error(res, "[COMPARE OLD & NEW #2]");
	  return (False);
       }

       n_number = atoi(PQgetvalue(res, 0, 0));
       PQclear(res);

       if (n_number != o_number)
       {
	  LOG_SYS(0, ("Number of records in old and new tables doesn't match.\n"));
	  return(False);
       }

       /* We can't delete users_info table if inherited table exists */
       delete_user_tbl(check_conn, "register_requests");
       /* Now we can delete old table and create view instead */
       delete_user_tbl(check_conn, USER_TBL1_NAME);
       create_user_tbl(check_conn, USER_TBL1_CREATE, USER_TBL1_NAME);
   }

#ifdef HAVE_GD
   /* Now we clear regcode table */
   delete_user_tbl(check_conn, REGI_TBL_NAME);
   create_user_tbl(check_conn, REGI_TBL_CREATE, REGI_TBL_NAME);
#endif

   PQfinish(check_conn);

   return(True);
}


/**************************************************************************/
/* This routine will check specified table                                */
/**************************************************************************/
BOOL check_user_tbl(PGconn *dbconn, char *tbl_name, int fldnum)
{
   PGresult *res;
   cstring dbcomm_str;

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
	   "SELECT COUNT(*) FROM pg_tables WHERE tablename LIKE '%s'",
	    tbl_name);

   res = PQexec(dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[TEST-TABLE]");
      return(False);
   }

   if (PQntuples(res) < 0)
   {
     LOG_SYS(0, ("Problem fetching information about table from pg_tables...\n"));
     PQclear(res);
     return (False);
   }

   if (atol(PQgetvalue(res, 0, 0)) == 0)
   {
     LOG_SYS(0, ("No \"%s\" table!\n", tbl_name));
     PQclear(res);
     return (False);
   }

   /* uff... table exist... but we should check if it have correct number
   of fields (i think checking field names and types is not neccessary) */

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
	   "SELECT * FROM %s WHERE false;", tbl_name);

   res = PQexec(dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[TEST TABLE STRUCTURE]");
      return(False);
   }

   if (PQnfields(res) != fldnum)
   {
      LOG_SYS(0, ("Table %s has struct problem...\n",tbl_name));
      LOG_SYS(0, ("It have %d fields instead of %d...\n", PQnfields(res), fldnum));
      PQclear(res);
      return(False);
   }


   return(True);
}


/**************************************************************************/
/* This routine will check specified table                                */
/* we need this to check if view exist or not.                            */
/**************************************************************************/
BOOL check_user_tbl2(PGconn *dbconn, char *tbl_name, int fldnum)
{
   PGresult *res;
   cstring dbcomm_str;

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
	   "select * FROM %s WHERE false;", tbl_name);

   res = PQexec(dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      PQclear(res);
      return(False);
   }

   if (PQnfields(res) != fldnum)
   {
      PQclear(res);
      LOG_SYS(0, ("Table %s has struct problem...\n",tbl_name));
      LOG_SYS(0, ("It have %d fields instead of %d...\n", PQnfields(res), fldnum));
      return(False);
   }

   return(True);
}


/**************************************************************************/
/* This routine will create specified table                               */
/**************************************************************************/
BOOL create_user_tbl(PGconn *dbconn, char *cr_query, char *t_name)
{
   PGresult *res;

   LOG_SYS(0, ("New table %s ...", t_name));
   res = PQexec(dbconn, cr_query);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[CREATE TABLE]");
      return(False);
   }

   LOG_SYS(0, (" Ok\n"));
   return(True);
}


/**************************************************************************/
/* This routine will grant permissions on table to specified user         */
/**************************************************************************/
BOOL grant_permissions(PGconn *dbconn, char *tbl_name, char *username)
{
   PGresult *res;
   cstring dbcomm_str;

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
	   "GRANT ALL ON %s TO %s", tbl_name, username);

   res = PQexec(dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      PQclear(res);
      return(False);
   }

   return(True);

}


/**************************************************************************/
/* This routine will delete specified table                               */
/**************************************************************************/
BOOL delete_user_tbl(PGconn *dbconn, char *tblname)
{
   PGresult *res;
   cstring dbcomm_str;

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
	   "DROP TABLE %s", tblname);

   res = PQexec(dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      PQclear(res);
      return(False);
   }

   return(True);
}

