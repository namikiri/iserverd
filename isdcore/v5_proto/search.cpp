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
/* META and old style search processing					  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#include "v5_defines.h"


/**************************************************************************/
/* This used to send reply packets on META_SEARCH_WHITE_PAGES             */
/**************************************************************************/
void v5_search_by_white(Packet &pack, struct online_user &user, 
		        unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short i, sub_cmd;
   unsigned long  uin_num, temp_stamp;
   struct full_user_info fuser;
   char dbcomm_str[4096];
   int fsend, keys_cnt;
   char *keys_ptr;
   fstring clause_temp, pkeys, interests_dir, affkeys, token;
   ffstring nick_str, first_str, last_str, email, city, state, company;
   ffstring department, position;
   unsigned short min_age, max_age, country, pcode;
   unsigned short int_index, aff_index;
   char gender, language, work_code, online_only;
   PGresult *res;
   BOOL is_first_clause, not_implemented, at_least_one_key;

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;


   send_event2ap(papack, ACT_SEARCH, user.uin, user.status,                
                 ipToIcq(user.ip), 51, longToTime(time(NULL)), "");
		 
   /* first block - personal information clauses */
   v5_extract_string(first_str, int_pack, 32, "first name", user);
   convert_to_postgres(first_str, sizeof(first_str));
   v5_extract_string(last_str,  int_pack, 32, "last name", user);
   convert_to_postgres(last_str, sizeof(last_str));
   v5_extract_string(nick_str,  int_pack, 32, "nick name", user);
   convert_to_postgres(nick_str, sizeof(nick_str));  
   v5_extract_string(email,  int_pack, 63, "email", user);
   convert_to_postgres(email, sizeof(email));
   
   /* Second block - age, gender, language information */
   int_pack >> min_age >> max_age;
   int_pack >> gender  >> language;
   
   /* Third block - location information */
   v5_extract_string(city,  int_pack, 32, "city", user);
   convert_to_postgres(city, sizeof(city));
   v5_extract_string(state,  int_pack, 32, "state", user);
   convert_to_postgres(state, sizeof(state));
   int_pack >> country;
   
   /* fourth block - work position information */
   v5_extract_string(company,  int_pack, 32, "company name", user);
   convert_to_postgres(company, sizeof(company));
   v5_extract_string(department,  int_pack, 32, "department", user);
   convert_to_postgres(department, sizeof(department));   
   v5_extract_string(position,  int_pack, 32, "work position", user);
   convert_to_postgres(position, sizeof(position));
   int_pack >> work_code;

   /* fifth block - past information */
   int_pack >> pcode;
   v5_extract_string(pkeys,  int_pack, 127, "past keywords", user);
   convert_to_postgres(pkeys, sizeof(pkeys));

   /* sixth block - interests information */   
   int_pack >> int_index;
   v5_extract_string(interests_dir,  int_pack, 127, "interests dir name", user);
   convert_to_postgres(interests_dir, sizeof(interests_dir));
   
   /* seventh block - affilations information */
   int_pack >> aff_index;
   v5_extract_string(affkeys,  int_pack, 127, "affilations keywords", user);
   convert_to_postgres(affkeys, sizeof(affkeys));

   int_pack >> online_only;

   /* now all information is extracted from packet and we can perform 
      white_pages search in database. There will be very complex SQL query 
   */
    
   /* first query part - constant */
   is_first_clause = True;
   not_implemented = True;
   
   snprintf(dbcomm_str, 4096, 
           "SELECT uin,nick,frst,last,email2,auth FROM Users_Info_Ext WHERE ");
   
   if (strlen(nick_str) != 0) 
   {
      snprintf(clause_temp, 255, "(upper(nick) like upper('%%%s%%')) ", nick_str);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if (strlen(first_str) != 0) 
   {
      snprintf(clause_temp, 255, "(upper(frst) like upper('%%%s%%')) ", first_str);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if (strlen(last_str) != 0) 
   {
      snprintf(clause_temp, 255, "(upper(last) like upper('%%%s%%')) ", last_str);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if (strlen(email) != 0) 
   {
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      snprintf(clause_temp, 255, "((upper(email2) like upper('%%%s%%')) ", email);
      safe_strcat(dbcomm_str, clause_temp, 4096);
      snprintf(clause_temp, 255, "OR (upper(email3) like upper('%%%s%%'))) ", email);
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if ((min_age != 0) && (max_age != 0) && (min_age < 20000) && (max_age < 20000)) 
   {
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      snprintf(clause_temp, 255, "((age >= %d) AND (age <= %d)) ", min_age, max_age);
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if ((gender < 16) && (gender > 0))
   {
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      snprintf(clause_temp, 255, "(sex = %d) ", gender);
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if ((language > 0) && (language < 127))
   {
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      snprintf(clause_temp, 255, "(%d in (lang1, lang2, lang3)) ", 
               language);
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if (strlen(city) != 0) 
   {
      snprintf(clause_temp, 255, "(upper(hcity) like upper('%%%s%%')) ", city);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if (strlen(state) != 0) 
   {
      snprintf(clause_temp, 255, "((upper(hstate) like upper('%%%s%%')) OR (upper(wstate) like upper('%%%s%%'))) ", state, state);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if ((country > 0) && (country < 20000)) 
   {
      snprintf(clause_temp, 255, "((hcountry=%d) OR (wcountry=%d)) ", country, country);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if (strlen(company) != 0) 
   {
      snprintf(clause_temp, 255, "(upper(wcompany) like upper('%%%s%%')) ", company);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if (strlen(position) != 0) 
   {
      snprintf(clause_temp, 255, "(upper(wtitle) like upper('%%%s%%')) ", position);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if ((work_code > 0) && (work_code < 127))
   {
      snprintf(clause_temp, 255, "(wocup = %d) ", work_code);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if ((pcode > 0) && (pcode < 60000))
   {
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      snprintf(clause_temp, 255, "(((past_ind1 = %d) AND (", pcode);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)pkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(past_key1) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }

      safe_strcat(dbcomm_str, "OR ", 4096);
      snprintf(clause_temp, 255, "((past_ind2 = %d) AND (", pcode);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)pkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(past_key2) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }

      safe_strcat(dbcomm_str, "OR ", 4096);
      snprintf(clause_temp, 255, "((past_ind3 = %d) AND (", pcode);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)pkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(past_key3) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False ))", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True ))", 4096);
      }

      safe_strcat(dbcomm_str, ")", 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if ((aff_index > 0) && (aff_index < 60000))
   {
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      snprintf(clause_temp, 255, "(((aff_ind1 = %d) AND (", aff_index);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)affkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(aff_key1) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }

      safe_strcat(dbcomm_str, "OR ", 4096);
      snprintf(clause_temp, 255, "((aff_ind2 = %d) AND (", aff_index);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)affkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(aff_key2) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }

      safe_strcat(dbcomm_str, "OR ", 4096);
      snprintf(clause_temp, 255, "((aff_ind3 = %d) AND (", aff_index);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)affkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(aff_key3) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }

      safe_strcat(dbcomm_str, ")", 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if ((int_index > 0) && (int_index < 60000))
   {
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      snprintf(clause_temp, 255, "(((int_ind1 = %d) AND (", int_index);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)interests_dir;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(int_key1) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }

      safe_strcat(dbcomm_str, "OR ", 4096);
      snprintf(clause_temp, 255, "((int_ind2 = %d) AND (", int_index);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)interests_dir;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(int_key2) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }

      safe_strcat(dbcomm_str, "OR ", 4096);
      snprintf(clause_temp, 255, "((int_ind3 = %d) AND (", int_index);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)interests_dir;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(int_key3) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }

      safe_strcat(dbcomm_str, "OR ", 4096);
      snprintf(clause_temp, 255, "((int_ind4 = %d) AND (", int_index);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)interests_dir;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(int_key4) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }

      safe_strcat(dbcomm_str, ")", 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   
   if (online_only) 
   {
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, "(uin IN (SELECT uin FROM online_users))", 4096);
   }
  
   if (not_implemented)
   {
       strncpy(fuser.nick,   "this ", 10);  
       strncpy(fuser.first,  "part not", 32);
       strncpy(fuser.email2, "implemented", 32);
       strncpy(fuser.last,   "yet", 10);    
	
       fuser.uin = 1000000; fuser.auth = 0;

       db_online_sseq_open(user);
       v5_send_white_user_found(seq2, user, fuser, True, True, 0);
       db_online_sseq_close(user, 1);
       return;
   }
  
   /* now query is ready - we can run it */
   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[WHITE PAGES SEARCH]");
      db_online_lookup(uin_num, user);
      db_online_sseq_open(user);
      v5_send_white_user_found(seq2, user, fuser, True, False, 0);
      db_online_sseq_close(user, 1);
      return;
    }

    fsend = PQntuples(res);
    
    DEBUG(10, ("Found %d users\n", fsend));
    
    db_online_sseq_open(user);


    if (fsend == 0) 
    {
       v5_send_white_user_found(seq2, user, fuser, True, False, 0);
    }
    
    for(i=0; i<fsend; i++)
    {      
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

       if ((i == (fsend - 1)) | (i == (lp_v5_maxsearch()-1))) 
       {
          if (fsend > lp_v5_maxsearch())
	  {
             v5_send_white_user_found(seq2, user, fuser, True, True, 1);
	  }
	  else
	  {
	     v5_send_white_user_found(seq2, user, fuser, True, True, 0);
	  }
       }
       else
       {
          v5_send_white_user_found(seq2, user, fuser, False, True, 0);
       }
       
       if (i == (lp_v5_maxsearch()-1)) break;       
       if (((i % 25) == 0) & (i != 0)) results_delay(200);
    }
      
    PQclear(res);

    if (fsend == 0)
    {
       db_online_sseq_close(user,1);
    }
    else
    {
       db_online_sseq_close(user,i);
    }
}


/**************************************************************************/
/* This used to send reply packets on META_SEARCH_WHITE_PAGES2            */
/**************************************************************************/
void v5_search_by_white2(Packet &pack, struct online_user &user, 
		        unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short i, sub_cmd;
   unsigned long  uin_num, temp_stamp;
   struct full_user_info fuser;
   char dbcomm_str[4096];
   int fsend, keys_cnt;
   char *keys_ptr;
   fstring clause_temp, pkeys, interests_dir, affkeys, pagekeys, token;
   ffstring nick_str, first_str, last_str, email, city, state, company;
   ffstring department, position;
   unsigned short min_age, max_age, country, pcode;
   unsigned short int_index, aff_index, page_index;
   char gender, language, work_code, online_only;
   PGresult *res;
   BOOL is_first_clause, not_implemented, at_least_one_key;

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;

   send_event2ap(papack, ACT_SEARCH, user.uin, user.status,                
                 ipToIcq(user.ip), 52, longToTime(time(NULL)), "");

   /* first block - personal information clauses */
   v5_extract_string(first_str, int_pack, 32, "first name", user);
   convert_to_postgres(first_str, sizeof(first_str));
   v5_extract_string(last_str,  int_pack, 32, "last name", user);
   convert_to_postgres(last_str, sizeof(last_str));
   v5_extract_string(nick_str,  int_pack, 32, "nick name", user);
   convert_to_postgres(nick_str, sizeof(nick_str));  
   v5_extract_string(email,  int_pack, 63, "email", user);
   convert_to_postgres(email, sizeof(email));
   
   /* Second block - age, gender, language information */
   int_pack >> min_age >> max_age;
   int_pack >> gender  >> language;
   
   /* Third block - location information */
   v5_extract_string(city,  int_pack, 32, "city", user);
   convert_to_postgres(city, sizeof(city));
   v5_extract_string(state,  int_pack, 32, "state", user);
   convert_to_postgres(state, sizeof(state));
   int_pack >> country;
   
   /* fourth block - work position information */
   v5_extract_string(company,  int_pack, 32, "company name", user);
   convert_to_postgres(company, sizeof(company));
   v5_extract_string(department,  int_pack, 32, "department", user);
   convert_to_postgres(department, sizeof(department));   
   v5_extract_string(position,  int_pack, 32, "work position", user);
   convert_to_postgres(position, sizeof(position));
   int_pack >> work_code;

   /* fifth block - past information */
   int_pack >> pcode;
   v5_extract_string(pkeys,  int_pack, 63, "past keywords", user);
   convert_to_postgres(pkeys, sizeof(pkeys));

   /* sixth block - interests information */   
   int_pack >> int_index;
   v5_extract_string(interests_dir,  int_pack, 127, "interests dir name", user);
   convert_to_postgres(interests_dir, sizeof(interests_dir));
   
   /* seventh block - affilations information */
   int_pack >> aff_index;
   v5_extract_string(affkeys,  int_pack, 127, "affilations keywords", user);
   convert_to_postgres(affkeys, sizeof(affkeys));

   int_pack >> page_index;
   v5_extract_string(pagekeys,  int_pack, 127, "page keywords", user);
   convert_to_postgres(pagekeys, sizeof(pagekeys));

   int_pack >> online_only;

   /* now all information is extracted from packet and we can preform 
      white_pages search in database. There will be very complex SQL query 
   */

   /* first query part - constant */
   is_first_clause = True;
   not_implemented = True;
   
   snprintf(dbcomm_str, 4096, 
            "SELECT uin,nick,frst,last,email2,auth FROM Users_Info_Ext WHERE ");
   
   if (strlen(nick_str) != 0) 
   {
      snprintf(clause_temp, 255, "(upper(nick) like upper('%%%s%%')) ", nick_str);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if (strlen(first_str) != 0) 
   {
      snprintf(clause_temp, 255, "(upper(frst) like upper('%%%s%%')) ", first_str);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if (strlen(last_str) != 0) 
   {
      snprintf(clause_temp, 255, "(upper(last) like upper('%%%s%%')) ", last_str);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if (strlen(email) != 0) 
   {
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      snprintf(clause_temp, 255, "((upper(email2) like upper('%%%s%%')) ", email);
      safe_strcat(dbcomm_str, clause_temp, 4096);
      snprintf(clause_temp, 255, "OR (upper(email3) like upper('%%%s%%'))) ", email);
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if ((min_age != 0) && (max_age != 0) && (min_age < 20000) && (max_age < 20000)) 
   {
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      snprintf(clause_temp, 255, "((age >= %d) AND (age <= %d)) ", min_age, max_age);
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if ((gender < 16) && (gender > 0))
   {
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      snprintf(clause_temp, 255, "(sex = %d) ", gender);
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if ((language > 0) && (language < 127))
   {
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      snprintf(clause_temp, 255, "(%d in (lang1, lang2, lang3)) ", 
               language);
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if (strlen(city) != 0) 
   {
      snprintf(clause_temp, 255, "(upper(hcity) like upper('%%%s%%')) ", city);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if (strlen(state) != 0) 
   {
      snprintf(clause_temp, 255, "((upper(hstate) like upper('%%%s%%')) OR (upper(wstate) like upper('%%%s%%'))) ", state, state);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if ((country > 0) && (country < 20000)) 
   {
      snprintf(clause_temp, 255, "((hcountry=%d) OR (wcountry=%d)) ", country, country);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if (strlen(company) != 0) 
   {
      snprintf(clause_temp, 255, "(upper(wcompany) like upper('%%%s%%')) ", company);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if (strlen(position) != 0) 
   {
      snprintf(clause_temp, 255, "(upper(wtitle) like upper('%%%s%%')) ", position);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if ((work_code > 0) && (work_code < 127))
   {
      snprintf(clause_temp, 255, "(wocup = %d) ", work_code);
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, clause_temp, 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if ((pcode > 0) && (pcode < 60000))
   {
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      snprintf(clause_temp, 255, "(((past_ind1 = %d) AND (", pcode);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)pkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(past_key1) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }

      safe_strcat(dbcomm_str, "OR ", 4096);
      snprintf(clause_temp, 255, "((past_ind2 = %d) AND (", pcode);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)pkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(past_key2) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }

      safe_strcat(dbcomm_str, "OR ", 4096);
      snprintf(clause_temp, 255, "((past_ind3 = %d) AND (", pcode);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)pkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(past_key3) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False ))", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True ))", 4096);
      }

      safe_strcat(dbcomm_str, ")", 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if ((aff_index > 0) && (aff_index < 60000))
   {
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      snprintf(clause_temp, 255, "(((aff_ind1 = %d) AND (", aff_index);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)affkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(aff_key1) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }

      safe_strcat(dbcomm_str, "OR ", 4096);
      snprintf(clause_temp, 255, "((aff_ind2 = %d) AND (", aff_index);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)affkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(aff_key2) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }

      safe_strcat(dbcomm_str, "OR ", 4096);
      snprintf(clause_temp, 255, "((aff_ind3 = %d) AND (", aff_index);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)affkeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(aff_key3) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }


      safe_strcat(dbcomm_str, ")", 4096);
      not_implemented = False;
      is_first_clause = False;
   }

   if ((int_index > 0) && (int_index < 60000))
   {
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      snprintf(clause_temp, 255, "(((int_ind1 = %d) AND (", int_index);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)interests_dir;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(int_key1) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }

      safe_strcat(dbcomm_str, "OR ", 4096);
      snprintf(clause_temp, 255, "((int_ind2 = %d) AND (", int_index);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)interests_dir;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(int_key2) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }

      safe_strcat(dbcomm_str, "OR ", 4096);
      snprintf(clause_temp, 255, "((int_ind3 = %d) AND (", int_index);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)interests_dir;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(int_key3) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }

      safe_strcat(dbcomm_str, "OR ", 4096);
      snprintf(clause_temp, 255, "((int_ind4 = %d) AND (", int_index);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)interests_dir;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(int_key4) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }


      safe_strcat(dbcomm_str, ")", 4096);
      not_implemented = False;
      is_first_clause = False;
   }


   if ((page_index > 0) && (page_index < 60000))
   {
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      snprintf(clause_temp, 255, "(((hpage_cat=%d) AND (", page_index);
      safe_strcat(dbcomm_str, clause_temp, 4096);

      keys_ptr = (char *)pagekeys;
      at_least_one_key = False;
      keys_cnt = 0;
      while (next_token(&keys_ptr,token,",",sizeof(token))) 
      {
        if (strlen(token) > 1) 
	{  
	   snprintf(clause_temp, 255, 
	   "(upper(hpage_txt) like upper('%%%s%%')) OR ", token);
	   safe_strcat(dbcomm_str, clause_temp, 4096);
	   at_least_one_key = True;
	   keys_cnt++;
	   if (keys_cnt > 5) break;
	}
      }

      if (at_least_one_key) 
      {
         safe_strcat(dbcomm_str, "False )) ", 4096);
      }
      else
      {
         safe_strcat(dbcomm_str, "True )) ", 4096);
      }

      safe_strcat(dbcomm_str, ")", 4096);
      not_implemented = False;
      is_first_clause = False;
   }
   
   if (online_only) 
   {
      if (!is_first_clause) { safe_strcat(dbcomm_str, "AND ", 4096); };
      safe_strcat(dbcomm_str, "(uin IN (SELECT uin FROM online_users))", 4096);
   }
  
   if (not_implemented)
   {
       strncpy(fuser.nick,   "this ", 10);  
       strncpy(fuser.first,  "part not", 32);
       strncpy(fuser.email2, "implemented", 32);
       strncpy(fuser.last,   "yet", 10);    
	
       fuser.uin = 1000000; fuser.auth = 0;

       db_online_sseq_open(user);
       v5_send_white_user_found2(seq2, user, fuser, True, True, 0);
       db_online_sseq_close(user, 1);
       return;
   }
  
   /* now query is ready - we can run it */
   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[WHITE PAGES SEARCH #2]");
      db_online_lookup(uin_num, user);
      db_online_sseq_open(user);
      v5_send_white_user_found2(seq2, user, fuser, True, False, 0);
      db_online_sseq_close(user, 1);
      return;
    }

    fsend = PQntuples(res);
    
    DEBUG(10, ("Found %d users\n", fsend));
    
    db_online_sseq_open(user);


    if (fsend == 0) 
    {
       v5_send_white_user_found2(seq2, user, fuser, True, False, 0);
    }
    
    for(i=0; i<fsend; i++)
    {      
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

       if ((i == (fsend - 1)) | (i == (lp_v5_maxsearch()-1))) 
       {
          if (fsend > lp_v5_maxsearch())
	  {
             v5_send_white_user_found2(seq2, user, fuser, True, True, 1);
	  }
	  else
	  {
	     v5_send_white_user_found2(seq2, user, fuser, True, True, 0);
	  }
       }
       else
       {
          v5_send_white_user_found2(seq2, user, fuser, False, True, 0);
       }
       
       if (i == (lp_v5_maxsearch()-1)) break;       
       if (((i % 25) == 0) & (i != 0)) results_delay(200);
    }
      
    PQclear(res);

    if (fsend == 0)
    {
       db_online_sseq_close(user,1);
    }
    else
    {
       db_online_sseq_close(user,i);
    }
}


/**************************************************************************/
/* This used to send reply packet on META_SEARCH_BYUIN                    */
/**************************************************************************/
void v5_search_by_uin(struct online_user &user, unsigned long target_uin, 
		      unsigned short seq2)
{
   struct full_user_info tuser;
   char str_uin[25];
   
   db_online_sseq_open(user);
   
   if (db_users_lookup(target_uin, tuser) >= 0)
   {
      /* we found user info - make send it to user */
      v5_send_user_found(seq2, user, tuser, True, True, 0);
   }
   else
   {
      /* user not found ot database problem */
      v5_send_user_found(seq2, user, tuser, True, False, 0);
   }

   snprintf(str_uin, 24, "%lu", target_uin);   
   send_event2ap(papack, ACT_SEARCH, user.uin, user.status,                
                 ipToIcq(user.ip), 53, longToTime(time(NULL)), str_uin);
   
   db_online_sseq_close(user, 1);
}


/**************************************************************************/
/* This used to send reply packet on OLD_SEARCH_BYUIN                     */
/**************************************************************************/
void v5_old_search_uin(unsigned long tuin, struct online_user &user, 
		       unsigned short seq2)
{
   struct full_user_info tuser;
   char str_uin[25];
   
   db_online_sseq_open(user);
   
   if (db_users_lookup(tuin, tuser) >= 0)
   {
      /* we found user info - build pack & send to user */
      v5_send_old_search_found(seq2, user, tuser);
      v5_send_old_search_end(seq2, user, False);
      db_online_sseq_close(user, 2);
   }
   else
   {
      /* user not found ot database problem */
      v5_send_old_search_end(seq2, user, False);
      db_online_sseq_close(user, 1);
   }

   snprintf(str_uin, 24, "%lu", tuin);   
   send_event2ap(papack, ACT_SEARCH, user.uin, user.status,                
                 ipToIcq(user.ip), 53, longToTime(time(NULL)), str_uin);

}


/**************************************************************************/
/* This used to send reply packet on META_SEARCH_BYUIN                    */
/**************************************************************************/
void v5_search_by_uin2(struct online_user &user, unsigned long target_uin, 
		      unsigned short seq2)
{
   struct full_user_info tuser;
   char str_uin[25];
   
   db_online_sseq_open(user);
   
   if (db_users_lookup(target_uin, tuser) >= 0)
   {
      /* we found user info - make send it to user */
      v5_send_user_found2(seq2, user, tuser, True, True, 0);
   }
   else
   {
      /* user not found ot database problem */
      v5_send_user_found2(seq2, user, tuser, True, False, 0);
   }
   
   db_online_sseq_close(user, 1);

   snprintf(str_uin, 24, "%lu", target_uin);   
   send_event2ap(papack, ACT_SEARCH, user.uin, user.status,                
                 ipToIcq(user.ip), 53, longToTime(time(NULL)), str_uin);

}


/**************************************************************************/
/* This used to send reply packet on META_SEARCH_BY_EMAIL                 */
/**************************************************************************/
void v5_search_by_email(Packet &pack, struct online_user &user,  
		        unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short str_len, i, sub_cmd;
   unsigned long  uin_num, temp_stamp;
   struct full_user_info fuser;
   cstring dbcomm_str;
   int fsend;
   fstring email_str;
   PGresult *res;

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;

   int_pack >> str_len;
   
   if (!islen_valid(str_len, 64, "V5 email search string", user))
   {
      move_user_offline(user.uin);
      v5_send_not_connected(int_pack);
      return;
	 
   } else { for(i=0; i<str_len; i++) int_pack >> email_str[i]; }

   DEBUG(10, ("V5 search by email: %s\n", email_str));
   
   send_event2ap(papack, ACT_SEARCH, user.uin, user.status,                
                 ipToIcq(user.ip), 54, longToTime(time(NULL)), email_str);   
   
   convert_to_postgres(email_str, sizeof(email_str));   

   /* make sql query string */
   snprintf(dbcomm_str, 255, 
          "SELECT uin,nick,frst,last,email2,auth FROM Users_info WHERE email2 like '%%%s%%' LIMIT %d", 
	   email_str, lp_v5_maxsearch()+1);
     
   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[V5 SEARCH BY EMAIL]");      
      db_online_lookup(uin_num, user);
      db_online_sseq_open(user);
      v5_send_user_found(seq2, user, fuser, True, False, 0);
      db_online_sseq_close(user, 1);
      return;
    }

    fsend = PQntuples(res);
    
    DEBUG(10, ("Found %d users\n", fsend));
    
    db_online_sseq_open(user);

    if (fsend == 0) 
    {
       v5_send_user_found(seq2, user, fuser, True, False, 0);
    }

    for(i=0; i<fsend; i++)
    {      
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

       if ((i == (fsend - 1)) | (i == (lp_v5_maxsearch()-1))) 
       {
          if (fsend > lp_v5_maxsearch())
	  {
             v5_send_user_found(seq2, user, fuser, True, True, 1);
	  }
	  else
	  {
	     v5_send_user_found(seq2, user, fuser, True, True, 0);
	  }
       }
       else
       {
          v5_send_user_found(seq2, user, fuser, False, True, 0);
       }
       
       if (i == (lp_v5_maxsearch()-1)) break;       
       if (((i % 25) == 0) & (i != 0)) results_delay(200);
    }
      
    PQclear(res);

    if (fsend == 0)
    {
       db_online_sseq_close(user,1);
    }
    else
    {
       db_online_sseq_close(user,i);
    }
}


/**************************************************************************/
/* This used to send reply packet on META_SEARCH_BYEMAIL                  */
/**************************************************************************/
void v5_search_by_email2(Packet &pack, struct online_user &user,  
		        unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short str_len, i, sub_cmd;
   unsigned long  uin_num, temp_stamp;
   struct full_user_info fuser;
   cstring dbcomm_str;
   int fsend;
   fstring email_str;
   PGresult *res;

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;

   int_pack >> str_len;
   
   if (!islen_valid(str_len, 64, "V5 email search string", user))
   {
      move_user_offline(user.uin);
      v5_send_not_connected(int_pack);
      return;
	 
   } else { for(i=0; i<str_len; i++) int_pack >> email_str[i]; }

   DEBUG(10, ("V5 search by email: %s\n", email_str));

   send_event2ap(papack, ACT_SEARCH, user.uin, user.status,                
                 ipToIcq(user.ip), 54, longToTime(time(NULL)), email_str);   
   
   convert_to_postgres(email_str, sizeof(email_str));   

   /* make sql query string */
   snprintf(dbcomm_str, 255, 
           "SELECT uin,nick,frst,last,email2,auth FROM Users_info WHERE email2 like '%%%s%%' LIMIT %d", 
	    email_str, lp_v5_maxsearch()+1);
     
   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[V5 SEARCH BY EMAIL #2]");
      db_online_lookup(uin_num, user);
      db_online_sseq_open(user);
      v5_send_user_found2(seq2, user, fuser, True, False, 0);
      db_online_sseq_close(user, 1);
      return;
    }

    fsend = PQntuples(res);
    
    DEBUG(10, ("Found %d users\n", fsend));
    
    db_online_sseq_open(user);

    if (fsend == 0) 
    {
       v5_send_user_found2(seq2, user, fuser, True, False, 0);
    }
    
    for(i=0; i<fsend; i++)
    {      
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

       if ((i == (fsend - 1)) | (i == (lp_v5_maxsearch()-1))) 
       {
          if (fsend > lp_v5_maxsearch())
	  {
             v5_send_user_found2(seq2, user, fuser, True, True, 1);
	  }
	  else
	  {
	     v5_send_user_found2(seq2, user, fuser, True, True, 0);
	  }
       }
       else
       {
          v5_send_user_found2(seq2, user, fuser, False, True, 0);
       }
       
       if (i == (lp_v5_maxsearch()-1)) break;
       if (((i % 25) == 0) & (i != 0)) results_delay(200);
    }
      
    PQclear(res);

    if (fsend == 0)
    {
       db_online_sseq_close(user,1);
    }
    else
    {
       db_online_sseq_close(user,i);
    }    
}


/**************************************************************************/
/* This used to send reply packet on META_SEARCH_BYNAME                   */
/**************************************************************************/
void v5_search_by_name(Packet &pack, struct online_user &user,  
		       unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short str_len, i, sub_cmd;
   unsigned long  uin_num, temp_stamp;
   struct full_user_info fuser;
   cstring dbcomm_str;
   int fsend;
   fstring nick_str, first_str, last_str;
   fstring clause1, clause2, clause3;
   PGresult *res;

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;

   int_pack >> str_len;
   
   if (!islen_valid(str_len, 32, "V5 nick search string", user))
   {
      move_user_offline(user.uin);
      v5_send_not_connected(int_pack);
      return;
	 
   } else { for(i=0; i<str_len; i++) int_pack >> first_str[i]; }

   int_pack >> str_len;

   if (!islen_valid(str_len, 32, "V5 firstname search string", user))
   {
      move_user_offline(user.uin);
      v5_send_not_connected(int_pack);
      return;
	 
   } else { for(i=0; i<str_len; i++) int_pack >> last_str[i]; }

   int_pack >> str_len;

   if (!islen_valid(str_len, 32, "V5 lastname search string", user))
   {
      move_user_offline(user.uin);
      v5_send_not_connected(int_pack);
      return;
	 
   } else { for(i=0; i<str_len; i++) int_pack >> nick_str[i]; }

   convert_to_postgres(nick_str, sizeof(nick_str));
   convert_to_postgres(first_str, sizeof(first_str));
   convert_to_postgres(last_str, sizeof(last_str));
   
   /* now we have all data from packet - we should build db query */
   if (strlen(nick_str) != 0) 
   {
      snprintf(clause1, 64, "(upper(nick) like upper('%%%s%%'))", nick_str);
   }
   else
   {
      strncpy(clause1, "", 2);
   }

   if (strlen(first_str) != 0) 
   {
      if (strlen(nick_str) != 0)
      {
         snprintf(clause2, 64, "AND (upper(frst) like upper('%%%s%%'))", 
	                        first_str);
      }
      else
      {
         snprintf(clause2, 64, "(upper(frst) like upper('%%%s%%'))",
			        first_str);
      }
   }
   else
   {
      strncpy(clause2, "", 2);
   }

   if (strlen(last_str) != 0) 
   {
      if ((strlen(nick_str) != 0) | (strlen(first_str) != 0))
      {
         snprintf(clause3, 64, "AND (upper(last) like upper('%%%s%%'))", 
	                        last_str);
      }
      else
      {
         snprintf(clause3, 64, "(upper(last) like upper('%%%s%%'))",
	                        last_str);
      }			
   }
   else
   {
      strncpy(clause3, "", 2);
   }
   

   /* make sql query string */
   snprintf(dbcomm_str, 255, 
           "SELECT uin,nick,frst,last,email2,auth FROM Users_info WHERE %s %s %s LIMIT %d", 
	    clause1, clause2, clause3, lp_v5_maxsearch()+1);

   snprintf(clause1, 250, "%s %s %s", nick_str, first_str, last_str);
   send_event2ap(papack, ACT_SEARCH, user.uin, user.status,                
                 ipToIcq(user.ip), 55, longToTime(time(NULL)), clause1);

     
   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[V5 SEARCH BY NAME]");
      db_online_lookup(uin_num, user);
      db_online_sseq_open(user);
      v5_send_user_found(seq2, user, fuser, True, False, 0);
      db_online_sseq_close(user, 1);
      return;
    }

    fsend = PQntuples(res);
    
    DEBUG(10, ("Found %d users\n", fsend));
    
    db_online_sseq_open(user);

    if (fsend == 0) 
    {
       v5_send_user_found(seq2, user, fuser, True, False, 0);
    }

    for(i=0; i<fsend; i++)
    {
      
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

       if ((i == (fsend - 1)) | (i == (lp_v5_maxsearch()-1))) 
       {
          if (fsend > lp_v5_maxsearch())
	  {
             v5_send_user_found(seq2, user, fuser, True, True, 1);
	  }
	  else
	  {
	     v5_send_user_found(seq2, user, fuser, True, True, 0);
	  }
       }
       else
       {
          v5_send_user_found(seq2, user, fuser, False, True, 0);
       }
       
       if (i == (lp_v5_maxsearch()-1)) break;       
       if (((i % 25) == 0) & (i != 0)) results_delay(200);
    }
      
    PQclear(res);

}


/**************************************************************************/
/* This used to send reply packet on META_SEARCH_BYNAME                   */
/**************************************************************************/
void v5_search_by_name2(Packet &pack, struct online_user &user,  
		       unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short str_len, i, sub_cmd;
   unsigned long  uin_num, temp_stamp;
   struct full_user_info fuser;
   cstring dbcomm_str;
   int fsend;
   fstring nick_str, first_str, last_str;
   fstring clause1, clause2, clause3;
   PGresult *res;

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp
	    >> sub_cmd;

   int_pack >> str_len;
   
   if (!islen_valid(str_len, 32, "V5 nick search string", user))
   {
      move_user_offline(user.uin);
      v5_send_not_connected(int_pack);
      return;
	 
   } else { for(i=0; i<str_len; i++) int_pack >> first_str[i]; }

   int_pack >> str_len;

   if (!islen_valid(str_len, 32, "V5 firstname search string", user))
   {
      move_user_offline(user.uin);
      v5_send_not_connected(int_pack);
      return;
	 
   } else { for(i=0; i<str_len; i++) int_pack >> last_str[i]; }

   int_pack >> str_len;

   if (!islen_valid(str_len, 32, "V5 lastname search string", user))
   {
      move_user_offline(user.uin);
      v5_send_not_connected(int_pack);
      return;
	 
   } else { for(i=0; i<str_len; i++) int_pack >> nick_str[i]; }

   convert_to_postgres(nick_str, sizeof(nick_str));
   convert_to_postgres(first_str, sizeof(first_str));
   convert_to_postgres(last_str, sizeof(last_str));
   
   /* now we have all data from packet - we should build db query */
   if (strlen(nick_str) != 0) 
   {
      snprintf(clause1, 64, "(upper(nick) like upper('%%%s%%'))", nick_str);
   }
   else
   {
      strncpy(clause1, "", 2);
   }

   if (strlen(first_str) != 0) 
   {
      if (strlen(nick_str) != 0)
      {
         snprintf(clause2, 64, "AND (upper(frst) like upper('%%%s%%'))", 
	                        first_str);
      }
      else
      {
         snprintf(clause2, 64, "(upper(frst) like upper('%%%s%%'))",
			        first_str);
      }
   }
   else
   {
      strncpy(clause2, "", 2);
   }

   if (strlen(last_str) != 0) 
   {
      if ((strlen(nick_str) != 0) | (strlen(first_str) != 0))
      {
         snprintf(clause3, 64, "AND (upper(last) like upper('%%%s%%'))", 
	                        last_str);
      }
      else
      {
         snprintf(clause3, 64, "(upper(last) like upper('%%%s%%'))",
	                        last_str);
      }			
   }
   else
   {
      strncpy(clause3, "", 2);
   }
   

   /* make sql query string */
   snprintf(dbcomm_str, 255, 
           "SELECT uin,nick,frst,last,email2,auth FROM Users_info WHERE %s %s %s LIMIT %d", 
	    clause1, clause2, clause3, lp_v5_maxsearch()+1);
     
   snprintf(clause1, 250, "%s %s %s", nick_str, first_str, last_str);
   send_event2ap(papack, ACT_SEARCH, user.uin, user.status,                
                 ipToIcq(user.ip), 55, longToTime(time(NULL)), clause1);
     
   DEBUG(10, ("Query string: %s\n", dbcomm_str)); 
   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[V5 SEARCH BY NAME #2]");
      db_online_lookup(uin_num, user);
      db_online_sseq_open(user);
      v5_send_user_found2(seq2, user, fuser, True, False, 0);
      db_online_sseq_close(user, 1);
      return;
    }

    fsend = PQntuples(res);
    
    DEBUG(10, ("Found %d users\n", fsend));
    
    db_online_sseq_open(user);


    if (fsend == 0) 
    {
       v5_send_user_found2(seq2, user, fuser, True, False, 0);
    }

    
    for(i=0; i<fsend; i++)
    {
      
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

       if ((i == (fsend - 1)) | (i == (lp_v5_maxsearch()-1))) 
       {
          if (fsend > lp_v5_maxsearch())
	  {
             v5_send_user_found2(seq2, user, fuser, True, True, 1);
	  }
	  else
	  {
	     v5_send_user_found2(seq2, user, fuser, True, True, 0);
	  }
       }
       else
       {
          v5_send_user_found2(seq2, user, fuser, False, True, 0);
       }
       
       if (i == (lp_v5_maxsearch()-1)) break;
       if (((i % 25) == 0) & (i != 0)) results_delay(200);
    }
      
    PQclear(res);

    if (fsend == 0)
    {
       db_online_sseq_close(user,1);
    }
    else
    {
       db_online_sseq_close(user,i);
    }    
}


/**************************************************************************/
/* This used to send reply packet on META_SEARCH_BYNAME                   */
/**************************************************************************/
void v5_old_search(Packet &pack, struct online_user &user,  
		       unsigned short seq2)
{
   unsigned short pvers, pcomm, seq1, seq2_2;
   unsigned short str_len, i;
   unsigned long  uin_num, temp_stamp;
   struct full_user_info fuser;
   cstring dbcomm_str;
   int fsend;
   fstring nick_str, first_str, last_str, email_str;
   fstring clause1, clause2, clause3, clause4;
   PGresult *res;

   int_pack.reset(); 
   int_pack >> pvers   >> temp_stamp
            >> uin_num >> temp_stamp 
	    >> pcomm   >> seq1 
	    >> seq2_2  >> temp_stamp;

   int_pack >> str_len;
   
   if (!islen_valid(str_len, 32, "V5 nick search string", user))
   {
      move_user_offline(user.uin);
      v5_send_not_connected(int_pack);
      return;
	 
   } else { for(i=0; i<str_len; i++) int_pack >> nick_str[i]; }

   int_pack >> str_len;

   if (!islen_valid(str_len, 32, "V5 firstname search string", user))
   {
      move_user_offline(user.uin);
      v5_send_not_connected(int_pack);
      return;
	 
   } else { for(i=0; i<str_len; i++) int_pack >> first_str[i]; }

   int_pack >> str_len;

   if (!islen_valid(str_len, 32, "V5 lastname search string", user))
   {
      move_user_offline(user.uin);
      v5_send_not_connected(int_pack);
      return;
	 
   } else { for(i=0; i<str_len; i++) int_pack >> last_str[i]; }

   int_pack >> str_len;

   if (!islen_valid(str_len, 32, "V5 email search string", user))
   {
      move_user_offline(user.uin);
      v5_send_not_connected(int_pack);
      return;
	 
   } else { for(i=0; i<str_len; i++) int_pack >> email_str[i]; }

   convert_to_postgres(nick_str, sizeof(nick_str));
   convert_to_postgres(first_str, sizeof(first_str));
   convert_to_postgres(last_str, sizeof(last_str));
   convert_to_postgres(email_str, sizeof(email_str));
   
   /* now we have all data from packet - we should build db query */
   if (strlen(nick_str) != 0) 
   {
      snprintf(clause1, 64, "(upper(nick) like upper('%%%s%%'))", nick_str);
   }
   else
   {
      strncpy(clause1, "", 2);
   }

   /* check if we should generate clause for this string */
   if (strlen(first_str) != 0) 
   {
      if (strlen(nick_str) != 0)
      {
         snprintf(clause2, 64, "AND (upper(frst) like upper('%%%s%%'))", 
	                        first_str);
      }
      else
      {
         snprintf(clause2, 64, "(upper(frst) like upper('%%%s%%'))",
			        first_str);
      }
   }
   else
   {
      strncpy(clause2, "", 2);
   }

   if (strlen(last_str) != 0) 
   {
      if ((strlen(nick_str) != 0) | (strlen(first_str) != 0))
      {
         snprintf(clause3, 64, "AND (upper(last) like upper('%%%s%%'))", 
	                        last_str);
      }
      else
      {
         snprintf(clause3, 64, "(upper(last) like upper('%%%s%%'))",
	                        last_str);
      }			
   }
   else
   {
      strncpy(clause3, "", 2);
   }
   

   if (strlen(email_str) != 0) 
   {
      if ((strlen(nick_str) != 0) | 
          (strlen(first_str) != 0) | 
          (strlen(last_str) != 0))
      {
         snprintf(clause4, 64, "AND (upper(email2) like upper('%%%s%%'))", 
	                        email_str);
      }
      else
      {
         snprintf(clause4, 64, "(upper(email2) like upper('%%%s%%'))",
	                        email_str);
      }
   }
   else
   {
      strncpy(clause4, "", 2);
   }
   

   /* make sql query string */
   snprintf(dbcomm_str, 255, 
           "SELECT uin,nick,frst,last,email2,auth FROM Users_info WHERE %s %s %s %s LIMIT %d", 
	    clause1, clause2, clause3, clause4, lp_v5_maxsearch()+1);
     
   snprintf(clause1, 250, "%s %s %s %s", nick_str, first_str, last_str, email_str);
   send_event2ap(papack, ACT_SEARCH, user.uin, user.status,                
                 ipToIcq(user.ip), 55, longToTime(time(NULL)), clause1);
     
   DEBUG(10, ("Query string: %s\n", dbcomm_str)); 
   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[V5 OLD SEARCH]");
      db_online_lookup(uin_num, user);
      db_online_sseq_open(user);
      v5_send_old_search_end(seq2, user, False);
      db_online_sseq_close(user, 1);
      return;
    }

    fsend = PQntuples(res);
    
    DEBUG(10, ("Found %d users\n", fsend));
    
    db_online_sseq_open(user);


    if (fsend == 0) 
    {
       v5_send_old_search_end(seq2, user, False);
    }

    
    for(i=0; i<fsend; i++)
    {
      
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

       v5_send_old_search_found(seq2, user, fuser);
       
       if (i == (lp_v5_maxsearch()-1)) break;       
    }
      
    PQclear(res);

    if (fsend == 0)
    {
       db_online_sseq_close(user,1);
    }
    else
    {    
       v5_send_old_search_end(seq2, user, False);
       db_online_sseq_close(user,i+1);
    }    
}

