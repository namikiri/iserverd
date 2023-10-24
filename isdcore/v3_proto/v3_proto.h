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

#ifndef _V3_PROTO_H
#define _V3_PROTO_H

void handle_v3_proto(Packet &pack);

/* packets function */
void v3_proto_init();
void v3_process_login();
void v3_process_ack();
void v3_send_ack(Packet &pack);
void v3_send_login_err(Packet &pack, char *errmessage);
void v3_send_end_contact(struct online_user &user);
void v3_send_pass_err(Packet &pack);
void v3_send_not_connected(Packet &pack);
void v3_send_user_online(struct online_user &to_user, struct online_user &user);
void v3_send_user_offline(struct online_user &to_user, unsigned long uin);
void v3_send_user_status(struct online_user &to_user, struct online_user &user);
void v3_send_busy(unsigned long uin_num, unsigned short seq2, struct in_addr from_ip, unsigned short from_port);
void v3_send_login_reply(Packet &pack, struct login_user_info &userinfo, struct online_user &user);
int  v3_send_indirect(Packet &pack, unsigned long to_uin, unsigned long shm_index);
void v3_send_oob(unsigned long uin_num);
void v3_send_notes(Packet &pack, struct online_user &user, struct notes_user_info notes);
void v3_send_invalid_user(Packet &pack, struct online_user &user);
void v3_send_basic_info(Packet &pack, struct online_user &user, struct full_user_info &tuser);
void v3_send_basic_info_single(Packet &pack, struct online_user &user, struct full_user_info &tuser);
void v3_send_home_info(Packet &pack, struct online_user &user, struct full_user_info &tuser);
void v3_send_home_web(Packet &pack, struct online_user &user, struct full_user_info &tuser);
void v3_send_work_info(Packet &pack, struct online_user &user, struct full_user_info &tuser); 
void v3_send_work_web(Packet &pack, struct online_user &user, struct full_user_info &tuser);
void v3_send_depslist(Packet &pack, struct online_user &tuser, unsigned short servseq);
void v3_send_depslist1(Packet &pack, struct online_user &tuser, unsigned short servseq);
void v3_send_reply_ok(Packet &pack, struct online_user &user, unsigned short command);
void v3_send_found_info(Packet &pack, struct online_user &user, struct found_info &tuser);
void v3_send_search_finished(Packet &pack, struct online_user &user, int more);
int  v3_send_user_message(struct msg_header &msg_hdr, struct online_user &to_user, char *message);
int  v3_send_user_sysmsg(struct msg_header &msg_hdr, struct online_user &to_user, char *message);
int  v3_send_packet_in_fragments(Packet &pack, unsigned long to_uin, unsigned long shm_index);
void v3_send_fragment(Packet &pack, unsigned long to_uin, unsigned short seq1, unsigned short seq2, int part_num, int part_cnt, unsigned long temp_stamp, char *buffer, int len, unsigned long shm_index);
void v3_send_registration_info(Packet &int_pack, unsigned short seq2);
void v3_send_registration_ok(Packet &pack, unsigned short seq1, unsigned short seq2, struct full_user_info &new_user);
void v3_send_srv_disconnect(struct online_user &user, char *errmessage);
void PutSeq3(Packet &pack, unsigned short cc);
void v3_disconnect_user(struct online_user *user);

/* Handlers */
void v3_process_logoff();
void v3_process_contact();
void v3_process_useradd();
void v3_process_ping();
void v3_process_status();
void v3_process_notes();
void v3_process_getinfo();
void v3_process_getinfo1();
void v3_process_getdeps();
void v3_process_firstlog();
void v3_process_getext();
void v3_process_getdeps1();
void v3_process_onlineinfo();
void v3_process_usagestats();
void v3_process_setnotes();
void v3_process_setweb(int htype);
void v3_process_setbasic();
void v3_process_sethome();
void v3_process_setwork();
void v3_process_unknow_dep();
void v3_process_sysmsg_req();
void v3_process_setpass();
void v3_process_setauth();
void v3_process_search();
void v3_process_search();
void v3_process_sysmsg();
void v3_process_sysack();
void v3_process_state();
void v3_defrag_packet();
void v3_process_reginfo_req();
void v3_process_reg_newuser();
void v3_process_broadcast();
void v3_process_wwp();
void v3_process_deluser_req();

unsigned short v3_convert_message_type(unsigned short type);

#endif /* _V3_PROTO_H */

