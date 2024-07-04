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
/* This unit implements online database calls (lookup, sseq control, 	  */
/* touch, vacuum, setstatus, transaction control, etc) for packet 	  */
/* processors 								  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"


/**************************************************************************/
/* Called from scheduler to validate nonrealtime online_users table   	  */
/**************************************************************************/
int check_online_table()
{
   PGresult *res;
   fstring dbcomm_str;

   PQclear(PQexec(users_dbconn, "BEGIN;"));

   res = PQexec(users_dbconn, "DECLARE uportal CURSOR FOR SELECT uin FROM Online_Users");

   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[DECLARE PORTAL IN ODB CHECK]");
      return(0);
   }
   else
   {
      PQclear(res);
   }

   /* Check every user in table */
   for (;;)
   {
      res = PQexec(users_dbconn, "FETCH IN uportal");
      if (PQresultStatus(res) != PGRES_TUPLES_OK)
      {
         handle_database_error(res, "[ODB CHECK FETCH FROM PORTAL]");
         break;
      }

      if (PQntuples(res) == 0)
      {
         PQclear(res);
         break;
      }

      if (!shm_user_exist(atoul(PQgetvalue(res, 0, 0))))
      {
         /* Delete this user from database only */
	 db_online_delete(atoul(PQgetvalue(res, 0, 0)), 0);
      }

      PQclear(res);
   }

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "UPDATE Online_Users SET lutm=%d",
   	    timeToLong(time(NULL)));

   PQclear(PQexec(users_dbconn, dbcomm_str));
   PQclear(PQexec(users_dbconn, "CLOSE uportal"));
   PQclear(PQexec(users_dbconn, "END;"));

   return (0);
}


/**************************************************************************/
/* This function called from scheduler to vacuum online tables	  	  */
/**************************************************************************/
int vacuum_online_tables()
{
   /* Well, only several tables need regulary vacuuming */
   PQclear(PQexec(users_dbconn, "VACUUM ANALYZE Online_Contacts"));
   PQclear(PQexec(users_dbconn, "VACUUM ANALYZE Online_Users"));
   PQclear(PQexec(users_dbconn, "VACUUM ANALYZE Fragment_Storage"));
   PQclear(PQexec(users_dbconn, "VACUUM ANALYZE Users_Messages"));

   LOG_SYS(50, ("Scheduled vacuuming server tables completed...\n"));

   return (0);
}


/**************************************************************************/
/* This func used to set server sequence to specified value for uin	  */
/**************************************************************************/
int db_online_update_servseq(struct online_user &temp_user)
{
   /* Depricated. Sseq handled by epacket processor */
   return (0);
}


/**************************************************************************/
/* This func will add specified number to server sequence		  */
/**************************************************************************/
int db_online_add_servseq(unsigned long uin, int number)
{
   /* Depricated. Sseq handled by epacket processor */
   return(0);
}


/**************************************************************************/
/* This func will update user's status			      		  */
/**************************************************************************/
int db_online_setstatus(struct online_user &temp_user)
{
   PGresult *res;
   cstring dbcomm_str;
   int      nRet;

   shm_setstatus(temp_user);

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "UPDATE Online_Users SET stat=%d, estat=%d WHERE uin=%lu",
	    temp_user.status, temp_user.estat, temp_user.uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SET STATUS]");
    nRet = -1;
   }
   else
   {
    nRet = ( (strcmp(PQcmdTuples(res),"") != 0) ? 0 : -1 );
      PQclear(res);
   }
  return(nRet);
}


/**************************************************************************/
/* This func delete all online users from database	      		  */
/**************************************************************************/
int db_online_clear()
{
   PGresult *res;

   res = PQexec(users_dbconn, "DELETE FROM Online_Users");

   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[Clear database]");
      return(-1);
   }

      PQclear(res);
      return(0);
}


/**************************************************************************/
/* This func update user's client state			      		  */
/**************************************************************************/
int db_online_setstate(struct online_user &temp_user, int state)
{
   PGresult *res;
   cstring dbcomm_str;
   int      nRet;

   shm_setstate(temp_user, state);

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "UPDATE Online_Users SET state=%d WHERE uin=%lu",
	    state, temp_user.uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SET STATE]");
    nRet = -1;
   }
   else
   {
    nRet = ( (strcmp(PQcmdTuples(res), "") != 0) ? 0 : -1 );
      PQclear(res);
   }
  return(nRet);
}


/**************************************************************************/
/* This func will update user's last update time	      		  */
/**************************************************************************/
int db_online_touch(struct online_user &temp_user)
{
   PGresult *res;
   cstring dbcomm_str;
  int      nRet;

  nRet = shm_touch(temp_user);

   /* If admin wants realtime uptodate online_users table */
   if (lp_realtime_odb())
   {
      /* exec select command on backend server */
      slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
              "UPDATE Online_Users SET lutm=%d WHERE uin=%lu",
   	       timeToLong(time(NULL)), temp_user.uin);

      res = PQexec(users_dbconn, dbcomm_str);
      if (PQresultStatus(res) != PGRES_COMMAND_OK)
      {
         handle_database_error(res, "[ONLINE TOUCH]");
      nRet = -1;
      }
      else
      {
      nRet = ( (strcmp(PQcmdTuples(res), "") != 0) ? 0 : -1 );
         PQclear(res);
      }
   }

  return nRet;
}


/**************************************************************************/
/* This function insert new user record to online database	 	  */
/**************************************************************************/
int  db_online_insert(struct online_user &temp_user)
{
   PGresult *res;
   cstring dbcomm_str;
   int      nRet;
   /* exec select command on backend server */

   if (!shm_add(temp_user)) return(-1);

   DEBUG(50, ("Adding user into online table (shm_index=%lu)\n", temp_user.shm_index));

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "INSERT INTO Online_Users values (%lu,%lu,%d,%d,%d,%lu,%lu,%d,%d,%d,%d,%lu,255,%d,%d,%d,%lu)",
	    temp_user.uin, ipToIcq(temp_user.ip), temp_user.udp_port, temp_user.tcp_port,
            temp_user.status, temp_user.uptime, temp_user.lutime, temp_user.ttl,
	    temp_user.protocol, temp_user.servseq, temp_user.servseq2,
	    temp_user.session_id, temp_user.tcpver, temp_user.estat, temp_user.active,
	    temp_user.shm_index);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[INSERT INTO ONLINE]");
      nRet = -1;
   }
   else
   {
      nRet = ( (strcmp(PQcmdTuples(res), "") != 0) ? 0 : -1 );
      PQclear(res);
   }
  return(nRet);
}


/**************************************************************************/
/* This function delete user record from online database	 	  */
/**************************************************************************/
int  db_online_delete(unsigned long to_uin, int shm)
{
   PGresult *res;
   fstring dbcomm_str;
   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "DELETE FROM Online_Users WHERE uin=%lu", to_uin);

   if (shm) shm_delete(to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[DELETE ONLINE USER]");
      return(-1);
   }

   PQclear(res);

   /* delete from online_profiles table */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "DELETE FROM Online_Profiles WHERE sn='%lu'", to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[DELETE ONLN PROFILE]");
      return(-1);
   }

   PQclear(res);

   if (lp_realtime_odb())
   {
      /* Just in case... I'll delete all expired records from database */
      /* We should do this only in online_db realtime mode             */
      slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
              "DELETE FROM Online_Users WHERE ((%lu-lutm) > (ttl+5)) AND (ttl > 0)",
	       time(NULL));

      res = PQexec(users_dbconn, dbcomm_str);

      if (PQresultStatus(res) != PGRES_COMMAND_OK)
      {
         handle_database_error(res, "[DELETE EXPIRED USERS]");
      }
      else
      {
      PQclear(res);
      }
   }

   return(0);
}


/**************************************************************************/
/* This function return user data from online database			  */
/**************************************************************************/
int db_add_online_lookup(unsigned long caller_uin, unsigned long to_uin,
			 struct online_user &temp_user)
{
   PGresult *res;
   fstring dbcomm_str;
  int      nRet;

   if (shm_lookup(to_uin, temp_user) != 0) return (-1);

   /* now we should find out if caller have rights to view user status */
   if (temp_user.status == ICQ_STATUS_PRIVATE)
   {
      slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
      "SELECT 1 FROM Online_Contacts WHERE (ouin=%lu) AND (tuin=%lu) AND (type=%d) LIMIT 1",
	        to_uin, caller_uin, VISIBLE_CONTACT);

      res = PQexec(users_dbconn, dbcomm_str);

      if (PQresultStatus(res) != PGRES_TUPLES_OK)
      {
         handle_database_error(res, "[ADD VISIBLE ONLINE CHECK]");
       nRet = -1;
      }
    else
      {
      nRet = ( (PQntuples(res) > 0) ? 0 : -1 );
         PQclear(res);
      }
   }
   else
   {
    /* we do not need to count, just try to fetch 1 record */
      slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
      "SELECT 1 FROM Online_Contacts WHERE (ouin=%lu) AND (tuin=%lu) AND (type=%d) LIMIT 1",
	        to_uin, caller_uin, INVISIBLE_CONTACT);

      res = PQexec(users_dbconn, dbcomm_str);

      if (PQresultStatus(res) != PGRES_TUPLES_OK)
      {
         handle_database_error(res, "[ADD INVISIBLE ONLINE CHECK]");
      nRet = -1;
      }
    else
      {
      nRet = ( (PQntuples(res) == 0) ? 0 : -1 );
         PQclear(res);
      }
   }
  return(nRet);
}


/**************************************************************************/
/* This function return user data from online database			  */
/**************************************************************************/
int db_online_lookup(unsigned long to_uin, struct online_user &temp_user)
{
   return(shm_lookup(to_uin, temp_user));
}


/**************************************************************************/
/* Return user data from online database if auser is in his contact list  */
/**************************************************************************/
int need_notification(unsigned long auin, unsigned long to_uin,
		      struct online_user &to_user)
{
   PGresult *res;
   fstring dbcomm_str;
   int      nRet;

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
    "SELECT * FROM Online_Users WHERE (uin=%lu) AND EXISTS(SELECT 1 FROM Online_Contacts WHERE (tuin=%lu) AND (ouin=%lu) AND (type=%d))",
	    to_uin, auin, to_uin, NORMAL_CONTACT);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[NEED NOTIFICATION]");
    nRet = -1;
   }
  else
   if (PQnfields(res) != ONLN_TBL_FIELDS)
   {
    LOG_SYS(0, ("Corrupted table structure in online_table: \n"));
      exit(EXIT_ERROR_DB_STRUCTURE);
   }
  else
  {
   if (PQntuples(res) > 0)
   {
      to_user.uin	 = atoul(PQgetvalue(res, 0, 0));
      to_user.ip	 = icqToIp(atoul(PQgetvalue(res, 0, 1)));
      to_user.udp_port	 = atol(PQgetvalue(res, 0,  2));
      to_user.tcp_port	 = atol(PQgetvalue(res, 0,  3));
      to_user.status	 = atol(PQgetvalue(res, 0,  4));
      to_user.uptime	 = atoul(PQgetvalue(res, 0,  5));
      to_user.lutime	 = atoul(PQgetvalue(res, 0,  6));
      to_user.ttl	 = atol(PQgetvalue(res, 0,  7));
      to_user.protocol	 = (unsigned short)atoi(PQgetvalue(res, 0, 8));
      to_user.servseq	 = atol(PQgetvalue(res, 0,  9));
      to_user.servseq2	 = atol(PQgetvalue(res, 0, 10));
      to_user.session_id = atoul(PQgetvalue(res, 0, 11));
      to_user.state	 = atol(PQgetvalue(res, 0, 12));
      to_user.tcpver	 = atol(PQgetvalue(res, 0, 13));
      to_user.estat 	 = atol(PQgetvalue(res, 0, 14));
      to_user.active	 = atol(PQgetvalue(res, 0, 15));

      nRet = 0;
   }
   else
   {
      nRet = -1;
    }
      PQclear(res);
   }
  return nRet;
}


/**************************************************************************/
/* This function return user data from online database			  */
/**************************************************************************/
int db_online_lookup(unsigned long to_uin,
		     struct online_user &temp_user,
		     unsigned short magnum1,
		     unsigned short magnum2)
{
   if ((to_uin == ADMIN_UIN) &&
       (ipc_vars->magic_num1 == magnum1) &&
       (ipc_vars->magic_num2 == magnum2))
   {
      temp_user.uin		= ADMIN_UIN;
      temp_user.ip		= icqToIp(INADDR_LOOPBACK);;
      temp_user.udp_port	= 4000;
      temp_user.tcp_port	= 4000;
      temp_user.status		= 0;
      temp_user.uptime		= 0;
      temp_user.lutime		= 0;
      temp_user.ttl		= 0;
      temp_user.protocol	= V3_PROTO;	/* internal proto */
      temp_user.servseq		= 0;
      temp_user.tcpver		= 0;
      temp_user.estat		= 0;
      temp_user.active		= 1;
      temp_user.dc_type		= 0;
      temp_user.dc_cookie	= 0;
      temp_user.uclass		= 0x400;	/* icq admin */

      ipc_vars->magic_num1 = random_num();
      ipc_vars->magic_num2 = random_num();

      return(0);
   }

   if (shm_lookup(to_uin, temp_user) == 0) return(0);

   return(-1);

}


/**************************************************************************/
/* This function return user data from online database and inc sseq 	  */
/**************************************************************************/
int db_online_sseq_open(struct online_user &temp_user)
{
   /* Depricated - sseq handled by epacket processor */
   return(0);
}


/**************************************************************************/
/* This function return user data from online database and inc sseq 	  */
/**************************************************************************/
int db_online_sseq_close(struct online_user &temp_user, int inc_num = 0)
{
   /* Depricated */
   return(0);
}


/**************************************************************************/
/* This func activate user's account			      		  */
/**************************************************************************/
int db_online_activate_user(unsigned long uin)
{
   PGresult *res;

   cstring dbcomm_str;

   shm_activate_user(uin);

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "UPDATE Online_Users SET active=1 WHERE uin=%lu", uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[ONLINE ACTIVATE]");
      return(-1);
   }

   if (strcmp(PQcmdTuples(res),"") != 0 )
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
/* Used to save user profile/away info in online_profiles table 	  */
/**************************************************************************/
int db_online_save_profile(char *sn, int type, char *mime,
                           char *data, int datasize)
{
   PGresult *res;
   fstring cmime;
   int nRet = 0;

   /* delete old record */
   slprintf(tempst3, sizeof(tempst3)-1,
           "DELETE FROM Online_Profiles WHERE sn='%s' AND type=%d", sn, type);

   res = PQexec(users_dbconn, tempst3);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[OPRF DELETE]");
      return(-1);
   }

   PQclear(res);
   if (datasize == 0) return(0); /* empty loc-info */

   /* now time to encode data and save all this shit to db */
   convert_to_postgres(cmime, sizeof(fstring)-1, mime);
   hexencode(tempst, data, datasize);
   slprintf(tempst3, sizeof(tempst3)-1,
           "INSERT INTO Online_Profiles values ('%s',%d,'%s','%s')",
	    sn, type, cmime, tempst);

   res = PQexec(users_dbconn, tempst3);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[OPRF INSERT]");
      nRet = -1;
   }
   else
   {
      nRet = ((strcmp(PQcmdTuples(res), "") != 0) ? 0 : -1);
      PQclear(res);
   }

   return(nRet);
}


/**************************************************************************/
/* Get user profile/away info from online_profiles table 	          */
/**************************************************************************/
int db_online_get_profile(char *sn, int type, char *mime, int max_mime,
                          char *data, int max_data)
{
   PGresult *res;
   int datasize = 0;

   slprintf(tempst3, sizeof(tempst3)-1,
           "SELECT mime,data FROM Online_Profiles WHERE (sn like '%s') AND (type=%d) LIMIT 1",
	    sn, type);

   res = PQexec(users_dbconn, tempst3);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[SELECT PRFL]");
      return(-1);
   }

   if (PQntuples(res) < 1) { PQclear(res); return(-1); }

   /* now time to decode data */
   snprintf(mime, max_mime, PQgetvalue(res, 0, 0));
   ITrans.translateToClient(mime);

   snprintf(tempst3, sizeof(tempst3)-1, PQgetvalue(res, 0, 1));
   datasize = hexdecode(data, tempst3, max_data);

   return(datasize);
}


