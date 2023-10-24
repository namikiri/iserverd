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
/* This unit implemets V3 search future 				  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#include "v3_defines.h"


/**************************************************************************/
/* Make string with sql statement to search by uin	    		  */
/**************************************************************************/
void v3_make_sql_search_by_uin(char *dbcomm_str, char *search_str, int smode)
{
   int suin = atoul(search_str);

   DEBUG(100, ("Processing search command (search by uin)\n"));
   
   slprintf(dbcomm_str, 1023, 
           "SELECT uin,nick,frst,last,email2,auth FROM Users_info WHERE uin=%u", 
	    suin);
}


/**************************************************************************/
/* Make string with sql statement to search by wcountry	    		  */
/**************************************************************************/
void v3_make_sql_search_by_numstr(char *dbcomm_str, char *fieldname,
				    char *search_str, int smode)
{
   DEBUG(100, ("Processing search command (search by wcountry)\n"));
   slprintf(dbcomm_str, 1023, 
           "SELECT uin,nick,frst,last,email2,auth FROM Users_info WHERE %s=%s", 
	    fieldname, search_str);
}


/**************************************************************************/
/* Make string with sql statement to search by specified field 		  */
/**************************************************************************/
void v3_make_sql_search_by_str(char *dbcomm_str, char *fieldname, 
			       char *search_str, int smode)
{
   fstring sstring;
   
   convert_to_postgres(sstring, sizeof(sstring)-1, search_str);

   DEBUG(100, ("Processing search command (search by %s), str: %s\n", 
               fieldname, sstring));

   switch (smode)  
   {
     case MODE_IS:
            slprintf(dbcomm_str, 1023, 
	   "SELECT uin,nick,frst,last,email2,auth FROM Users_info WHERE upper(%s) like upper('%s') LIMIT %d", 
 	    fieldname, sstring, lp_v3_maxsearch()); break;
     case MODE_ISNOT:
            slprintf(dbcomm_str, 1023, 
	   "SELECT uin,nick,frst,last,email2,auth FROM Users_info WHERE upper(%s) not like upper('%s') LIMIT %d", 
 	    fieldname, sstring, lp_v3_maxsearch()); break;
     case MODE_CONTAIN:
            slprintf(dbcomm_str, 1023, 
	   "SELECT uin,nick,frst,last,email2,auth FROM Users_info WHERE upper(%s) like upper('%%%s%%') LIMIT %d", 
 	    fieldname, sstring, lp_v3_maxsearch()); break;
     case MODE_NOTCONTAIN:
            slprintf(dbcomm_str, 1023, 
	   "SELECT uin,nick,frst,last,email2,auth FROM Users_info WHERE upper(%s) not like upper('%%%s%%') LIMIT %d", 
 	    fieldname, sstring, lp_v3_maxsearch()); break;
     case MODE_BEGINS:
            slprintf(dbcomm_str, 1023, 
	   "SELECT uin,nick,frst,last,email2,auth FROM Users_info WHERE upper(%s) not like upper('%s%%') LIMIT %d", 
 	    fieldname, sstring, lp_v3_maxsearch()); break;
     case MODE_ENDS:
            slprintf(dbcomm_str, 1023, 
	   "SELECT uin,nick,frst,last, email2,auth FROM Users_info WHERE upper(%s) not like upper('%%%s') LIMIT %d", 
 	    fieldname, sstring, lp_v3_maxsearch()); break;
   }
}


/**************************************************************************/
/* Start search with specified rules			    		  */
/**************************************************************************/
void v3_process_search()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned short str_len, i, fsend;
   unsigned long  uin_num, temp_stamp;
   char t, search_type, comp_mode;
   struct online_user user;
   struct found_info fuser;
   cstring dbcomm_str;
   fstring search_string;
   PGresult *res;

   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num;
 
   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
          (user.udp_port == int_pack.from_port))
      {
         v3_send_ack(int_pack);
      
         int_pack.reset(); search_type = 0; comp_mode = 0;
         int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num >> temp_stamp;
         int_pack >> t >> search_type;
         int_pack >> t >> comp_mode;

         strncpy(dbcomm_str, "", 2);

         if ((search_type != BY_WCOUNTRY) & (search_type != BY_HCOUNTRY) &
             (search_type != BY_WDEPART) )
         {	  
            /* extract search string from packet */
            int_pack >> str_len;
            if (!islen_valid(str_len, 64, "search string", user))
            {
               move_user_offline(user.uin);
   	       v3_send_not_connected(int_pack);
   	       return;
   	 
            }
            else
            {
               for(i=0; i<str_len; i++) int_pack >> search_string[i];
            }
         }
         else
         {
            int_pack >> str_len;
            snprintf(search_string, sizeof(search_string)-1, "%d", str_len);
         }

         DEBUG(100, ("Search command from user %lu (search_str: \"%s\", type: %d, mode: %d)\n", 
	             uin_num, search_string, search_type, comp_mode));


         send_event2ap(papack, ACT_SEARCH, user.uin, user.status,
	               ipToIcq(user.ip), 3, longToTime(time(NULL)), search_string);
		       
         switch (search_type)
         {
           case BY_UIN:	     v3_make_sql_search_by_uin(dbcomm_str, 
   			     search_string, comp_mode); break;
   	   case BY_NICK:     v3_make_sql_search_by_str(dbcomm_str, "nick",
   			     search_string, comp_mode); break;
   	   case BY_FIRST:    v3_make_sql_search_by_str(dbcomm_str, "frst",
   			     search_string, comp_mode); break;
   	   case BY_LAST:     v3_make_sql_search_by_str(dbcomm_str, "last",
   			     search_string, comp_mode); break;
   	   case BY_EMAIL:    v3_make_sql_search_by_str(dbcomm_str, "email2",
   			     search_string, comp_mode); break;
   	   case BY_AGE:	     v3_make_sql_search_by_numstr(dbcomm_str, "age",
   			     search_string, comp_mode); break; 
   	   case BY_WCITY:    v3_make_sql_search_by_str(dbcomm_str, "wcity",
   			     search_string, comp_mode); break;
   	   case BY_WSTATE:   v3_make_sql_search_by_str(dbcomm_str, "wstate",
   			     search_string, comp_mode); break;
   	   case BY_WCOUNTRY: v3_make_sql_search_by_numstr(dbcomm_str, "wcountry",
   			     search_string, comp_mode); break;
   	   case BY_WCOMPANY: v3_make_sql_search_by_str(dbcomm_str, "wcompany",
   			     search_string, comp_mode); break;
   	   case BY_WTITLE:   v3_make_sql_search_by_str(dbcomm_str, "wtitle",
   			     search_string, comp_mode); break;
   	   case BY_WDEPART:  v3_make_sql_search_by_numstr(dbcomm_str, "wdepart",
   			     search_string, comp_mode); break;
   	   case BY_HCITY:    v3_make_sql_search_by_str(dbcomm_str, "hcity",
   			     search_string, comp_mode); break;
   	   case BY_HSTATE:   v3_make_sql_search_by_str(dbcomm_str, "hstate",
   			     search_string, comp_mode); break;
   	   case BY_HCOUNTRY: v3_make_sql_search_by_numstr(dbcomm_str, "hcountry",
   			     search_string, comp_mode); break;
        }
        
        if (strequal(dbcomm_str, ""))
        {
          strncpy(fuser.nick,   "this ", 10);  
  	  strncpy(fuser.first,  "part not", 32);
  	  strncpy(fuser.email2, "implemented", 32);
          strncpy(fuser.last,   "yet", 10);    
  	
           fuser.uin = 1000000; fuser.auth = 0;
           db_online_lookup(uin_num, user);
  	   db_online_sseq_open(user);
   	   v3_send_found_info(int_pack, user, fuser);
           v3_send_search_finished(int_pack, user, False);
  	   db_online_sseq_close(user, 2);
  	
  	   return;
        }

        res = PQexec(users_dbconn, dbcomm_str);
        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
           handle_database_error(res, "[V3 SEARCH ROUTINE]");
  	   db_online_lookup(uin_num, user);
  	   db_online_sseq_open(user);
   	   v3_send_search_finished(int_pack, user, False);
  	   db_online_sseq_close(user, 1);
           return;
        }

        fsend = PQntuples(res);
        db_online_sseq_open(user);
      
        for(i=0; i<fsend;i++)
        {
  	   if (fsend == 0) break;
        
           strncpy(fuser.nick,   PQgetvalue(res, i,  1), sizeof(fuser.nick)-1);
           strncpy(fuser.first,  PQgetvalue(res, i,  2), sizeof(fuser.first)-1);
           strncpy(fuser.last,   PQgetvalue(res, i,  3), sizeof(fuser.last)-1);    
           strncpy(fuser.email2, PQgetvalue(res, i,  4), sizeof(fuser.email2)-1);
           ITrans.translateToClient(fuser.nick);
           ITrans.translateToClient(fuser.first);
           ITrans.translateToClient(fuser.last);
           ITrans.translateToClient(fuser.email2);
  	 
           fuser.uin  = atoul(PQgetvalue(res, i,  0));
           fuser.auth = atol(PQgetvalue(res, i,  5));

           v3_send_found_info(int_pack, user, fuser);
	   
	   /* This is some delay do deliver all data */
	   if (((i % 50) == 0) & (i != 0)) results_delay(200);
        }
      
        PQclear(res);
    
        if (fsend > lp_v3_maxsearch()) 
        {
           v3_send_search_finished(int_pack, user, True);
        }
        else
        {
           v3_send_search_finished(int_pack, user, False);
        }

        db_online_sseq_close(user, fsend+1);
     }
     else
     {
    
        LOG_ALARM(0, ("Spoofed search request from %s:%d (%lu)\n", 
        inet_ntoa(int_pack.from_ip), int_pack.from_port, uin_num));
     }
  }
  else
  {
 
     v3_send_not_connected(int_pack);

  }
}

