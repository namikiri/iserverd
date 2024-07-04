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
/* This unit implements functions that used to gather fregmented packets  */
/* into big one (it use special database large objects - LO) 		  */
/*                                                                        */
/**************************************************************************/


#include "includes.h"


/**************************************************************************/
/* This func add message to database (if user is not online) 		  */
/**************************************************************************/
int db_defrag_addpart(unsigned long from_uin, int seq, int part_num,
			int part_cnt, int len, char *buffer)
{
  PGresult *res;
  unsigned long oid;
  cstring dbcomm_str;
  int lo_fd;
  int nRet;

  DEBUG(50, ("Incoming fragmented packet... part %d of %d\n",
             part_num, part_cnt));

  PQclear(PQexec(users_dbconn, "BEGIN"));

  /* first of all we should create large object and write data in it */
  oid   = lo_creat(users_dbconn, INV_READ|INV_WRITE);
  lo_fd = lo_open(users_dbconn, oid, INV_READ|INV_WRITE);
  lo_write(users_dbconn, lo_fd, buffer, len);
  lo_close(users_dbconn, lo_fd);

  /* then we create record with created oid int Fragment_Storage tbl */
  slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
       "INSERT INTO Fragment_Storage values (%lu, %d, %d, %d, %d, %lu, %lu)",
            from_uin, seq, part_num, part_cnt, len, oid, time(NULL));

  res = PQexec(users_dbconn, dbcomm_str);
  if (PQresultStatus(res) != PGRES_COMMAND_OK)
  {
     handle_database_error(res, "[INSERT FRAGMENT]");
     nRet = -1;
  }
  else
  if (strcmp(PQcmdTuples(res), "") != 0)
  {
     /* insert completed successfully */
     nRet = 0;
  }
  else
  {
     /* command ok, but insert failed */
     nRet = -1;
  }

     PQclear(res);
     PQclear(PQexec(users_dbconn, "END"));

  return(nRet);
}


/**************************************************************************/
/* This func delete all non-merged parts of the specified user		  */
/**************************************************************************/
int db_defrag_delete(unsigned long from_uin)
{
  PGresult *res;
  unsigned long oid;
  cstring dbcomm_str;
  unsigned short frag_cnt;

  PQclear(PQexec(users_dbconn, "BEGIN"));

  slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
       "SELECT frg FROM Fragment_Storage WHERE uin=%u", from_uin);

  res = PQexec(users_dbconn, dbcomm_str);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
     handle_database_error(res, "[SELECT FRAGMENTS]");
     PQclear(PQexec(users_dbconn, "END"));
     return(-1);
  }

  frag_cnt = PQntuples(res);
  if (frag_cnt > 0)
  {
     char **valid = NULL;

     for (int i=0;i<frag_cnt;i++)
     {
       oid = strtoul(PQgetvalue(res, i, 0), valid, 10);
       lo_unlink(users_dbconn, oid);
       valid = NULL;
     }

     PQclear(res);

     slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
             "DELETE FROM Fragment_Storage WHERE uin=%u", from_uin);

     PQclear(PQexec(users_dbconn, dbcomm_str));
     PQclear(PQexec(users_dbconn, "END"));
     return(0);
  }
  else
  {
     PQclear(res);
     PQclear(PQexec(users_dbconn, "END"));
     return(-1);
  }
}


/**************************************************************************/
/* This func delete all non-merged parts of the specified user		  */
/**************************************************************************/
int db_defrag_delete(unsigned long from_uin, unsigned long seq)
{
  PGresult *res;
  unsigned long oid;
  cstring dbcomm_str;
  unsigned short frag_cnt;

  PQclear(PQexec(users_dbconn, "BEGIN"));

  slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
      "SELECT frg FROM Fragment_Storage WHERE uin=%u AND seq=%d",
       from_uin, seq);

  res = PQexec(users_dbconn, dbcomm_str);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
     handle_database_error(res, "[SELECT FRAGMENTS (for delete)]");
     PQclear(PQexec(users_dbconn, "END"));
     return(-1);
  }

  frag_cnt = PQntuples(res);
  if (frag_cnt > 0)
  {
     char **valid = NULL;

     for (int i=0;i<frag_cnt;i++)
     {
       oid = strtoul(PQgetvalue(res, i, 0), valid, 10);
       DEBUG(100, ("DELETE LARGE OBJECT with OID=%lu\n", oid));
       lo_unlink(users_dbconn, oid);
       valid = NULL;
     }

     PQclear(res);

     slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
             "DELETE FROM Fragment_Storage WHERE uin=%u", from_uin);

     res = PQexec(users_dbconn, dbcomm_str);

     PQclear(res);
     PQclear(PQexec(users_dbconn, "END"));
     return(0);
  }
  else
  {
     PQclear(res);
     PQclear(PQexec(users_dbconn, "END"));
     return(-1);
  }
}


/**************************************************************************/
/* This func check parts with seq if we have them all and return result	  */
/* message lenght (or -1 if there is no one or more parts of message )    */
/**************************************************************************/
int db_defrag_check(unsigned long from_uin, unsigned short seq)
{
  PGresult *res;
  fstring dbcomm_str;
  unsigned short frag_cnt, curr_size = 0, total_frags;
  int pack_lenght = 0, i, lo_fd;
  struct online_user user;
  BOOL all_fragments_exist = True;

  /* begin transaction */
  PQclear(PQexec(users_dbconn, "BEGIN"));

  /* we need all exept uin and seq */
  slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
       "SELECT frg, prt_num, prt_cnt, len FROM Fragment_Storage WHERE uin=%lu AND seq=%d ORDER BY prt_num",
	from_uin, seq);

  res = PQexec(users_dbconn, dbcomm_str);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
     handle_database_error(res, "[SELECT FRAG FOR ASSEMBLE]");
     return(-1);
  }

  frag_cnt = PQntuples(res);

  /* let's check if we have enought parts to defragment packet */
  if (frag_cnt != 0)
  {
     total_frags = atol(PQgetvalue(res, 0, 2));

     if (frag_cnt != total_frags)
     {
        PQclear(res);
        PQclear(PQexec(users_dbconn, "END"));
        return(False);
     }
  }
  else
  {
      PQclear(res);
      PQclear(PQexec(users_dbconn, "END"));
      return(False);
  }

  DEBUG(100, ("Returned %d records from fragment_storage (total=%d)...\n",
               frag_cnt, total_frags));

  /* check if we have all packet fragments */
  for (i=0;i<frag_cnt;i++)
  {
    curr_size = fragment_exist(res, frag_cnt, i);
    if (curr_size == 0)
    {
	all_fragments_exist = False;
	break;
    }
    else
    {
        pack_lenght += curr_size;
    }
  }

  /* now if we have all fragments so we can merge them */
  if (all_fragments_exist)
  {
     /* check each packet and sum fragments len */
     DEBUG(100, ("We have defragmented packet with lenght = %d\n", pack_lenght));

     /* Sanity check :) */
     if (pack_lenght > MAX_PACKET_SIZE)
     {
        /* alarm to console */
        LOG_ALARM(0, ("WARNING: User %lu sent in fragments too big packet (%d bytes)\n",
		       from_uin, pack_lenght));
	LOG_ALARM(0, ("Making dump of first 50 packet bytes to debug log file...\n"));

	Packet pack;
	/* reading packet from large object */
	lo_fd = lo_open(users_dbconn, atol(PQgetvalue(res, 0, 0)), INV_READ);
	pack.sizeVal = lo_read(users_dbconn, lo_fd, pack.buff, 50);
	lo_close(users_dbconn, lo_fd);

	/* dump it to log file */
	log_alarm_packet(0, pack);
	PQclear(res);

	PQclear(PQexec(users_dbconn, "END"));
	/* delete packet fragments from database */
	db_defrag_delete(from_uin, seq);
	return(0);
     }

     /* Now we have all parts we need and result packet not too big */
     /* We should merge all parts and send result packet to pipe */
     if (db_online_lookup(from_uin, user) == 0)
     {
        Packet inject_pack;
        inject_pack.from_ip   = user.ip;
	inject_pack.from_port = user.udp_port;
	inject_pack.sizeVal   = pack_lenght;
	inject_pack.reset();

	for (i=0; i<frag_cnt; i++)
	{
	  lo_fd = lo_open(users_dbconn, atoul(PQgetvalue(res, i, 0)), INV_READ);
	  inject_pack.nextData += lo_read(users_dbconn, lo_fd,
	                                  inject_pack.nextData,
	                                  atoul(PQgetvalue(res, i, 3)));
	  lo_close(users_dbconn, lo_fd);
	}

        pipe_send_packet(inject_pack);
     }

     PQclear(res);
     PQclear(PQexec(users_dbconn, "END"));
     db_defrag_delete(from_uin, seq);
     return(True);
  }
  else
  {
     PQclear(res);
     PQclear(PQexec(users_dbconn, "END"));
     return(False);
  }
}


/**************************************************************************/
/* This func check if given fragment number exist in returned tuples 	  */
/**************************************************************************/
int fragment_exist(PGresult *res, unsigned short tuples_num,
		   unsigned short frag_number)
{
  for(int i=0; i<tuples_num; i++)
  {
    if (atol(PQgetvalue(res, i, 1)) == frag_number)
	return atol(PQgetvalue(res, i, 3));
  }

  return(0);
}


/**************************************************************************/
/* This func check expired packet fragments and delete them 		  */
/**************************************************************************/
int db_defrag_check()
{
  PGresult *res;
  unsigned long frag_cnt;
  fstring dbcomm_str;

  /* begin transaction */
  PQclear(PQexec(users_dbconn, "BEGIN"));

  /* we need all exept uin and seq */
  slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
       "SELECT frg, prt_num, prt_cnt, len FROM Fragment_Storage WHERE %lu - frgtime > %d",
        time(NULL), lp_v3_pingtime()*10);

  res = PQexec(users_dbconn, dbcomm_str);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
     handle_database_error(res, "[TEST EXPIRED FRAGS]");
     PQclear(PQexec(users_dbconn, "END"));
     return(-1);
  }

  frag_cnt = PQntuples(res);

  if (frag_cnt > 0)
  {
     LOG_SYS(0, ("Fragment_storage cleanup has detected %lu expired records\n",
                  frag_cnt));
  }

  PQclear(res);
  PQclear(PQexec(users_dbconn, "END"));
  db_defrag_delete();

  return (0);
}


/**************************************************************************/
/* This func delete all expired packet fragments from fragment_storage	  */
/**************************************************************************/
int db_defrag_delete()
{
  PGresult *res;
  unsigned long oid;
  cstring dbcomm_str;
  unsigned short frag_cnt;

  PQclear(PQexec(users_dbconn, "BEGIN"));

  slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
      "SELECT frg FROM Fragment_Storage WHERE %lu - frgtime > %d",
       time(NULL), lp_v3_pingtime()*10);

  res = PQexec(users_dbconn, dbcomm_str);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
     handle_database_error(res, "[DELETE EXPIRED FRAGS]");
     PQclear(PQexec(users_dbconn, "END"));
     return(-1);
  }

  frag_cnt = PQntuples(res);
  if (frag_cnt > 0)
  {
     char **valid = NULL;

     for (int i=0;i<frag_cnt;i++)
     {
       oid = strtoul(PQgetvalue(res, i, 0), valid, 10);
       DEBUG(100, ("DELETE LARGE OBJECT with OID=%lu\n", oid));
       lo_unlink(users_dbconn, oid);
       valid = NULL;
     }

     PQclear(res);

     slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
             "DELETE FROM Fragment_Storage WHERE %lu-frgtime > %d",
	     time(NULL), lp_v3_pingtime()*10);

     res = PQexec(users_dbconn, dbcomm_str);

     PQclear(res);
     PQclear(PQexec(users_dbconn, "END"));
     return(0);
  }
  else
  {
     PQclear(res);
     PQclear(PQexec(users_dbconn, "END"));
     return(-1);
  }
}

