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
/**************************************************************************/

#ifndef _V5_PROTO_H
#define _V5_PROTO_H

void handle_v5_proto(Packet &pack);
void V5Encrypt(Packet &pack);
void V5Decrypt(Packet &pack);
unsigned long  ReverseLong(unsigned long l);
unsigned short ReverseShort(unsigned short l);
unsigned long GetKey(Packet &pack);
unsigned long calculate_checkcode(Packet &pack);
void PutKey(Packet &pack, unsigned long cc);
void PutSeq(Packet &pack, unsigned short cc);
void v5_send_indirect(Packet &pack, unsigned long to_uin, unsigned long shm_index);
void v5_proto_init();
void v5_send_busy(Packet &pack);
void v5_send_srv_disconnect(struct online_user &user);
void v5_send_ack(Packet &pack);
void v5_send_login_err(Packet &pack, char *errmessage);
void v5_send_test_err(Packet &pack, unsigned short commd, char *errmessage);
void v5_send_pass_err(Packet &pack);
void v5_send_login_reply(Packet &pack, struct login_user_info &userinfo, struct online_user &user);
void v5_send_not_connected(Packet &pack);
void v5_send_end_contact(struct online_user &user);
void v5_send_user_online(struct online_user &to_user, struct online_user &user);
void v5_send_user_offline(struct online_user &to_user, unsigned long uin);
void v5_send_user_status(struct online_user &to_user, struct online_user &user);
void v5_send_end_sysmsg(struct online_user &user);
void v5_send_lmeta(struct online_user &user, unsigned short seq2);
int  v5_send_user_message(struct msg_header &msg_hdr, struct online_user &to_user, char *message);
int  v5_send_user_sysmsg(struct msg_header &msg_hdr, struct online_user &to_user, char *message);
int  v5_send_offline_messages(struct online_user &user);
void v5_send_old_style_info(struct online_user &user, struct full_user_info &tuser);
void v5_send_old_style_info_ext(struct online_user &user, struct full_user_info &tuser, struct notes_user_info &notes);
void v5_send_invalid_uin(struct online_user &user, unsigned long uin_num);
void v5_send_ack_ext(Packet &pack, unsigned long xx1, BOOL isOk);

/* meta-reply packets */
void v5_prepare_meta_fail(unsigned short command, unsigned short seq2, struct online_user &user);
void v5_send_meta_info(unsigned short seq2, struct online_user &user, struct full_user_info &tuser, BOOL success);
void v5_send_meta_info2(unsigned short seq2, struct online_user &user, struct full_user_info &tuser, BOOL success);
void v5_send_meta_info3(unsigned short seq2, struct online_user &user, struct full_user_info &tuser, BOOL success);
void v5_send_meta_more(unsigned short seq2, struct online_user &user, struct full_user_info &tuser, BOOL success);
void v5_send_meta_more2(unsigned short seq2, struct online_user &user, struct full_user_info &tuser, BOOL success);
void v5_send_meta_work(unsigned short seq2, struct online_user &user, struct full_user_info &tuser, BOOL success);
void v5_send_meta_work2(unsigned short seq2, struct online_user &user, struct full_user_info &tuser, BOOL success);
void v5_send_meta_about(unsigned short seq2, struct online_user &user, struct notes_user_info &tuser, BOOL success);
void v5_send_user_found(unsigned short seq2, struct online_user &user, struct full_user_info &tuser, BOOL last, BOOL success, unsigned long users_left);
void v5_send_user_found2(unsigned short seq2, struct online_user &user, struct full_user_info &tuser, BOOL last, BOOL success, unsigned long users_left);
void v5_send_white_user_found(unsigned short seq2, struct online_user &user, struct full_user_info &tuser, BOOL last, BOOL success, unsigned long users_left);
void v5_send_white_user_found2(unsigned short seq2, struct online_user &user, struct full_user_info &tuser, BOOL last, BOOL success, unsigned long users_left);
void v5_send_old_search_found(unsigned short seq2, struct online_user &user, struct full_user_info &tuser);
void v5_send_old_search_end(unsigned short seq2, struct online_user &user, BOOL more);

void v5_send_meta_set_ack(unsigned short seq2, struct online_user &user, unsigned short command, BOOL success);
void v5_send_depslist(unsigned short seq2, struct online_user &user);
void v5_send_new_uin(Packet &pack, unsigned long uin_num, unsigned long xx1);

void v5_reply_metainfo_request(struct online_user &user, unsigned long target_uin, unsigned short seq2);
void v5_reply_metainfo2_request(struct online_user &user, unsigned long target_uin, unsigned short seq2);
void v5_reply_metafullinfo_request(struct online_user &user, unsigned long target_uin, unsigned short seq2);
void v5_reply_metafullinfo_request2(struct online_user &user, unsigned long target_uin, unsigned short seq2);

void v5_search_by_uin(struct online_user &user, unsigned long target_uin, unsigned short seq2);
void v5_search_by_uin2(struct online_user &user, unsigned long target_uin, unsigned short seq2);
void v5_search_by_email(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_search_by_email2(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_search_by_name(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_search_by_name2(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_search_by_white(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_search_by_white2(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_old_search(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_old_search_uin(unsigned long tuin, struct online_user &user, unsigned short seq2);

void v5_send_meta_hpage_cat(unsigned short seq2, struct online_user &user, struct full_user_info &tuser, BOOL success);
void v5_send_meta_interestsinfo(unsigned short seq2, struct online_user &user, struct full_user_info &tuser, BOOL success);
void v5_send_meta_affilationsinfo(unsigned short seq2, struct online_user &user, struct full_user_info &tuser, BOOL success);

void v5_set_basic_info(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_set_basic_info2(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_set_work_info(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_set_work_info2(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_set_about_info(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_set_secure_info(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_set_more_info(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_set_more_info2(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_set_password(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_set_hpcat_info(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_set_interests_info(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_set_affilations_info(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_unregister_user(Packet &pack, struct online_user &user, unsigned short seq2);
void v5_disconnect_user(struct online_user *user);

void v5_process_login();
void v5_process_contact();
void v5_process_sysmsg();
void v5_process_visible_list();
void v5_process_invisible_list(); 
void v5_process_change_vilists();
void v5_process_ack();
void v5_process_logoff();
void v5_process_ping();
void v5_process_user_meta();
void v5_process_status();
void v5_process_old_search();
void v5_process_old_srchuin();
void v5_process_useradd();
void v5_process_firstlog();
void v5_process_getdeps();
void v5_process_sysmsg_delete();
void v5_process_old_info();
void v5_process_old_info_ext();
void v5_process_ack_new_uin();

BOOL v5_extract_string(char *dst_str, Packet &spack, int max_len, char *fname, struct online_user &user);
unsigned short convert_message_type(unsigned short type);
char *convert_message_text(unsigned long from_uin, unsigned short type, char *message);

#endif /* _V5_PROTO_H */

