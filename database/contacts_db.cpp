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
/* This unit implements database functions related contacts processing    */
/*									  */
/**************************************************************************/


#include "includes.h"

/**************************************************************************/
/* This func delete all online contacts from database	      		  */
/**************************************************************************/
int db_contacts_clear()
{
   PGresult *res;

   res = PQexec(users_dbconn, "DELETE FROM Online_Contacts");

   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[Clear contacts]");
      return(-1);
   }

   PQclear(res);
   return(0);
}


/**************************************************************************/
/* Insert new contact list in contact database 				  */
/**************************************************************************/
int db_contact_insert(unsigned long uin, int number,
		      unsigned long *contact, int type,
		      unsigned long rid)
{
   /* There is two ways to add contact list to database */
   /* 1. Use copy from stdin (int_db_contact_insert_c)  */
   /* 2. Use set of inserts  (int_db_contact_insert_i)  */
   /* first is much faster than second                  */
   int_db_contact_insert_c(uin, number, contact, type, rid);

   return(0);
}


/**************************************************************************/
/* This is the internal function to add user contact list to database     */
/* It use copy from stdin statement                                       */
/**************************************************************************/
int int_db_contact_insert_c(unsigned long uin, int number,
		            unsigned long *contact, int type,
			    unsigned long rid)
{
   PGresult *res;

   cstring dbcomm_str;

   DEBUG(100, ("Copy %d records for %lu to contact database.\n",
            number, uin));

   res = PQexec(users_dbconn, "COPY Online_Contacts FROM stdin");

   if (PQresultStatus(res) != PGRES_COPY_IN)
   {
      handle_database_error(res, "[DB_CONTACT_COPY_IN]");
      return(-1);
   }
   else
   {
      PQclear(res);
   }

   for (int i=0; i<number; i++)
   {
      slprintf(dbcomm_str, sizeof(dbcomm_str)-1, "%lu\t%lu\t%d\t%lu\n",
	       uin, contact[i], type, rid);

      PQputline(users_dbconn, dbcomm_str);
   }

   PQputline(users_dbconn, "\\.\n");
   PQendcopy(users_dbconn);

   return(0);
}


/**************************************************************************/
/* This is the internal function to add user contact list to database     */
/* It use set of insert commands in single transaction                    */
/**************************************************************************/
int int_db_contact_insert_i(unsigned long uin, int number,
		            unsigned long *contact, int type,
			    unsigned long rid)
{
   PGresult *res;

   cstring dbcomm_str;

   DEBUG(100, ("Inserting %d records for %lu to contact database.\n",
            number, uin));

   PQclear(PQexec(users_dbconn, "BEGIN"));

   for (int i=0; i<number; i++)
   {
      slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
              "INSERT INTO Online_Contacts values (%lu, %lu, %d, %lu)",
	      uin, contact[i], type, rid);

      res = PQexec(users_dbconn, dbcomm_str);
      if (PQresultStatus(res) != PGRES_COMMAND_OK)
      {
         handle_database_error(res, "[DB_CONTACT_INSERT]");
         return(-1);
      }
      else
      {
         PQclear(res);
      }
   }

   PQclear(PQexec(users_dbconn, "END"));

   return(0);
}


/**************************************************************************/
/* Delete contact records with specified owner from database		  */
/**************************************************************************/
int db_contact_delete(unsigned long uin)
{
   PGresult *res;

   fstring dbcomm_str;
   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "DELETE FROM Online_Contacts WHERE ouin=%lu", uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[CONTACT LIST DELETE]");
      return(-1);
   }

   if (strcmp(PQcmdTuples(res),"") != 0)
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
/* Delete contact records with specified owner from database		  */
/**************************************************************************/
int db_contact_delete(unsigned long ouin, unsigned long tuin,
		      unsigned short type)
{
   PGresult *res;

   fstring dbcomm_str;
   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "DELETE FROM Online_Contacts WHERE ouin=%lu AND tuin=%lu AND type=%d", ouin, tuin, type);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[CONTACT DELETE (typed)]");
      return(-1);
   }

   if (strcmp(PQcmdTuples(res),"") != 0)
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
/* Lookup all users who should be alerted if user with specified	  */
/* uin change status. This function return number of retrieved users	  */
/* also it alloc memory for returned list, "contact" pointer should be 	  */
/* NULL to avoid memory leak 						  */
/**************************************************************************/
int db_contact_lookup(unsigned long uin, int type, unsigned long **contact)
{
   PGresult *res;
   int number;

   fstring dbcomm_str;
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT ouin FROM Online_Contacts WHERE tuin=%lu AND type=%d",
	   uin, type);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[CONTACT LOOKUP]");
      return(0);
   }

   number = PQntuples(res);

   if (number > 0)
   {
      if (PQnfields(res) < 1)
      {
         LOG_SYS(0, ("Corrypted table structure in db: \"%s\"\n",
	             lp_db_users()));
         exit(EXIT_ERROR_DB_STRUCTURE);
      }

      *contact = (unsigned long *)malloc(sizeof(unsigned long)*number);
      DEBUG(200, ("Trying to malloc %d bytes for %d records (result: %06X)\n",
                  (sizeof(unsigned long)*number), number,
		  (unsigned long)*contact));

      for (int i=0;i<number;i++) (*contact)[i] = atoul(PQgetvalue(res, i, 0));

      PQclear(res);
      return(number);
   }
   else
   {
      PQclear(res);
      return(0);
   }
}


/**************************************************************************/
/* Delete contact records with specified owner and type from database	  */
/**************************************************************************/
int db_contact_delete(unsigned long ouin, unsigned short type)
{
   PGresult *res;

   fstring dbcomm_str;
   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "DELETE FROM Online_Contacts WHERE ouin=%lu AND type=%d", ouin, type);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[CONTACT DELETE (typed2)]");
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


