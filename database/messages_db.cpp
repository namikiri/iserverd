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
/* This unit implements offline messages database calls (add & delete	  */
/* for packet processors						  */
/*                                                                        */
/**************************************************************************/


#include "includes.h"


/**************************************************************************/
/* This func add message to database (if user is not online) 		  */
/**************************************************************************/
int db_add_message(struct msg_header &msg_hdr, char *message)
{
  PGresult *res;
  bigstring nmessage;
  time_t stime;

  convert_to_postgres(nmessage, sizeof(nmessage)-1 ,message);

  stime = time(NULL);

  bigstring dbcomm_str;
  /* exec select command on backend server */
  slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
       "INSERT INTO Users_Messages values (%lu, %lu, %lu, %d, '%s')",
            msg_hdr.touin, msg_hdr.fromuin, mktime(gmtime(&stime)),
	    msg_hdr.mtype, nmessage);

  res = PQexec(users_dbconn, dbcomm_str);
  if (PQresultStatus(res) != PGRES_COMMAND_OK)
  {
     handle_database_error(res, "[INSERT MESSAGE]");
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
/* This function remove messages from database				  */
/**************************************************************************/
int db_del_messages(unsigned long to_uin, unsigned long last_time)
{
  PGresult *res;

  cstring dbcomm_str;
  /* exec select command on backend server */
  slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
       "DELETE FROM Users_Messages WHERE to_uin=%lu and msg_date<=%lu",
        to_uin, last_time);

  res = PQexec(users_dbconn, dbcomm_str);
  if (PQresultStatus(res) != PGRES_COMMAND_OK)
  {
     handle_database_error(res, "[DB DELETE MESSAGE]");
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

