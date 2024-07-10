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
/* This unit contain functions related broadcasting - status broadcasting */
/* message broadcasting, alert broadcasting. This unit work with database */
/* directly to increase perfomance and reduce database queries 		  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* Purpose: online event broadcasting for users wanted this (by contact)  */
/* This will cause 3 complex database queries (instead of (csz-ofn)*3+1)  */
/**************************************************************************/
void broadcast_online( struct online_user &auser )
{
   PGresult *res;			/* struct for database requests   */
   cstring dbcomm_str;  		/* string space for query         */
   int users_cnt;			/* number of users to alert       */
   struct online_user to_user;		/* structure for target user      */
   struct online_user *puser;

   /* first of all we should request online users that have target 	  */
   /* in their contacts and after that lock database rows          	  */

   PQclear( PQexec(users_dbconn, "BEGIN" ));

/* Define query to avoid multistring literals */
/* This is query for non-invisible users      */
#define BCST_NPAM54 \
"SELECT uin,ishm FROM online_users \
 JOIN \
 ( \
    SELECT ouin FROM online_contacts \
    WHERE (tuin=%lu) AND (type=%d) \
    EXCEPT \
    ( \
       SELECT tuin FROM online_contacts \
       WHERE (ouin=%lu) AND (type=%d) \
    ) \
    LIMIT %d \
 ) \
 AS TMP on uin=TMP.ouin \
 WHERE active=1"

/* Define query to avoid multistring literals */
/* This is query for invisible users          */
#define BCST_PAM74 \
"SELECT uin,ishm FROM online_users \
 JOIN \
 ( \
    SELECT ouin FROM online_contacts \
    WHERE (tuin=%lu) AND (type=%d) \
    INTERSECT \
    ( \
       SELECT tuin FROM online_contacts \
       WHERE (ouin=%lu) AND (type=%d) \
    ) \
    LIMIT %d \
 ) \
 AS TMP on uin=TMP.ouin \
 WHERE active=1"

   if (auser.status != ICQ_STATUS_PRIVATE)
   {
      slprintf(dbcomm_str, sizeof( dbcomm_str )-1,
	       BCST_NPAM54, auser.uin, NORMAL_CONTACT, auser.uin,
	       INVISIBLE_CONTACT, lp_v7_max_contact_size());
   }
   else
   {
      slprintf(dbcomm_str, sizeof( dbcomm_str )-1,
		BCST_PAM74, auser.uin, NORMAL_CONTACT, auser.uin,
		VISIBLE_CONTACT, lp_v7_max_contact_size());
   }

   res = PQexec(users_dbconn, dbcomm_str);

   if (PQresultStatus( res ) != PGRES_TUPLES_OK )
   {
      handle_database_error(res, "[BROADCAST ONLINE]");
      PQclear( PQexec(users_dbconn, "ABORT") );
      return;
   }

   /* number of returned users (tuples with information) */
   users_cnt = PQntuples( res );
   DEBUG(100, ("Complex query return %d users to alert...\n", users_cnt));

   /* now it is time to send online alert to all users in loop */
   if ( PQnfields( res ) == 2 )
   {
      for( int i=0; i<users_cnt; i++ )
      {
         /* first fill online_user structure */
         to_user.uin	    = atoul( PQgetvalue(res, i, 0));
	 to_user.shm_index  = atoul( PQgetvalue(res, i, 1));

	 /* get shm data by index and copy it into to_user */
	 puser = shm_iget_user(to_user.uin, to_user.shm_index);
	 if (puser == NULL) continue;

	 memcpy((void *)&to_user, (const void *)puser,
	         sizeof(struct online_user));

         /* now we can alert found user about incoming online event */
         send_user_online( to_user, auser );
      }
   }

   PQclear( res );
   PQclear( PQexec(users_dbconn, "END" ));
}

/**************************************************************************/
/* Purpose: offline event broadcasting for users wanted this (by contact) */
/* This will cause 3 complex database queries (instead of (csz-ofn)*3+1)  */
/**************************************************************************/
void broadcast_offline( unsigned long uin )
{
   PGresult *res;			/* struct for database requests   */
   fstring dbcomm_str;  		/* string space for query         */
   int users_cnt;			/* number of users to alert       */
   struct online_user to_user;		/* structure for target user      */
   struct online_user *puser;

   /* first of all we should request online users that have target 	  */
   /* in their contacts and after that lock database rows          	  */

#define BCST_AF157 \
"SELECT uin,ishm FROM online_users \
 JOIN \
 ( \
    SELECT ouin FROM online_contacts \
    WHERE (tuin=%lu) AND (type=%d) \
    GROUP BY ouin \
    LIMIT %d \
 ) \
 AS TMP on uin=TMP.ouin \
 WHERE (active=1)"

   PQclear( PQexec(users_dbconn, "BEGIN" ));
   slprintf(dbcomm_str, sizeof( dbcomm_str )-1,
	    BCST_AF157, uin, NORMAL_CONTACT, lp_v7_max_contact_size());

   res = PQexec(users_dbconn, dbcomm_str);

   if (PQresultStatus( res ) != PGRES_TUPLES_OK )
   {
      handle_database_error(res, "[BROADCAST OFFLINE]");
      PQclear( PQexec(users_dbconn, "ABORT") );
      return;
   }

   /* number of returned users (tuples with information) */
   users_cnt = PQntuples( res );
   /* now it is time to send online alert to all users in loop */
   if ( PQnfields( res ) == 2 )   /* sanity check - to avoid core dumps */
   {
      for( int i=0; i<users_cnt; i++ )
      {
         /* first fill online_user structure */
         to_user.uin	    = atoul( PQgetvalue(res, i, 0));
	 to_user.shm_index  = atoul( PQgetvalue(res, i, 1));

	 /* get shm data by index and copy it into to_user */
	 puser = shm_iget_user(to_user.uin, to_user.shm_index);
	 if (puser == NULL) continue;

	 memcpy((void *)&to_user, (const void *)puser,
	         sizeof(struct online_user));

         /* all ready we can send packet */
         send_user_offline( to_user, uin );
      }
   }

   PQclear( res );
   PQclear( PQexec(users_dbconn, "END" ));
}


/**************************************************************************/
/* Purpose: Status change event broadcasting				  */
/* This will cause 3 complex database queries (instead of (csz-ofn)*3+1)  */
/**************************************************************************/
void broadcast_status( struct online_user &auser, unsigned long old_status )
{
   PGresult *res;			/* struct for database requests   */
   cstring dbcomm_str, priv_str; 	/* string space for query         */
   int users_cnt;			/* number of users to alert       */
   struct online_user to_user;		/* structure for target user      */
   struct online_user *puser;

   /* first of all we should request online users that have target 	  */
   /* in their contacts and after that lock database rows          	  */

   PQclear( PQexec(users_dbconn, "BEGIN" ));

/* Define query to avoid multistring literals    */
/* Query to broadcast status change (to private) */
#define BCST_ASTP228 \
"SELECT uin,ishm FROM online_users \
 JOIN \
 ( \
    SELECT ouin FROM online_contacts \
    WHERE (tuin=%lu) AND (type=%d) \
    EXCEPT \
    ( \
       SELECT tuin FROM online_contacts \
       WHERE (ouin=%lu) AND (type=%d) \
    ) \
    LIMIT %d \
 ) \
 AS TMP on uin=TMP.ouin \
 WHERE active=1"

/* Define query to avoid multistring literals      */
/* Query to broadcast status change (to private) */
#define BCST_ASTPI248 \
"SELECT uin,ishm FROM online_users \
 JOIN \
 ( \
    SELECT ouin FROM online_contacts \
    WHERE (tuin=%lu) AND (type=%d) \
    INTERSECT \
    ( \
       SELECT tuin FROM online_contacts \
       WHERE (ouin=%lu) AND (type=%d) \
    ) \
    LIMIT %d \
 ) \
 AS TMP on uin=TMP.ouin \
 WHERE active=1"

   if (((auser.status & 0x0000ffff) == ICQ_STATUS_PRIVATE) &&
       ((old_status & 0x0000ffff) != ICQ_STATUS_PRIVATE))
   {

      slprintf(dbcomm_str, sizeof( dbcomm_str )-1,
		 BCST_ASTP228, auser.uin, NORMAL_CONTACT, auser.uin,
		 INVISIBLE_CONTACT, lp_v7_max_contact_size());

      slprintf(priv_str, sizeof( dbcomm_str )-1,
		 BCST_ASTPI248, auser.uin, NORMAL_CONTACT, auser.uin,
		 VISIBLE_CONTACT, lp_v7_max_contact_size());

      res = PQexec(users_dbconn, dbcomm_str);

      if (PQresultStatus( res ) != PGRES_TUPLES_OK )
      {
         handle_database_error(res, "[BROADCAST STATUS]");
	 PQclear( PQexec(users_dbconn, "ABORT") );
         return;
      }

      /* number of returned users (tuples with information) */
      users_cnt = PQntuples( res );

      /* now it is time to send online alert to all users in loop */
      if ( PQnfields( res ) == 2 )
      {
         for( int i=0; i<users_cnt; i++ )
         {
            /* first fill online_user structure */
            to_user.uin	    = atoul( PQgetvalue(res, i, 0));
   	    to_user.shm_index  = atoul( PQgetvalue(res, i, 1));

	    /* get shm data by index and copy it into to_user */
	    puser = shm_iget_user(to_user.uin, to_user.shm_index);
	    if (puser == NULL) continue;

	    memcpy((void *)&to_user, (const void *)puser,
	            sizeof(struct online_user));

            /* all ready so we can send packet */
            send_user_offline( to_user, auser.uin );
         }
      }

      PQclear( res );

      /* send status for hidden users */
      res = PQexec(users_dbconn, priv_str);

      if (PQresultStatus( res ) != PGRES_TUPLES_OK )
      {
         handle_database_error(res, "[BROADCAST STATUS2]");
	 PQclear( PQexec( users_dbconn, "ABORT") );
         return;
      }

      /* number of returned users (tuples with information) */
      users_cnt = PQntuples( res );
      /* now it is time to send online alert to all users in loop */
      if ( PQnfields( res ) == 2 )   /* sanity check - to avoid core dumps */
      {
         for( int i=0; i<users_cnt; i++ )
         {
            /* first fill online_user structure */
            to_user.uin	       = atoul( PQgetvalue(res, i, 0));
	    to_user.shm_index  = atoul( PQgetvalue(res, i, 1));

	    /* get shm data by index and copy it into to_user */
	    puser = shm_iget_user(to_user.uin, to_user.shm_index);
	    if (puser == NULL) continue;

	    memcpy((void *)&to_user, (const void *)puser,
	            sizeof(struct online_user));

            /* all ready we can send packet */
            send_user_status( to_user, auser );
         }
      }

      PQclear( res );
      PQclear( PQexec(users_dbconn, "END" ));
      return;
   }

/* Define query to avoid multistring literals      */
/* Query to broadcast status change (from private) */
#define BCST_ASFP352 \
"SELECT uin,ishm FROM online_users \
 JOIN \
 ( \
    SELECT ouin FROM online_contacts \
    WHERE (tuin=%lu) AND (type=%d) \
    EXCEPT \
    ( \
       SELECT tuin FROM online_contacts \
       WHERE (ouin=%lu) AND (type=%d) \
    ) \
    LIMIT %d \
 ) \
 AS TMP on uin=TMP.ouin \
 WHERE active=1"

   if (((auser.status & 0x0000ffff) != ICQ_STATUS_PRIVATE) &&
       ((old_status & 0x0000ffff) == ICQ_STATUS_PRIVATE))
   {

      slprintf(dbcomm_str, sizeof( dbcomm_str )-1,
	       BCST_ASFP352, auser.uin, NORMAL_CONTACT, auser.uin,
	       INVISIBLE_CONTACT, lp_v7_max_contact_size());

      res = PQexec(users_dbconn, dbcomm_str);

      if (PQresultStatus( res ) != PGRES_TUPLES_OK )
      {
         handle_database_error(res, "[BROADCAST STATUS_PV]");
	 PQclear( PQexec( users_dbconn, "ABORT") );
         return;
      }

      /* number of returned users (tuples with information) */
      users_cnt = PQntuples( res );

      /* now it is time to send online alert to all users in loop */
      if ( PQnfields( res ) == 2 )
      {
         for( int i=0; i<users_cnt; i++ )
         {
            /* first fill online_user structure */
            to_user.uin	    = atoul( PQgetvalue(res, i, 0));
	    to_user.shm_index  = atoul( PQgetvalue(res, i, 1));

	    /* get shm data by index and copy it into to_user */
	    puser = shm_iget_user(to_user.uin, to_user.shm_index);
	    if (puser == NULL) continue;

	    memcpy((void *)&to_user, (const void *)puser,
	            sizeof(struct online_user));

            /* all ready we can send packet */
            send_user_online( to_user, auser );
         }
      }

      PQclear( res );

      PQclear( PQexec(users_dbconn, "END" ));
      return;
   }

/* Define query to avoid multistring literals      */
/* Query to broadcast status change (from private) */
#define BCST_ASN417 \
"SELECT uin,ishm FROM online_users \
 JOIN \
 ( \
    SELECT ouin FROM online_contacts \
    WHERE (tuin=%lu) AND (type=%d) \
    EXCEPT \
    ( \
       SELECT tuin FROM online_contacts \
       WHERE (ouin=%lu) AND (type=%d) \
    ) \
    LIMIT %d \
 ) \
 AS TMP on uin=TMP.ouin \
 WHERE active=1"

   if (((auser.status & 0x0000ffff) != ICQ_STATUS_PRIVATE) &&
       ((old_status & 0x0000ffff) != ICQ_STATUS_PRIVATE))
   {
      slprintf(dbcomm_str, sizeof( dbcomm_str )-1,
	        BCST_ASN417, auser.uin, NORMAL_CONTACT, auser.uin,
	        INVISIBLE_CONTACT, lp_v7_max_contact_size());

      res = PQexec(users_dbconn, dbcomm_str);

      if (PQresultStatus( res ) != PGRES_TUPLES_OK )
      {
         handle_database_error(res, "[BROADCAST STATUS_PV2]");
	 PQclear( PQexec( users_dbconn, "ABORT" ) );
         return;
      }

      /* number of returned users (tuples with information) */
      users_cnt = PQntuples( res );
      /* now it is time to send online alert to all users in loop */
      if ( PQnfields( res ) == 2 )
      {
         for( int i=0; i<users_cnt; i++ )
         {
            /* first fill online_user structure */
            to_user.uin	    = atoul( PQgetvalue(res, i, 0));
	    to_user.shm_index  = atoul( PQgetvalue(res, i, 1));

	    /* get shm data by index and copy it into to_user */
	    puser = shm_iget_user(to_user.uin, to_user.shm_index);
	    if (puser == NULL) continue;

	    memcpy((void *)&to_user, (const void *)puser,
	            sizeof(struct online_user));

            /* all ready so we can send packet */
            send_user_status( to_user, auser );
         }
      }

      PQclear( res );
      PQclear( PQexec(users_dbconn, "END" ));
      return;
   }

   /* just in case. This code should be unreachable */
   PQclear( PQexec(users_dbconn, "END" ));
}

/**************************************************************************/
/* Purpose: Broadcast administrator message to online (and possibly to    */
/* offline) users.							  */
/**************************************************************************/
void broadcast_message(BOOL online_only, unsigned long department,
		       char *message, unsigned short mess_type,
		       unsigned long from_uin)
{
   PGresult *res;		/* structure pointer for db requests */
   cstring dbcomm_str;		/* string for db queries */
   long users_cnt;
   struct online_user to_user;
   struct online_user *puser;
   struct msg_header msg_hdr;
   unsigned long curr_time;
   int oust_size = 0;

   bzero((void *)&msg_hdr, sizeof(msg_hdr));

/* ------------------------------------------ */
/* First stage: Send messages to online users */
/* ------------------------------------------ */

/* Define query to avoid multistring literals */
/* broadcast message for specified department */
#define BCST_MD502 \
"SELECT uin,ishm FROM Online_Users \
 WHERE (active=1) AND uin IN \
 ( \
    SELECT uin FROM Users_Info_Ext \
    WHERE wdepart='%lu' \
 ) \
 AND uin<>%lu"

/* Define query to avoid multistring literals */
/* broadcast message for all users            */
#define BCST_MA515 \
"SELECT uin,ishm FROM Online_Users \
 WHERE (active=1) AND (uin<>%lu)"

   PQclear( PQexec(users_dbconn, "BEGIN" ));

   /* if depart = 0 we'll send msg 2 all */
   if (department != 0)
   {
      slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
	       BCST_MD502, department, from_uin);
   }
   else
   {
      slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
               BCST_MA515, from_uin);
   }

   res = PQexec(users_dbconn, dbcomm_str);

   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[BROADCAST-MESS2ONLINE]");
      return;
   }

   users_cnt      = PQntuples(res);
   curr_time      = timeToLong(time(NULL));
   oust_size      = sizeof(struct online_user);
   msg_hdr.msglen = strlen(message);

   /* now it is time to send online alert to all users in loop */
   for( int i=0; i<users_cnt; i++ )
   {
      /* first fill online_user structure */
      to_user.uin	 = atoul( PQgetvalue(res, i, 0));
      to_user.shm_index  = atoul( PQgetvalue(res, i, 1));

      /* get shm data by index and copy it into to_user */
      puser = shm_iget_user(to_user.uin, to_user.shm_index);
      if (puser == NULL) continue;

      memcpy((void *)&to_user, (const void *)puser, oust_size);

      /* all things ready, now we can send packet */
      msg_hdr.fromuin  = from_uin;
      msg_hdr.touin    = to_user.uin;
      msg_hdr.mtype    = mess_type;
      msg_hdr.from_ver = V3_PROTO;
      msg_hdr.mtime    = curr_time;

      send_online_message(msg_hdr, to_user, message);
   }

   PQclear(res);
   PQclear( PQexec(users_dbconn, "END" ));

/* -------------------------------------------- */
/* Second stage: Send messages to offline users */
/* -------------------------------------------- */

/* Define query to avoid multistring literals */
/* broadcast message for department (offline) */
#define BCST_MDO576 \
"SELECT uin FROM Users_Info_Ext \
 WHERE uin NOT IN \
 ( \
    SELECT uin FROM Online_Users \
 ) \
 AND wdepart='%lu'"

/* Define query to avoid multistring literals */
/* broadcast message for all offline users    */
#define BCST_MAO586 \
"SELECT uin FROM Users_Info_Ext \
 WHERE uin NOT IN \
 ( \
    SELECT uin FROM Online_Users \
 )"

   PQclear( PQexec(users_dbconn, "BEGIN" ));
   if (online_only != True)
   {
      /* if depart = 0 we'll send msg 2 all */
      if (department != 0)
      {
         /* we should get uins for all unconnected */
	 /* users from specified department */
         slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
	          BCST_MDO576, department);
      }
      else
      {
         /* we should get uins for all unconnected users */
         slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
	          BCST_MAO586);
      }

      res = PQexec(users_dbconn, dbcomm_str);
      if (PQresultStatus(res) != PGRES_TUPLES_OK)
      {
         handle_database_error(res, "[BROADCAST-MESS2OFFLINE]");
         return;
      }

      users_cnt = PQntuples(res); /* number of found users */

      PGresult *res1;		  /* second struct for queries */
      bigstring nmessage;	  /* for converted message */
      time_t stime, mtime;	  /* for time values */
      bigstring dbcomm_str;	  /* for query body */

      /* translate to server and remove all system chars */
      convert_to_postgres(nmessage, sizeof(nmessage)-1 ,message);
      stime = time(NULL); mtime = mktime(gmtime(&stime));

      /* now it is time to add offline message to all users in loop */
      for( long i=0; i<users_cnt; i++ )
      {
         to_user.uin = atoul( PQgetvalue(res, i, 0));
         slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
                  "INSERT INTO Users_Messages values (%lu, %lu, %lu, %d, '%s')",
                   to_user.uin, from_uin, mktime(gmtime(&stime)), mess_type, nmessage);

         /* exec select command on backend server */
         res1 = PQexec(users_dbconn, dbcomm_str);
         if (PQresultStatus(res1) != PGRES_COMMAND_OK)
         {
            handle_database_error(res1, "[INSERT BROADCAST-MESS]");
            return;
         }
	 PQclear(res1);
      }
      PQclear( res );
   }

   PQclear( PQexec(users_dbconn, "END" ));
}

