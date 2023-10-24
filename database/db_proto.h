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
/*                                                                        */
/**************************************************************************/

#ifndef DB_PROTO_H
#define DB_PROTO_H

int check_online_table();
int vacuum_online_tables();
int db_online_lookup(unsigned long user_uin, struct online_user &temp_user);
int db_add_online_lookup(unsigned long caller_uin, unsigned long user_uin, struct online_user &temp_user);
int need_notification(unsigned long auin, unsigned long to_uin, struct online_user &auser);
int db_online_lookup(unsigned long to_uin, struct online_user &temp_user, unsigned short magnum1, unsigned short magnum2);
int db_online_sseq_open(struct online_user &temp_user);
int db_online_sseq_close(struct online_user &temp_user, int);
int db_online_delete(unsigned long uin, int shm=1);
int db_online_insert(struct online_user &temp_user);
int db_online_touch(struct online_user &temp_user);
int db_online_update_servseq(struct online_user &temp_user);
int db_online_add_servseq(unsigned long uin, int number);
int db_online_setstatus(struct online_user &temp_user);
int db_online_setstate(struct online_user &temp_user, int state);
int db_online_activate_user(unsigned long uin);
int db_online_clear();
int db_online_save_profile(char *sn, int type, char *mime, char *data, int datasize);
int db_online_get_profile(char *sn, int type, char *mime, int max_mime, char *data, int max_data);

void oNoticeStub(void * arg, const char * message);
void uNoticeStub(void * arg, const char * message);

int db_contacts_clear();
int db_contact_insert(unsigned long uin, int number, unsigned long *contact, int type, unsigned long rid);
int int_db_contact_insert_c(unsigned long uin, int number, unsigned long *contact, int type, unsigned long rid);
int int_db_contact_insert_i(unsigned long uin, int number, unsigned long *contact, int type, unsigned long rid);

int db_contact_delete(unsigned long uin);
int db_contact_lookup(unsigned long uin, int type, unsigned long *contact[]);
int db_contact_delete(unsigned long ouin, unsigned long tuin, unsigned short type);
int db_contact_delete(unsigned long ouin, unsigned short type);
int db_users_lookup(unsigned long to_uin, struct full_user_info &temp_user);
int db_users_lookup_short(unsigned long to_uin, struct full_user_info &temp_user);
int db_users_lookup(unsigned long to_uin, struct login_user_info &temp_user);
int db_users_lookup(unsigned long to_uin, struct ext_user_info &temp_user);

int db_users_delete_user(unsigned long uin_num, char *password);
int db_users_touch(struct online_user &user);
int db_users_notes(unsigned long to_uin, struct notes_user_info &notes);
int db_users_lock_message(unsigned long to_uin, fstring &lock_text);
int db_users_setnotes(unsigned long to_uin, struct notes_user_info &notes);
int db_users_setbasic_info(unsigned long to_uin, struct full_user_info &userinfo);
int db_users_setbasic_info2(unsigned long to_uin, struct full_user_info &userinfo);
int db_users_setbasic_info3(unsigned long to_uin, struct full_user_info &userinfo);
int db_users_setwebpage_info(unsigned long to_uin, char *user_page, int htype);
int db_users_sethome_info(unsigned long to_uin, struct full_user_info &userinfo);
int db_users_setwork_info(unsigned long to_uin, struct full_user_info &userinfo);
int db_users_setwork_info2(unsigned long to_uin, struct full_user_info &userinfo);
int db_users_setV5more_info(unsigned long to_uin, struct full_user_info &userinfo);
int db_users_setsecure_info(unsigned long to_uin, char auth, char iphide, char webaware);
int db_users_setpassword(unsigned long to_uin, char *new_password);
int db_users_setauthmode(unsigned long to_uin, int auth);
int db_users_sethpagecat_info(unsigned long to_uin, char enabled, short index, char *description);
int db_users_setinterests_info(unsigned long to_uin, struct ext_user_info &userinfo);
int db_users_setaffilations_info(unsigned long to_uin, struct ext_user_info &userinfo);

int db_users_clear_ch_pass(unsigned long to_uin);
int db_users_add_user(struct full_user_info &user);
int db_new_add_user(struct full_user_info &user);
int db_new_lookup(unsigned long to_uin, struct full_user_info &temp_user);

unsigned long db_users_new_uin();
unsigned long db_users_new_uin2();

int db_add_message(struct msg_header &msg_hdr, char *message);
int db_del_messages(unsigned long to_uin, unsigned long last_time);

int db_defrag_addpart(unsigned long from_uin, int seq, int part_num, int part_cnt, int len, char *buffer);
int db_defrag_delete();
int db_defrag_delete(unsigned long from_uin);
int db_defrag_delete(unsigned long from_uin, unsigned long seq);
int db_defrag_check();
int db_defrag_check(unsigned long from_uin, unsigned short seq);
BOOL fragment_exist(PGresult *res, unsigned short tuples_num, unsigned short frag_number);

void usersdb_connect();
void db_commit();

BOOL new_users_table_exist();
void create_new_users_table();

/* init_db.cpp protos */
BOOL check_and_fix_database(char *dbname, char *dbuser, char *dbpass, char *dbaddr, char *dbport);
BOOL check_user_tbl(PGconn *dbconn, char *tbl_name, int fldnum);
BOOL check_user_tbl2(PGconn *dbconn, char *tbl_name, int fldnum);
BOOL create_user_tbl(PGconn *dbconn, char *cr_query);
BOOL grant_permissions(PGconn *dbconn, char *tbl_name, char *username);
BOOL delete_user_tbl(PGconn *dbconn, char *tblname);

/* error_db.cpp */
int  handle_database_error(PGresult *res, char where[40]);
void wait_for_database();

int db_cookie_insert(unsigned long uin, char *cookie, unsigned short type);
int db_cookie_delete(unsigned long uin);
int db_cookie_delete(char *cookie);
unsigned long db_cookie_check(char *cookie);
void db_cookie_use(unsigned long uin);
void db_cookie_use(char *cookie);
void db_cookie_check_expired();
int db_cookie_get(unsigned long uin, char *cookie, unsigned short &used, unsigned short type);

/* ssi_db.cpp */
int db_ssi_get_modinfo(struct online_user *user, unsigned long &mod_date, unsigned short &item_num);
int db_ssi_set_modinfo(struct online_user *user, BOOL import);
int ssi_db_add_item(struct online_user *user, char *item_name, unsigned short gid, unsigned short iid, unsigned short itype, class tlv_chain_c &tlv_chain);
int ssi_db_update_item(struct online_user *user, char *item_name, unsigned short gid, unsigned short iid, unsigned short itype, class tlv_chain_c &tlv_chain);
int ssi_db_del_item(struct online_user *user, char *item_name, unsigned short gid, unsigned short iid, unsigned short itype, class tlv_chain_c &tlv_chain);
unsigned short db_ssi_get_itemnum(unsigned long uin, unsigned short itype);
BOOL db_ssi_get_auth(unsigned long uin, unsigned long tuin);
void db_ssi_auth_add(unsigned long uin, unsigned long tuin, unsigned long rid);
int db_item_count(unsigned long uin, unsigned short type, int gid, int iid);
BOOL db_ssi_auth_grant(unsigned long fuin, unsigned long tuin);
BOOL db_ssi_get_added(unsigned long uin, unsigned long by_uin);
void db_ssi_added_add(unsigned long uin, unsigned long by_uin, unsigned long rid);
int db_ssi_check_dgroup(struct online_user *user);
int db_ssi_create_modinfo(struct online_user *user);
int update_import_item(unsigned long uin);

#endif /* DB_PROTO_H */

