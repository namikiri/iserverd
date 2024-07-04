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
/* This unit implements users info database calls (lookup, change, add,   */
/* delete, auth, etc) for packet processors				  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"


/**************************************************************************/
/* This function add user record to database			   	  */
/**************************************************************************/
int db_new_add_user(struct full_user_info &user)
{
  PGresult *res;

  typedef char ins_string[10000];
  ins_string insert_str;

#define ADD_UM047 \
"INSERT INTO register_requests VALUES \
 ( \
    '%lu', '%s', '%d', '%lu', '%lu', '%d', '%lu', '%d', '%s', '%s', \
    '%s', '%s', '%s', '%s', '%d', '%d', '%d', '%d', '%d', '%d', \
    '%d', '%d', '%s', '%s', '%s', '%d', '%s', '%s', '%d', '%lu', \
    '%s', '%s', '%s', '%lu', '%s', '%s', '%s', '%s', '%s', '%d', \
    '%s', '%s', '%s', '%lu', '%s', '%lu' \
 )"

  /* now prepare sql command string */
  snprintf(insert_str, 10000, ADD_UM047,
        user.uin, user.passwd, user.disabled, user.lastlogin, user.ip_addr,
	user.can_broadcast, user.cr_date, user.ch_password, user.nick,
	user.first, user.last, user.email1, user.email2, user.email3,
	user.pemail1, user.gmt_offset, user.auth, user.gender, user.age,
	user.bday, user.bmonth, user.byear, user.waddr, user.wcity,
	user.wstate, user.wcountry, user.wcompany, user.wtitle, user.wocup,
	user.wdepart, user.wphone, user.wfax, user.wpager, user.wzip,
	user.wpage, user.notes, user.haddr, user.hcity, user.hstate,
	user.hcountry, user.hphone, user.hfax, user.hcell, user.hzip,
	user.hpage, user.nupdate);

   res = PQexec(users_dbconn, insert_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[INSERT REGISTER_REQUESTS]");
      return(-1);
   }
   else
   {
      PQclear(res);
   }

   return(0);
}


/**************************************************************************/
/* This function add user record to database			   	  */
/**************************************************************************/
int db_users_add_user(struct full_user_info &user)
{
  PGresult *res;

  typedef char ins_string[10000];
  ins_string insert_str;

#define ADD_UA095 \
"INSERT INTO Users_info_Ext VALUES \
 ( \
    '%lu', '%s', '%d', '%lu', '%lu', '%d', '%lu', '%d', '%s', '%s', \
    '%s', '%s', '%s', '%s', '%d', '%d', '%d', '%d', '%d', '%d', \
    '%d', '%d', '%s', '%s', '%s', '%d', '%s', '%s', '%d', '%lu', \
    '%s', '%s', '%s', '%lu', '%s', '%s', '%s', '%s', '%s', '%d', \
    '%s', '%s', '%s', '%lu', '%s', '%lu' \
 )"

  /* now prepare sql command string */
  snprintf(insert_str, 10000, ADD_UA095,
        user.uin, user.passwd, user.disabled, user.lastlogin, user.ip_addr,
	user.can_broadcast, user.cr_date, user.ch_password, user.nick,
	user.first, user.last, user.email1, user.email2, user.email3,
	user.pemail1, user.gmt_offset, user.auth, user.gender, user.age,
	user.bday, user.bmonth, user.byear, user.waddr, user.wcity,
	user.wstate, user.wcountry, user.wcompany, user.wtitle, user.wocup,
	user.wdepart, user.wphone, user.wfax, user.wpager, user.wzip,
	user.wpage, user.notes, user.haddr, user.hcity, user.hstate,
	user.hcountry, user.hphone, user.hfax, user.hcell, user.hzip,
	user.hpage, user.nupdate);

   res = PQexec(users_dbconn, insert_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[INSERT NEW USER]");
      return(-1);
   }
   else
   {
      PQclear(res);
   }

   return(0);
}


/**************************************************************************/
/* This function used to create new register requests table     	  */
/**************************************************************************/
void create_new_users_table()
{
   PGresult *res;
   cstring dbcomm_str;

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT * INTO TABLE register_requests FROM users_info_ext WHERE False;");

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[CREATE REGISTER_REQUESTS]");
      return;
   }

   PQclear(res);
}


/**************************************************************************/
/* This function used generte new uin number 				  */
/**************************************************************************/
BOOL new_users_table_exist()
{
   PGresult *res;
   cstring dbcomm_str;

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "select count(*) FROM pg_tables WHERE tablename = 'register_requests'");

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[CHECK FOR REGISTER_REQUESTS]");
      return(False);
   }

   if (PQnfields(res) != 1)
   {
      LOG_SYS(0, ("Corrypted table structure in db: \"%s\"\n",
                  lp_db_users()));
      exit(EXIT_ERROR_DB_STRUCTURE);
   }

   if (PQntuples(res) > 0) return (atol(PQgetvalue(res, 0, 0)));

   return (False);
}


/**************************************************************************/
/* New uin generating for registration request 				  */
/**************************************************************************/
unsigned long db_users_new_uin()
{
   PGresult *res;
   cstring dbcomm_str;
   unsigned long result_uin;
   char **valid = NULL;

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT max(uin)+1 FROM users_info_ext");

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[GENERATE NEW UIN]");
      return(0);
   }

   if (PQnfields(res) != 1)
   {
      LOG_SYS(0, ("Corrypted table structure in db: \"%s\"\n",
                  lp_db_users()));
      exit(EXIT_ERROR_DB_STRUCTURE);
   }

   if (PQntuples(res) > 0)
   {
      result_uin = strtoul(PQgetvalue(res, 0, 0), valid, 10);
      if (result_uin < 1001) result_uin += 1001;

      PQclear(res);
      return(result_uin);
   }
   else
   {
      PQclear(res);
      return(0);
   }
}


/**************************************************************************/
/* This function used to generate new uin number 			  */
/**************************************************************************/
unsigned long db_users_new_uin2()
{
   PGresult *res;
   cstring dbcomm_str;
   unsigned long result_uin;
   char **valid = NULL;

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT max(uin)+1 FROM register_requests");

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[GENERATE NEW UIN #2]");
      return(0);
   }

   if (PQnfields(res) != 1)
   {
      LOG_SYS(0, ("Corrypted table structure in db: \"%s\"\n",
                  lp_db_users()));
      exit(EXIT_ERROR_DB_STRUCTURE);
   }

   if (PQntuples(res) > 0)
   {
      result_uin = strtoul(PQgetvalue(res, 0, 0), valid, 10);

      PQclear(res);
      return(result_uin);
   }
   else
   {
      PQclear(res);
      return(0);
   }
}


/**************************************************************************/
/* This function used to clear change_password flag in userinfo		  */
/**************************************************************************/
int db_users_clear_ch_pass(unsigned long to_uin)
{
   PGresult *res;
   cstring dbcomm_str;

   /* exec update command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "UPDATE Users_Info_Ext SET cpass=0 WHERE uin=%lu", to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[CLEAR CH_PASSWORD]");
      return(-1);
   }

   if (!strcsequal(PQcmdTuples(res),""))
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
/* This function used to delete user from users_info [unregistration]	  */
/**************************************************************************/
int db_users_delete_user(unsigned long uin_num, char *password)
{
   PGresult *res;
   cstring dbcomm_str;

   /* exec update command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "DELETE FROM Users_Info_Ext WHERE (uin=%lu) AND (pass like '%s')",
	    uin_num, password);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[DELETE DATABASE USER]");
      return(-1);
   }

   if (!strcsequal(PQcmdTuples(res),""))
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
/* This function used to change user's authorization mode		  */
/**************************************************************************/
int db_users_setauthmode(unsigned long to_uin, int auth)
{
   PGresult *res;
   cstring dbcomm_str;

   /* exec update command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "UPDATE Users_Info_Ext SET auth=%d WHERE uin=%lu",
	    auth, to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SET AUTH MODE]");
      return(-1);
   }

   if (!strcsequal(PQcmdTuples(res),""))
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
/* This function used to change user's password				  */
/**************************************************************************/
int db_users_setpassword(unsigned long to_uin, char *new_password)
{
   PGresult *res;
   cstring dbcomm_str;
   fstring npassword;

   /* now we should convert string to specified locale and PG format */
   convert_to_postgres(npassword, sizeof(fstring)-1, new_password);

   /* exec update command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "UPDATE Users_Info_Ext SET pass='%s' WHERE uin=%lu",
	    npassword, to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SET PASSWORD]");
      return(-1);
   }

   if (!strcsequal(PQcmdTuples(res),""))
   {
      PQclear(res);
      db_users_clear_ch_pass(to_uin);

      return(0);
   }
   else
   {
      PQclear(res);
      return(-1);
   }
}


/**************************************************************************/
/* This function used to set user's work info				  */
/**************************************************************************/
int db_users_setwork_info(unsigned long to_uin, struct full_user_info &userinfo)
{
   PGresult *res;
   cstring dbcomm_str;
   fstring waddr, wcity, wphone, wfax, wpager, wstate, wtitle, wcompany, wzip2;

   /* now we should convert string to specified locale and PG format */
   convert_to_postgres(waddr,    sizeof(fstring)-1, userinfo.waddr);
   convert_to_postgres(wcity,    sizeof(fstring)-1, userinfo.wcity);
   convert_to_postgres(wstate,   sizeof(fstring)-1, userinfo.wstate);
   convert_to_postgres(wphone,   sizeof(fstring)-1, userinfo.wphone);
   convert_to_postgres(wfax,     sizeof(fstring)-1, userinfo.wfax);
   convert_to_postgres(wpager,   sizeof(fstring)-1, userinfo.wpager);
   convert_to_postgres(wtitle,   sizeof(fstring)-1, userinfo.wtitle);
   convert_to_postgres(wcompany, sizeof(fstring)-1, userinfo.wcompany);
   convert_to_postgres(wzip2,	 sizeof(fstring)-1, userinfo.wzip2);

#define UPDT_WI431 \
"UPDATE Users_Info_Ext SET \
   waddr='%s', \
   wcity='%s', \
   wstate='%s', \
   wcountry=%d, \
   wphon='%s', \
   wfax='%s', \
   wpager='%s', \
   wzip='%d', \
   wzip2='%s', \
   wdepart='%d', \
   wtitle='%s', \
   wcompany='%s' \
 WHERE uin=%lu"

   /* exec update command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, UPDT_WI431,
	    waddr, wcity, wstate, userinfo.wcountry, wphone, wfax, wpager,
	    userinfo.wzip, wzip2, userinfo.wdepart, wtitle, wcompany, to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SET WORK INFO]");
      return(-1);
   }

   if (!strcsequal(PQcmdTuples(res),""))
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
/* This function used to set homepage category info			  */
/**************************************************************************/
int db_users_sethpagecat_info(unsigned long to_uin, char enabled, short index,
			   char *description)
{
   PGresult *res;
   cstring dbcomm_str;
   fstring description2;

   /* now we should convert string to specified locale and PG format */
   convert_to_postgres(description2,    sizeof(fstring)-1, description);

   /* exec update command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "UPDATE Users_Info_Ext SET hpage_cf=%d, hpage_cat=%d, hpage_txt='%s' WHERE uin=%lu",
	    enabled, index, description2, to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SET HPAGE CATEGORY]");
      return(-1);
   }

   if (!strcsequal(PQcmdTuples(res),""))
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
/* This function used to set user's work info				  */
/**************************************************************************/
int db_users_setsecure_info(unsigned long to_uin, char auth,
			    char iphide, char webaware)
{
   PGresult *res;
   cstring dbcomm_str;

   /* exec update command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "UPDATE Users_Info_Ext SET webaware=%d, iphide=%d, auth=%d WHERE uin=%lu",
	    webaware, iphide, auth, to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SET SECURITY INFO]");
      return(-1);
   }

   if (!strcsequal(PQcmdTuples(res),""))
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
/* This function used to set V5 "more" info 				  */
/**************************************************************************/
int db_users_setV5more_info(unsigned long to_uin,
			    struct full_user_info &userinfo)
{
   PGresult *res;
   cstring dbcomm_str;
   fstring hpage;

   /* now we should convert string to specified locale and PG format */
   convert_to_postgres(hpage,    sizeof(fstring)-1, userinfo.hpage);

   /* exec update command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "UPDATE Users_Info_Ext SET age=%d, sex=%d, hweb='%s', byear=%d, bmon=%d, bday=%d, lang1=%d, lang2=%d, lang3=%d WHERE uin=%lu",
	    userinfo.age, userinfo.gender, hpage, userinfo.byear, userinfo.bmonth,
	    userinfo.bday, userinfo.lang1, userinfo.lang2, userinfo.lang3, to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SET V5 MORE INFO]");
      return(-1);
   }

   if (!strcsequal(PQcmdTuples(res),""))
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
/* This function used to set interests info 				  */
/**************************************************************************/
int db_users_setinterests_info(unsigned long to_uin,
			       struct ext_user_info &userinfo)
{
   PGresult *res;
   cstring dbcomm_str;
   fstring key1, key2, key3, key4;

   /* now we should convert string to specified locale and PG format */
   convert_to_postgres(key1,    sizeof(fstring)-1, userinfo.int_key1);
   convert_to_postgres(key2,    sizeof(fstring)-1, userinfo.int_key2);
   convert_to_postgres(key3,    sizeof(fstring)-1, userinfo.int_key3);
   convert_to_postgres(key4,    sizeof(fstring)-1, userinfo.int_key4);

#define UPDT_II598 \
"UPDATE Users_Info_Ext SET \
   int_num=%d, \
   int_ind1=%d, \
   int_ind2=%d, \
   int_ind3=%d, \
   int_ind4=%d, \
   int_key1='%s', \
   int_key2='%s', \
   int_key3='%s', \
   int_key4='%s' \
 WHERE uin=%lu"

   /* exec update command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, UPDT_II598,
	    userinfo.int_num, userinfo.int_ind1, userinfo.int_ind2,
	    userinfo.int_ind3, userinfo.int_ind4, key1, key2, key3,
	    key4, to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SET INTERESTS]");
      return(-1);
   }

   if (!strcsequal(PQcmdTuples(res),""))
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
/* This function used to set affilations info 				  */
/**************************************************************************/
int db_users_setaffilations_info(unsigned long to_uin,
			       struct ext_user_info &userinfo)
{
   PGresult *res;
   cstring dbcomm_str;
   fstring pkey1, pkey2, pkey3;
   fstring akey1, akey2, akey3;

   /* now we should convert string to specified locale and PG format */
   convert_to_postgres(pkey1,    sizeof(fstring)-1, userinfo.past_key1);
   convert_to_postgres(pkey2,    sizeof(fstring)-1, userinfo.past_key2);
   convert_to_postgres(pkey3,    sizeof(fstring)-1, userinfo.past_key3);

   convert_to_postgres(akey1,    sizeof(fstring)-1, userinfo.aff_key1);
   convert_to_postgres(akey2,    sizeof(fstring)-1, userinfo.aff_key2);
   convert_to_postgres(akey3,    sizeof(fstring)-1, userinfo.aff_key3);

#define UPDT_AI657 \
"UPDATE Users_Info_Ext SET \
   past_num=%d, \
   past_ind1=%d, \
   past_ind2=%d, \
   past_ind3=%d, \
   past_key1='%s', \
   past_key2='%s', \
   past_key3='%s', \
   aff_num=%d, \
   aff_ind1=%d, \
   aff_ind2=%d, \
   aff_ind3=%d, \
   aff_key1='%s', \
   aff_key2='%s', \
   aff_key3='%s' \
 WHERE uin=%lu"

   /* exec update command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, UPDT_AI657,
	    userinfo.past_num, userinfo.past_ind1, userinfo.past_ind2,
	    userinfo.past_ind3, pkey1, pkey2, pkey3, userinfo.aff_num,
	    userinfo.aff_ind1, userinfo.aff_ind2, userinfo.aff_ind3,
	    akey1, akey2, akey3, to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SET AFFILATIONS]");
      return(-1);
   }

   if (!strcsequal(PQcmdTuples(res),""))
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
/* This function used to set user's home info				  */
/**************************************************************************/
int db_users_sethome_info(unsigned long to_uin, struct full_user_info &userinfo)
{
   PGresult *res;
   cstring dbcomm_str;
   fstring haddr, hcity, hphone, hfax, hcell, hstate, hzip2;

   /* now we should convert string to specified locale and PG format */
   convert_to_postgres(haddr,  sizeof(fstring)-1, userinfo.haddr);
   convert_to_postgres(hcity,  sizeof(fstring)-1, userinfo.hcity);
   convert_to_postgres(hstate, sizeof(fstring)-1, userinfo.hstate);
   convert_to_postgres(hphone, sizeof(fstring)-1, userinfo.hphone);
   convert_to_postgres(hfax,   sizeof(fstring)-1, userinfo.hfax);
   convert_to_postgres(hcell,  sizeof(fstring)-1, userinfo.hcell);
   convert_to_postgres(hzip2,  sizeof(fstring)-1, userinfo.hzip2);

#define UPDT_HI720 \
"UPDATE Users_Info_Ext SET \
   haddr='%s', \
   hcity='%s', \
   hstate='%s', \
   hcountry=%d, \
   hphon='%s', \
   hfax='%s', \
   hcell='%s', \
   hzip='%d', \
   hzip2='%s', \
   sex=%d, \
   bday=%d, \
   bmon=%d, \
   byear=%d, \
   age=%d \
 WHERE uin=%lu"

   /* exec update command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, UPDT_HI720,
	    haddr, hcity, hstate, userinfo.hcountry, hphone, hfax,
	    hcell, userinfo.hzip, hzip2, userinfo.gender, userinfo.bday,
	    userinfo.bmonth, userinfo.byear, userinfo.age, to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SET HOME INFO]");
      return(-1);
   }

   if (!strcsequal(PQcmdTuples(res),""))
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
/* This function used to set user's home webpage address 		  */
/**************************************************************************/
int db_users_setwebpage_info(unsigned long to_uin, char *user_page, int htype)
{
   PGresult *res;
   cstring dbcomm_str;
   fstring webpage;

   /* now we should convert string to specified locale and PG format */
   convert_to_postgres(webpage, sizeof(fstring)-1, user_page);

   /* exec update command on backend server */

   if (htype == HOME) {
       slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
       "UPDATE Users_Info_Ext SET hweb='%s' WHERE uin=%lu", webpage, to_uin);
   }

   if (htype == WORK) {
       slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
       "UPDATE Users_Info_Ext SET wweb='%s' WHERE uin=%lu", webpage, to_uin);
   }

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SET WEBPAGE INFO]");
      return(-1);
   }

   if (!strcsequal(PQcmdTuples(res),""))
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
/* This function used to set user's basic info				  */
/**************************************************************************/
int db_users_setbasic_info(unsigned long to_uin, struct full_user_info &userinfo)
{
   PGresult *res;
   cstring dbcomm_str;
   fstring nick, first, last, email;

   /* now we should convert string to specified locale and PG format */
   convert_to_postgres(nick,  sizeof(fstring)-1, userinfo.nick);
   convert_to_postgres(first, sizeof(fstring)-1, userinfo.first);
   convert_to_postgres(last,  sizeof(fstring)-1, userinfo.last);
   convert_to_postgres(email, sizeof(fstring)-1, userinfo.email2);

   /* exec update command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "UPDATE Users_Info_Ext SET nick='%s', frst='%s', last='%s', email1='%s', email2='%s' WHERE uin=%lu",
 	    nick, first, last, email, email, to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SET BASIC INFO]");
      return(-1);
   }

   if (!strcsequal(PQcmdTuples(res),""))
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
/* This function used to set new user's notes				  */
/**************************************************************************/
int db_users_setnotes(unsigned long to_uin, struct notes_user_info &notes)
{
   PGresult *res;
   cstring dbcomm_str, lnotes;

   /* now we should convert string to specified locale and PG format */
   convert_to_postgres(lnotes, sizeof(lnotes)-128, notes.notes);

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "UPDATE Users_Info_Ext SET notes='%s',nnotes='%lu' WHERE uin='%lu'",
	    lnotes, time(NULL), to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SET NOTES]");
      return(-1);
   }

   if (!strcsequal(PQcmdTuples(res),""))
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
/* This function used to obtain user's notes				  */
/**************************************************************************/
int db_users_notes(unsigned long to_uin, struct notes_user_info &notes)
{
   PGresult *res;
   fstring dbcomm_str;

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT notes,nnotes,llog,iadr FROM Users_info_Ext WHERE uin=%lu",
	    to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[GET NOTES]");
      return(-1);
   }

   if (PQnfields(res) != 4)
   {
      LOG_SYS(0, ("Corrypted table structure in db: \"%s\"\n", lp_db_users()));
      exit(EXIT_ERROR_DB_STRUCTURE);
   }

   if (PQntuples(res) > 0)
   {
      strncpy(notes.notes, PQgetvalue(res, 0, 0), sizeof(notes.notes)-1);

      notes.uin	      = to_uin;
      notes.nupdate   = atoul(PQgetvalue(res, 0, 1));
      notes.lastlogin = atoul(PQgetvalue(res, 0, 2));
      notes.ip_addr   = atoul(PQgetvalue(res, 0, 3));

      ITrans.translateToClient(notes.notes);
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
/* This function used in login func to return full user info    	  */
/**************************************************************************/
int db_users_lookup(unsigned long to_uin, struct full_user_info &temp_user)
{
   PGresult *res;
   fstring dbcomm_str;

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT * FROM Users_Info_Ext WHERE uin=%lu", to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[USERS LOOKUP]");
      return(-2);
   }

   if (PQnfields(res) != USER_TBL2_FIELDS)
   {
      LOG_SYS(0, ("Corrypted table structure in db: \"%s\"\n",
                  lp_db_users()));
      exit(EXIT_ERROR_DB_STRUCTURE);
   }

   if (PQntuples(res) > 0)
   {
      char **valid = NULL;

      /* basic info */
      temp_user.uin		= atoul(PQgetvalue(res, 0,  0));
      temp_user.e1publ		= atol(PQgetvalue(res, 0, 14));
      temp_user.disabled	= atol(PQgetvalue(res, 0,  2));
      temp_user.can_broadcast	= atol(PQgetvalue(res, 0,  5));
      temp_user.ch_password	= atol(PQgetvalue(res, 0,  7));
      temp_user.auth		= atol(PQgetvalue(res, 0, 16));
      temp_user.age		= atol(PQgetvalue(res, 0, 18));
      temp_user.gender		= atol(PQgetvalue(res, 0, 17));
      temp_user.gmt_offset      = atol(PQgetvalue(res, 0, 15));

      temp_user.bmonth		= atol(PQgetvalue(res, 0, 20));
      temp_user.bday		= atol(PQgetvalue(res, 0, 19));
      temp_user.byear		= atol(PQgetvalue(res, 0, 21));
      temp_user.wocup		= atol(PQgetvalue(res, 0, 28));

      temp_user.wzip 		= atoul(PQgetvalue(res, 0, 33));
      if (temp_user.wzip == 0)  temp_user.wcountry = -1;
      temp_user.hzip 		= atoul(PQgetvalue(res, 0, 43));
      if (temp_user.hzip == 0)  temp_user.wcountry = -1;
      temp_user.wdepart		= atoul(PQgetvalue(res, 0, 29));
      if (temp_user.wdepart == 0) temp_user.wdepart = -1;

      temp_user.wcountry	= atol(PQgetvalue(res, 0, 25));
      if (temp_user.wcountry == 0) temp_user.wcountry = -1;
      temp_user.hcountry	= atol(PQgetvalue(res, 0, 39));
      if (temp_user.hcountry == 0) temp_user.hcountry = -1;

      temp_user.cr_date		= strtoul(PQgetvalue(res, 0,  6),
                                  valid, 10); valid = NULL;
      temp_user.lastlogin	= strtoul(PQgetvalue(res, 0,  3),
                                  valid, 10); valid = NULL;
      temp_user.ip_addr		= strtoul(PQgetvalue(res, 0,  4),
                                  valid, 10); valid = NULL;
      temp_user.nupdate		= strtoul(PQgetvalue(res, 0, 45),
                                  valid, 10); valid = NULL;

      snprintf(temp_user.passwd, 32, PQgetvalue(res, 0,  1));
      ITrans.translateToClient(temp_user.passwd);
      snprintf(temp_user.nick  , 32, PQgetvalue(res, 0,  8));
      ITrans.translateToClient(temp_user.nick);
      snprintf(temp_user.last  , 32, PQgetvalue(res, 0, 10));
      ITrans.translateToClient(temp_user.last);
      snprintf(temp_user.first , 32, PQgetvalue(res, 0,  9));
      ITrans.translateToClient(temp_user.first);
      snprintf(temp_user.email1, 64, PQgetvalue(res, 0, 11));
      ITrans.translateToClient(temp_user.email1);
      snprintf(temp_user.email2, 64, PQgetvalue(res, 0, 12));
      ITrans.translateToClient(temp_user.email2);
      snprintf(temp_user.email3, 64, PQgetvalue(res, 0, 13));
      ITrans.translateToClient(temp_user.email3);

      snprintf(temp_user.notes , 255, PQgetvalue(res, 0, 45));
      ITrans.translateToClient(temp_user.notes);

      /* home info */
      snprintf(temp_user.haddr , 64, PQgetvalue(res, 0, 36));
      ITrans.translateToClient(temp_user.haddr);
      snprintf(temp_user.hcity , 32, PQgetvalue(res, 0, 37));
      ITrans.translateToClient(temp_user.hcity);
      snprintf(temp_user.hstate, 32, PQgetvalue(res, 0, 38));
      ITrans.translateToClient(temp_user.hstate);
      snprintf(temp_user.hphone, 32, PQgetvalue(res, 0, 40));
      ITrans.translateToClient(temp_user.hphone);
      snprintf(temp_user.hfax  , 32, PQgetvalue(res, 0, 41));
      ITrans.translateToClient(temp_user.hfax);
      snprintf(temp_user.hcell , 32, PQgetvalue(res, 0, 42));
      ITrans.translateToClient(temp_user.hcell);
      snprintf(temp_user.hpage ,128, PQgetvalue(res, 0, 44));
      ITrans.translateToClient(temp_user.hpage);

      /* work info */
      snprintf(temp_user.waddr , 64, PQgetvalue(res, 0, 22));
      ITrans.translateToClient(temp_user.waddr);
      snprintf(temp_user.wcity , 32, PQgetvalue(res, 0, 23));
      ITrans.translateToClient(temp_user.wcity);
      snprintf(temp_user.wstate, 32, PQgetvalue(res, 0, 24));
      ITrans.translateToClient(temp_user.wstate);
      snprintf(temp_user.wcompany, 32, PQgetvalue(res, 0, 26));
      ITrans.translateToClient(temp_user.wcompany);
      snprintf(temp_user.wtitle, 32, PQgetvalue(res, 0, 27));
      ITrans.translateToClient(temp_user.wtitle);
      snprintf(temp_user.wphone, 32, PQgetvalue(res, 0, 30));
      ITrans.translateToClient(temp_user.wphone);
      snprintf(temp_user.wfax  , 32, PQgetvalue(res, 0, 31));
      ITrans.translateToClient(temp_user.wfax);
      snprintf(temp_user.wpager, 32, PQgetvalue(res, 0, 32));
      ITrans.translateToClient(temp_user.wpager);
      snprintf(temp_user.wpage ,128, PQgetvalue(res, 0, 34));
      ITrans.translateToClient(temp_user.wpage);

      temp_user.lang1        = atol(PQgetvalue(res, 0, 46));
      temp_user.lang2        = atol(PQgetvalue(res, 0, 47));
      temp_user.lang3        = atol(PQgetvalue(res, 0, 48));
      temp_user.hpage_cf     = atol(PQgetvalue(res, 0, 49));
      temp_user.hpage_cat    = atol(PQgetvalue(res, 0, 50));

      snprintf(temp_user.hpage_txt ,63, PQgetvalue(res, 0, 51));
      ITrans.translateToClient(temp_user.hpage_txt);
      snprintf(temp_user.wdepart2 ,63, PQgetvalue(res, 0, 52));
      ITrans.translateToClient(temp_user.wdepart2);

      temp_user.past_num     = atol(PQgetvalue(res, 0, 53));
      temp_user.past_ind1    = atol(PQgetvalue(res, 0, 54));
      temp_user.past_ind2    = atol(PQgetvalue(res, 0, 56));
      temp_user.past_ind3    = atol(PQgetvalue(res, 0, 58));

      snprintf(temp_user.past_key1 ,63, PQgetvalue(res, 0, 55));
      ITrans.translateToClient(temp_user.past_key1);
      snprintf(temp_user.past_key2 ,63, PQgetvalue(res, 0, 57));
      ITrans.translateToClient(temp_user.past_key2);
      snprintf(temp_user.past_key3 ,63, PQgetvalue(res, 0, 59));
      ITrans.translateToClient(temp_user.past_key3);


      temp_user.int_num     = atol(PQgetvalue(res, 0, 60));
      temp_user.int_ind1    = atol(PQgetvalue(res, 0, 61));
      temp_user.int_ind2    = atol(PQgetvalue(res, 0, 63));
      temp_user.int_ind3    = atol(PQgetvalue(res, 0, 65));
      temp_user.int_ind4    = atol(PQgetvalue(res, 0, 67));

      snprintf(temp_user.int_key1 ,63, PQgetvalue(res, 0, 62));
      ITrans.translateToClient(temp_user.int_key1);
      snprintf(temp_user.int_key2 ,63, PQgetvalue(res, 0, 64));
      ITrans.translateToClient(temp_user.int_key2);
      snprintf(temp_user.int_key3 ,63, PQgetvalue(res, 0, 66));
      ITrans.translateToClient(temp_user.int_key3);
      snprintf(temp_user.int_key4 ,63, PQgetvalue(res, 0, 68));
      ITrans.translateToClient(temp_user.int_key4);

      temp_user.aff_num     = atol(PQgetvalue(res, 0, 69));
      temp_user.aff_ind1    = atol(PQgetvalue(res, 0, 70));
      temp_user.aff_ind2    = atol(PQgetvalue(res, 0, 72));
      temp_user.aff_ind3    = atol(PQgetvalue(res, 0, 74));

      snprintf(temp_user.aff_key1 ,63, PQgetvalue(res, 0, 71));
      ITrans.translateToClient(temp_user.aff_key1);
      snprintf(temp_user.aff_key2 ,63, PQgetvalue(res, 0, 73));
      ITrans.translateToClient(temp_user.aff_key2);
      snprintf(temp_user.aff_key3 ,63, PQgetvalue(res, 0, 75));
      ITrans.translateToClient(temp_user.aff_key3);

      temp_user.iphide     = atol(PQgetvalue(res, 0, 76));
      temp_user.webaware   = atol(PQgetvalue(res, 0, 77));
      temp_user.martial    = atol(PQgetvalue(res, 0, 78));
      temp_user.bcountry   = atol(PQgetvalue(res, 0, 81));

      snprintf(temp_user.wzip2, 11, PQgetvalue(res, 0, 79));
      ITrans.translateToClient(temp_user.wzip2);
      snprintf(temp_user.hzip2, 11, PQgetvalue(res, 0, 80));
      ITrans.translateToClient(temp_user.hzip2);
      snprintf(temp_user.bstate, 11, PQgetvalue(res, 0, 82));
      ITrans.translateToClient(temp_user.bstate);
      snprintf(temp_user.bcity, 11, PQgetvalue(res, 0, 83));
      ITrans.translateToClient(temp_user.bcity);

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
/* This function used in 'added' messages to return short user info    	  */
/**************************************************************************/
int db_users_lookup_short(unsigned long to_uin, struct full_user_info &temp_user)
{
   PGresult *res;
   fstring dbcomm_str;

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT nick,frst,last,email1 FROM Users_Info_Ext WHERE uin=%lu", to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[USERS LOOKUP SHORT]");
      return(-2);
   }

   if (PQntuples(res) > 0)
   {
      temp_user.uin		= to_uin;

      snprintf(temp_user.nick  , 32, PQgetvalue(res, 0, 0));
      ITrans.translateToClient(temp_user.nick);

      snprintf(temp_user.first , 32, PQgetvalue(res, 0, 1));
      ITrans.translateToClient(temp_user.first);

      snprintf(temp_user.last  , 32, PQgetvalue(res, 0, 2));
      ITrans.translateToClient(temp_user.last);

      snprintf(temp_user.email1, 64, PQgetvalue(res, 0, 3));
      ITrans.translateToClient(temp_user.email1);

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
/* This function used in login func to return full user info    	  */
/**************************************************************************/
int db_new_lookup(unsigned long to_uin, struct full_user_info &temp_user)
{
   PGresult *res;
   fstring dbcomm_str;

   if (!new_users_table_exist()) return (-1);

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT * FROM register_requests WHERE uin=%lu", to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[NEW USERS LOOKUP]");
      return(-2);
   }

   if (PQnfields(res) != USER_TBL2_FIELDS)
   {
      LOG_SYS(0, ("Corrypted table structure in register_requests\n"));
      exit(EXIT_ERROR_DB_STRUCTURE);
   }

   if (PQntuples(res) > 0)
   {
      char **valid = NULL;

      /* basic info */
      temp_user.uin		= atoul(PQgetvalue(res, 0,  0));
      temp_user.e1publ		= atol(PQgetvalue(res, 0, 14));
      temp_user.disabled	= atol(PQgetvalue(res, 0,  2));
      temp_user.can_broadcast	= atol(PQgetvalue(res, 0,  5));
      temp_user.ch_password	= atol(PQgetvalue(res, 0,  7));
      temp_user.auth		= atol(PQgetvalue(res, 0, 16));
      temp_user.age		= atol(PQgetvalue(res, 0, 18));
      temp_user.gender		= atol(PQgetvalue(res, 0, 17));
      temp_user.gmt_offset      = atol(PQgetvalue(res, 0, 15));

      temp_user.bmonth		= atol(PQgetvalue(res, 0, 20));
      temp_user.bday		= atol(PQgetvalue(res, 0, 19));
      temp_user.byear		= atol(PQgetvalue(res, 0, 21));
      temp_user.wocup		= atol(PQgetvalue(res, 0, 28));

      temp_user.cr_date		= strtoul(PQgetvalue(res, 0,  6),
                                  valid, 10); valid = NULL;
      temp_user.lastlogin	= strtoul(PQgetvalue(res, 0,  3),
                                  valid, 10); valid = NULL;
      temp_user.ip_addr		= strtoul(PQgetvalue(res, 0,  4),
                                  valid, 10); valid = NULL;
      temp_user.nupdate		= strtoul(PQgetvalue(res, 0, 45),
                                  valid, 10); valid = NULL;

      snprintf(temp_user.nick  , 32, PQgetvalue(res, 0,  8));
      ITrans.translateToClient(temp_user.nick);
      snprintf(temp_user.last  , 32, PQgetvalue(res, 0, 10));
      ITrans.translateToClient(temp_user.last);
      snprintf(temp_user.first , 32, PQgetvalue(res, 0,  9));
      ITrans.translateToClient(temp_user.first);
      snprintf(temp_user.email1, 64, PQgetvalue(res, 0, 11));
      ITrans.translateToClient(temp_user.email1);
      snprintf(temp_user.wzip2, 11, PQgetvalue(res, 0, 79));
      ITrans.translateToClient(temp_user.wzip2);
      snprintf(temp_user.hzip2, 11, PQgetvalue(res, 0, 80));
      ITrans.translateToClient(temp_user.hzip2);
      snprintf(temp_user.bstate, 11, PQgetvalue(res, 0, 82));
      ITrans.translateToClient(temp_user.bstate);
      snprintf(temp_user.bcity, 11, PQgetvalue(res, 0, 83));
      ITrans.translateToClient(temp_user.bcity);

      temp_user.lang1        = atol(PQgetvalue(res, 0, 46));
      temp_user.lang2        = atol(PQgetvalue(res, 0, 47));
      temp_user.lang3        = atol(PQgetvalue(res, 0, 48));

      temp_user.iphide     = atol(PQgetvalue(res, 0, 76));
      temp_user.webaware   = atol(PQgetvalue(res, 0, 77));
      temp_user.martial    = atol(PQgetvalue(res, 0, 78));
      temp_user.bcountry   = atol(PQgetvalue(res, 0, 81));

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
/* This function used in retrieve-V5-full-info to get extended info  	  */
/**************************************************************************/
int db_users_lookup(unsigned long to_uin, struct ext_user_info &temp_user)
{
   PGresult *res;
   fstring dbcomm_str;

#define SLCT_EI1128 \
"SELECT \
   lang1,lang2,lang3,hpage_cf,hpage_cat,past_num, past_ind1, \
   past_ind2,past_ind3,aff_num,aff_ind1,aff_ind2,aff_ind3,int_num, \
   int_ind1,int_ind2,int_ind3,int_ind4,hpage_txt,int_key1,int_key2, \
   int_key3,int_key4,aff_key1,aff_key2,aff_key3,past_key1,past_key2, \
   past_key3,wdepart2 \
 FROM Users_Info_Ext \
 WHERE uin=%lu"

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, SLCT_EI1128, to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[EXT USERS LOOKUP]");
      return(-2);
   }

   if (PQnfields(res) != USERS_DB_FNUM)
   {
      LOG_SYS(0, ("Corrupted table structure in db: \"%s\"\n",
                  lp_db_users()));
      exit(EXIT_ERROR_DB_STRUCTURE);
   }

   if (PQntuples(res) > 0)
   {
      /* basic info */
      temp_user.lang1		= atol(PQgetvalue(res, 0,  0));
      temp_user.lang2		= atol(PQgetvalue(res, 0,  1));
      temp_user.lang3		= atol(PQgetvalue(res, 0,  2));
      temp_user.hpage_cf	= atol(PQgetvalue(res, 0,  3));
      temp_user.hpage_cat	= atol(PQgetvalue(res, 0,  4));
      temp_user.past_num	= atol(PQgetvalue(res, 0,  5));
      temp_user.past_ind1	= atol(PQgetvalue(res, 0,  6));
      temp_user.past_ind2	= atol(PQgetvalue(res, 0,  7));
      temp_user.past_ind3       = atol(PQgetvalue(res, 0,  8));

      temp_user.aff_num         = atol(PQgetvalue(res, 0,  9));
      temp_user.aff_ind1        = atol(PQgetvalue(res, 0,  10));
      temp_user.aff_ind2        = atol(PQgetvalue(res, 0,  11));
      temp_user.aff_ind3        = atol(PQgetvalue(res, 0,  12));

      temp_user.int_num         = atol(PQgetvalue(res, 0,  13));
      temp_user.int_ind1        = atol(PQgetvalue(res, 0,  14));
      temp_user.int_ind2        = atol(PQgetvalue(res, 0,  15));
      temp_user.int_ind3        = atol(PQgetvalue(res, 0,  16));
      temp_user.int_ind4        = atol(PQgetvalue(res, 0,  17));

      snprintf(temp_user.hpage_txt, 128, PQgetvalue(res, 0,  18));
      ITrans.translateToClient(temp_user.hpage_txt);

      snprintf(temp_user.int_key1, 64, PQgetvalue(res, 0,  19));
      ITrans.translateToClient(temp_user.int_key1);
      snprintf(temp_user.int_key2, 64, PQgetvalue(res, 0,  20));
      ITrans.translateToClient(temp_user.int_key2);
      snprintf(temp_user.int_key3, 64, PQgetvalue(res, 0,  21));
      ITrans.translateToClient(temp_user.int_key3);
      snprintf(temp_user.int_key4, 64, PQgetvalue(res, 0,  22));
      ITrans.translateToClient(temp_user.int_key4);

      snprintf(temp_user.aff_key1, 64, PQgetvalue(res, 0,  23));
      ITrans.translateToClient(temp_user.aff_key1);
      snprintf(temp_user.aff_key2, 64, PQgetvalue(res, 0,  24));
      ITrans.translateToClient(temp_user.aff_key2);
      snprintf(temp_user.aff_key3, 64, PQgetvalue(res, 0,  25));
      ITrans.translateToClient(temp_user.aff_key3);

      snprintf(temp_user.past_key1, 64, PQgetvalue(res, 0,  26));
      ITrans.translateToClient(temp_user.past_key1);
      snprintf(temp_user.past_key2, 64, PQgetvalue(res, 0,  27));
      ITrans.translateToClient(temp_user.past_key2);
      snprintf(temp_user.past_key3, 64, PQgetvalue(res, 0,  28));
      ITrans.translateToClient(temp_user.past_key3);

      snprintf(temp_user.wdepart2, 64, PQgetvalue(res, 0,  29));
      ITrans.translateToClient(temp_user.wdepart2);


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
/* This function used in login func to return partial user info 	  */
/**************************************************************************/
int db_users_lookup(unsigned long to_uin, struct login_user_info &temp_user)
{
   PGresult *res;
   fstring dbcomm_str;

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT uin,pass,e1publ,ulock,bcst,cpass,auth,iadr FROM Users_info_Ext WHERE uin=%lu",
	    to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[LOGIN USERS LOOKUP]");
      return(-2);
   }

   if (PQnfields(res) != 8)
   {
    LOG_SYS(0, ("Corrupted table structure in db: \"%s\"\n",
                  lp_db_users()));
      exit(EXIT_ERROR_DB_STRUCTURE);
   }

   if (PQntuples(res) > 0)
   {
      temp_user.uin		= atoul(PQgetvalue(res, 0, 0));
      temp_user.pemail1		= atol(PQgetvalue(res, 0, 2));
      temp_user.disabled	= atol(PQgetvalue(res, 0, 3));
      temp_user.can_broadcast	= atol(PQgetvalue(res, 0, 4));
      temp_user.ch_password	= atol(PQgetvalue(res, 0, 5));
      temp_user.auth		= atol(PQgetvalue(res, 0, 6));
      temp_user.ip_addr		= atoul(PQgetvalue(res, 0, 7));

      strncpy(temp_user.passwd, PQgetvalue(res, 0, 1), 32);
      ITrans.translateToClient(temp_user.passwd);

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
/* This function used to update user information (ip, lastlogin)	  */
/**************************************************************************/
int db_users_touch(struct online_user &user)
{
   PGresult *res;
   fstring dbcomm_str;

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "UPDATE Users_info_Ext SET llog=%lu,iadr=%lu WHERE uin=%lu",
	    timeToLong(time(NULL)), ipToIcq(user.ip), user.uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[DB_USERS TOUCH]");
      return(-1);
   }

  PQclear(res);
  return(0);
}


/**************************************************************************/
/* If user's account locked this function used to obtain 		  */
/* iserver administrator lock message 					  */
/**************************************************************************/
int db_users_lock_message(unsigned long to_uin, fstring &lock_text)
{
   PGresult *res;
   fstring dbcomm_str;

   /* exec select command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT lck_text FROM Users_lock WHERE luin=%lu", to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[GET LOCK MESSAGE]");
      return(0);
   }

   if (PQnfields(res) != 1)
   {
    LOG_SYS(0, ("Corrupted table structure in db: \"%s\"\n",
                  lp_db_users()));
      exit(EXIT_ERROR_DB_STRUCTURE);
   }

   if (PQntuples(res) > 0)
   {
      snprintf(lock_text, sizeof(lock_text)-1, PQgetvalue(res, 0, 0));
      PQclear(res);
      return(1);
   }
   else
   {
      PQclear(res);
      return(0);
   }

}


/**************************************************************************/
/* This function make connection to users database		 	  */
/**************************************************************************/
void usersdb_connect()
{
   char	    *pghost, *pgport,  *pgoptions, *pgtty;
   char	    *dbName, *dblogin, *dbpassw;

   pghost  = lp_db_addr();	/* address of db server if NULL - unix */
   pgport  = lp_db_port();	/* port of the backend */
   dbName  = lp_db_users();	/* online database name */
   dblogin = lp_db_user();	/* database user */
   dbpassw = lp_db_pass();	/* database password */

   /* I found that people often use default "sicq" password for    */
   /* their databases. This is very stupid and I must do something */
   /* about to solve this REAL security problem                    */

   if (!strcmp(dbpassw, "sicq") || (!strcmp(dbpassw, "DEFAULT")))
   {
      LOG_SYS(0, ("Hey man, ARE YOU CRAZY ? You are trying to use default database password\n"));
      LOG_SYS(0, ("Sorry, but I'm shuting down. Try to change password first\n"));
      exit(EXIT_ERROR_DB_CONNECT);
   }

   pgoptions = NULL;		/* special options for backend server */
   pgtty     = NULL;		/* debugging tty for the backend server */

				/* make a connection to the database */
   users_dbconn = PQsetdbLogin(pghost, pgport, pgoptions, pgtty,
                               dbName, dblogin, dbpassw);

   if (PQstatus(users_dbconn) == CONNECTION_BAD)
   {
   	   LOG_SYS(0, ("Connection to users database failed.\n"));
	   exit(EXIT_ERROR_DB_CONNECT);
   }

   /* notice processor to catch errors and failures */
   PQsetNoticeProcessor(users_dbconn,
   (void (*)(void *, const char *))uNoticeStub, NULL);
}



/**************************************************************************/
/* This function used to set user's basic info in V5 protocol module	  */
/**************************************************************************/
int db_users_setbasic_info2(unsigned long to_uin,
			    struct full_user_info &userinfo)
{
   PGresult *res;
   cstring dbcomm_str;
   ffstring nick, first, last, email1, email2, email3, hcity;
   ffstring hphone, hfax, hcell, haddr, hstate;

   /* now we should convert string to specified locale and PG format */
   convert_to_postgres(nick,   sizeof(ffstring)-1, userinfo.nick);
   convert_to_postgres(first,  sizeof(ffstring)-1, userinfo.first);
   convert_to_postgres(last,   sizeof(ffstring)-1, userinfo.last);
   convert_to_postgres(email1, sizeof(ffstring)-1, userinfo.email1);
   convert_to_postgres(email2, sizeof(ffstring)-1, userinfo.email2);
   convert_to_postgres(email3, sizeof(ffstring)-1, userinfo.email3);
   convert_to_postgres(hcity,  sizeof(ffstring)-1, userinfo.hcity);
   convert_to_postgres(hstate, sizeof(ffstring)-1, userinfo.hstate);
   convert_to_postgres(hphone, sizeof(ffstring)-1, userinfo.hphone);
   convert_to_postgres(hfax,   sizeof(ffstring)-1, userinfo.hfax);
   convert_to_postgres(hcell,  sizeof(ffstring)-1, userinfo.hcell);
   convert_to_postgres(haddr,  sizeof(ffstring)-1, userinfo.haddr);

#define UPDT_BI1407 \
"UPDATE Users_Info_Ext SET \
   nick='%s', \
   frst='%s', \
   last='%s', \
   email1='%s', \
   email2='%s', \
   email3='%s', \
   hcity='%s', \
   hstate='%s', \
   hphon='%s', \
   hfax='%s', \
   hcell='%s', \
   haddr='%s', \
   hcountry=%d, \
   gmtoffs=%d, \
   hzip='%lu', \
   e1publ=%d \
 WHERE uin=%lu"

   /* exec update command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, UPDT_BI1407,
	nick, first, last, email1, email2, email3, hcity, hstate, hphone,
	hfax, hcell, haddr, userinfo.hcountry, userinfo.gmt_offset,
	userinfo.hzip, userinfo.e1publ, to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SET BASIC INFO #2]");
      return(-1);
   }

   if (!strcsequal(PQcmdTuples(res),""))
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
/* This function used to set user's basic info in V7 protocol module	  */
/**************************************************************************/
int db_users_setbasic_info3(unsigned long to_uin,
			    struct full_user_info &userinfo)
{
   PGresult *res;
   cstring dbcomm_str;
   ffstring nick, first, last, email1, hcity;
   ffstring hphone, hfax, hcell, haddr, hstate;

   /* now we should convert string to specified locale and PG format */
   convert_to_postgres(nick,   sizeof(ffstring)-1, userinfo.nick);
   convert_to_postgres(first,  sizeof(ffstring)-1, userinfo.first);
   convert_to_postgres(last,   sizeof(ffstring)-1, userinfo.last);
   convert_to_postgres(email1, sizeof(ffstring)-1, userinfo.email1);
   convert_to_postgres(hcity,  sizeof(ffstring)-1, userinfo.hcity);
   convert_to_postgres(hstate, sizeof(ffstring)-1, userinfo.hstate);
   convert_to_postgres(hphone, sizeof(ffstring)-1, userinfo.hphone);
   convert_to_postgres(hfax,   sizeof(ffstring)-1, userinfo.hfax);
   convert_to_postgres(hcell,  sizeof(ffstring)-1, userinfo.hcell);
   convert_to_postgres(haddr,  sizeof(ffstring)-1, userinfo.haddr);

#define UPDT_BI1476 \
"UPDATE Users_Info_Ext SET \
   nick='%s', \
   frst='%s', \
   last='%s', \
   email1='%s', \
   hcity='%s', \
   hstate='%s', \
   hphon='%s', \
   hfax='%s', \
   hcell='%s', \
   haddr='%s', \
   hcountry=%d, \
   gmtoffs=%d, \
   hzip='%lu', \
   e1publ=%d \
 WHERE uin=%lu"

   /* exec update command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, UPDT_BI1476,
	nick, first, last, email1, hcity, hstate, hphone, hfax, hcell, haddr,
	userinfo.hcountry, userinfo.gmt_offset, userinfo.hzip, userinfo.e1publ, to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SET BASIC INFO #2]");
      return(-1);
   }

   if (!strcsequal(PQcmdTuples(res),""))
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
/* This function used to set user's work info in V5 protocol module	  */
/**************************************************************************/
int db_users_setwork_info2(unsigned long to_uin,
			   struct full_user_info &userinfo)
{
   PGresult *res;
   cstring dbcomm_str;
   ffstring wcity, wstate, wphone, wfax, waddr;
   ffstring wdepart, wposition, wpage, wcompany;

   /* now we should convert string to specified locale and PG format */
   convert_to_postgres(wcity,     sizeof(ffstring)-1, userinfo.wcity);
   convert_to_postgres(wstate,    sizeof(ffstring)-1, userinfo.wstate);
   convert_to_postgres(wphone,    sizeof(ffstring)-1, userinfo.wphone);
   convert_to_postgres(wfax,      sizeof(ffstring)-1, userinfo.wfax);
   convert_to_postgres(waddr,     sizeof(ffstring)-1, userinfo.waddr);
   convert_to_postgres(wcompany,  sizeof(ffstring)-1, userinfo.wcompany);
   convert_to_postgres(wdepart,   sizeof(ffstring)-1, userinfo.wdepart2);
   convert_to_postgres(wposition, sizeof(ffstring)-1, userinfo.wtitle);
   convert_to_postgres(wpage,     sizeof(ffstring)-1, userinfo.wpage);

#define UPDT_WI1541 \
"UPDATE Users_Info_Ext SET \
   wcity='%s', \
   wstate='%s', \
   wphon='%s', \
   wfax='%s', \
   waddr='%s', \
   wcompany='%s', \
   wtitle='%s', \
   wweb='%s', \
   wocup=%d, \
   wcountry=%d, \
   wzip=%lu, \
   wdepart2='%s' \
 WHERE uin=%lu"

   /* exec update command on backend server */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, UPDT_WI1541,
	wcity, wstate, wphone, wfax, waddr, wcompany, wposition, wpage,
	userinfo.wocup, userinfo.wcountry, userinfo.wzip, wdepart, to_uin);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SET WORK INFO #2]");
      return(-1);
   }

   if (!strcsequal(PQcmdTuples(res),""))
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

