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
/* This file contain definitions for database table-create-queries and 	  */
/* table parameters for automatic database initialization and checks 	  */
/*                                                                        */
/**************************************************************************/

/* index in check_result array */
#define DB_EXIST 	  0

/* online contacts table initialization data */
#define CONT_TBL_OK	  1
#define CONT_TBL_FIELDS   4
#define CONT_TBL_NAME     "online_contacts"
#define CONT_TBL_CREATE	  "CREATE TABLE Online_Contacts (ouin float8, tuin float8, type int2, rid float8 DEFAULT 0); \
			   CREATE INDEX onln_ouin ON Online_Contacts (ouin); \
			   CREATE INDEX onln_tuin ON Online_Contacts (tuin); \
			   CREATE INDEX onln_rid ON Online_Contacts (rid);"

/* departments table initialization data */
#define DEPS_TBL_OK	  2
#define DEPS_TBL_FIELDS   3
#define DEPS_TBL_NAME     "users_deps"
#define DEPS_TBL_CREATE	  "CREATE TABLE Users_Deps (depcode float8, depmin name, depname text); \
		           CREATE UNIQUE INDEX depcode ON Users_Deps (depcode);"

/* fragment storage table initialization data */
#define DFRG_TBL_OK	  3
#define DFRG_TBL_FIELDS   7
#define DFRG_TBL_NAME     "fragment_storage"
#define DFRG_TBL_CREATE   "CREATE TABLE Fragment_storage (uin float8, seq int4, prt_num int2, \
    			   prt_cnt int2, len int2, frg int4, frgtime int8); \
		           CREATE INDEX frg_uin ON Fragment_storage (uin); \
		           CREATE INDEX frg_prt ON Fragment_storage (prt_num); \
		           CREATE INDEX frg_seq ON Fragment_storage (seq);"

/* locks table initialization data */
#define LOCK_TBL_OK	  4
#define LOCK_TBL_FIELDS   2
#define LOCK_TBL_NAME     "users_lock"
#define LOCK_TBL_CREATE   "CREATE TABLE Users_Lock (luin float8, lck_text text); \
		           CREATE UNIQUE INDEX lock_uin ON Users_Lock (luin); "

/* messages table initialization data */
#define MESS_TBL_OK	  5
#define MESS_TBL_FIELDS   5
#define MESS_TBL_NAME     "users_messages"
#define MESS_TBL_CREATE   "CREATE TABLE Users_Messages (to_uin float8, from_uin float8, msg_date float8, \
		           msg_type int4, msgtext text); \
		           CREATE INDEX mess_from_uin ON Users_Messages (from_uin); \
		           CREATE INDEX mess_to_uin ON Users_Messages (to_uin); \
		           CREATE INDEX mess_msg_date ON Users_Messages (msg_date);"

/* online_users table initialization data */
#define ONLN_TBL_OK	  6
#define ONLN_TBL_FIELDS   17
#define ONLN_TBL_NAME     "online_users"
#define ONLN_TBL_CREATE   "CREATE TABLE Online_Users (uin float8, iadr float8, uprt int4, \
		           tprt int4, stat int4, uptm float8, lutm float8, ttl int4, \
		           prv int2, sseq int4, sseq2 int4, sess_id float8, state int2, \
			   tcpver int2, estat int4, active int2 DEFAULT 0, ishm int4 DEFAULT 0); \
		           CREATE UNIQUE INDEX onln_uin ON Online_Users(uin);"

/* users table initialization data */
#define USER_TBL1_OK	   7
#define USER_TBL1_FIELDS   46
#define USER_TBL1_NAME     "users_info"
#define USER_TBL1_CREATE   "CREATE VIEW Users_Info AS SELECT uin, pass, ulock, \
			   llog, iadr, bcst, cdate, cpass, nick, frst, last, \
			   email1, email2, email3, e1publ, gmtoffs, auth, \
			   sex, age, bday, bmon, byear, waddr, wcity, wstate, \
			   wcountry, wcompany, wtitle, wocup, wdepart, wphon, \
		           wfax, wpager, wzip, wweb, notes, haddr, hcity, \
			   hstate, hcountry, hphon, hfax, hcell, hzip, hweb, \
			   nnotes FROM Users_Info_Ext; "

/* users table initialization data */
#define USER_TBL2_OK	   8
#define USER_TBL2_FIELDS   84
#define USER_TBL2_NAME     "users_info_ext"
#define USER_TBL2_CREATE   "CREATE TABLE Users_Info_Ext (uin float8, pass text, ulock int2, llog float8, \
		           iadr float8, bcst int2, cdate float8, cpass int2, nick text, frst text, \
		           last text, email1 text, email2 text, email3 text, e1publ int2, \
		           gmtoffs int2, auth int2, sex int2, age int2, bday int2, bmon int2, \
		           byear int2, waddr text, wcity text, wstate text, wcountry int4, \
		           wcompany text, wtitle text, wocup int2, wdepart text, wphon text, \
		           wfax text, wpager text, wzip text, wweb text, notes text, haddr text, \
		           hcity text, hstate text, hcountry int4, hphon text, hfax text, \
		           hcell text, hzip text, hweb text, nnotes float8, lang1 int2 DEFAULT 0, \
			   lang2 int2 DEFAULT 0, lang3 int2 DEFAULT 0, hpage_cf int2 DEFAULT 0, \
			   hpage_cat int4 DEFAULT 0, hpage_txt text DEFAULT '', \
			   wdepart2 text DEFAULT '', past_num int2 DEFAULT 0, \
			   past_ind1 int4 DEFAULT 0, past_key1 text DEFAULT '', \
			   past_ind2 int4 DEFAULT 0, past_key2 text DEFAULT '', \
			   past_ind3 int4 DEFAULT 0, past_key3 text DEFAULT '', \
			   int_num int2 DEFAULT 0, int_ind1 int4 DEFAULT 0, \
			   int_key1 text DEFAULT '', int_ind2 int4 DEFAULT 0, \
			   int_key2 text DEFAULT '', int_ind3 int4 DEFAULT 0, \
			   int_key3 text DEFAULT '', int_ind4 int4 DEFAULT 0, \
			   int_key4 text DEFAULT '', aff_num int2 DEFAULT 0, \
			   aff_ind1 int4 DEFAULT 0, aff_key1 text DEFAULT '', aff_ind2 int4 DEFAULT 0, \
			   aff_key2 text DEFAULT '', aff_ind3 int4 DEFAULT 0, aff_key3 text DEFAULT '', \
			   iphide int2 DEFAULT 00, webaware int2 DEFAULT 1, martial int2 DEFAULT 0, \
			   wzip2 text, hzip2 text, bcountry int4, bstate text, bcity text); "

/* indices will slow down update and insert queries */
/* but they speed up search, select queries.        */
#define USER_TBL2_CREATE1 "CREATE UNIQUE INDEX euser_uin ON Users_Info_Ext (uin); \
			   CREATE INDEX euser_last ON Users_Info_Ext (last); \
		           CREATE INDEX euser_frst ON Users_Info_Ext (frst); \
		           CREATE INDEX euser_nick ON Users_Info_Ext (nick); \
		           CREATE INDEX euser_iadr ON Users_Info_Ext (iadr); \
		           CREATE INDEX euser_llog ON Users_Info_Ext (llog); \
		           CREATE VIEW Users AS SELECT uin, frst, last, bcst, pass FROM Users_Info_Ext;"


/* users login cookies table initialization data */
#define LOGC_TBL_OK        9
#define LOGC_TBL_FIELDS    5
#define LOGC_TBL_NAME     "login_cookies"
#define LOGC_TBL_CREATE   "CREATE TABLE login_cookies (uin float8, cdate float8, cookie text, used int2, type int2); \
			   CREATE UNIQUE INDEX cookie_uin ON login_cookies (uin);"
			   

/* user ssi table initialization data */
#define USSI_TBL_OK      10
#define USSI_TBL_FIELDS  17
#define USSI_TBL_NAME   "users_ssi_data"
#define USSI_TBL_CREATE "CREATE TABLE users_ssi_data \
                         ( \
			    ouin float8, uin float8, gid int4, iid int4, type int2, \
			    itype int2, auth int2, udate float8, revnum int2, \
			    iname text, nick text, blob text, iperm float8, \
			    privacy int2, vclass float8, perms float8, \
			    readonly int2 DEFAULT 0 \
			 ); \
			 CREATE UNIQUE INDEX ssi_data_idx ON users_ssi_data (ouin, gid, iid, type); \
			 CREATE INDEX ssi_data_uin ON users_ssi_data (uin);"


/* authorization/added table */
#define UPRM_TBL_OK	  11
#define UPRM_TBL_FIELDS   4
#define UPRM_TBL_NAME     "users_perms"
#define UPRM_TBL_CREATE	  "CREATE TABLE Users_Perms (uin float8, tuin float8, type int2, rid float8); \
			   CREATE UNIQUE INDEX uperm_ind ON Users_Perms (uin,tuin,type);"

/* online profiles table */
#define OPRF_TBL_OK	  12
#define OPRF_TBL_FIELDS   4
#define OPRF_TBL_NAME     "online_profiles"
#define OPRF_TBL_CREATE	  "CREATE TABLE Online_Profiles (sn text, type int2, mime text, data text); \
			   CREATE UNIQUE INDEX oprof_ind ON Online_Profiles (sn,type);"

#ifdef HAVE_GD
/* regimage code table */
#define REGI_TBL_OK	  13
#define REGI_TBL_FIELDS	  3
#define REGI_TBL_NAME	  "regimage_codes"
#define REGI_TBL_CREATE	  "CREATE TABLE Regimage_Codes (iadr float8, cdate float8,code text); \
			  CREATE UNIQUE INDEX regi_ind ON Regimage_Codes (iadr);"
#endif
