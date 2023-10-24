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
/* This module contain snac creating/processing functions prototypes      */
/*                                                                        */
/**************************************************************************/

#ifndef _SNAC_PROTOS_H_
#define _SNAC_PROTOS_H_

void process_snac_generic(struct snac_header &snac, Packet &pack);
void process_snac_location(struct snac_header &snac, Packet &pack);
void process_snac_buddylist(struct snac_header &snac, Packet &pack);
void process_snac_messaging(struct snac_header &snac, Packet &pack);
void process_snac_advert(struct snac_header &snac, Packet &pack);
void process_snac_invitation(struct snac_header &snac, Packet &pack);
void process_snac_admin(struct snac_header &snac, Packet &pack);
void process_snac_popup(struct snac_header &snac, Packet &pack);
void process_snac_bos(struct snac_header &snac, Packet &pack);
void process_snac_search(struct snac_header &snac, Packet &pack);
void process_snac_stats(struct snac_header &snac, Packet &pack);
void process_snac_translate(struct snac_header &snac, Packet &pack);
void process_snac_chat_navig(struct snac_header &snac, Packet &pack);
void process_snac_chat(struct snac_header &snac, Packet &pack);
void process_snac_ssi(struct snac_header &snac, Packet &pack);
void process_snac_ext_messages(struct snac_header &snac, Packet &pack);
void process_snac_registration(struct snac_header &snac, Packet &pack);

void process_loc_get_info2(struct snac_header &snac, Packet &pack);
void process_loc_get_info(struct snac_header &snac, Packet &pack);
void process_loc_set_info(struct snac_header &snac, Packet &pack);
void send_user_loc_info(struct snac_header &snac, struct online_user *user, struct online_user *auser, unsigned long datamask);
void process_blm_add_contact(struct snac_header &snac, Packet &pack);
void process_blm_del_contact(struct snac_header &snac, Packet &pack);

void process_snac_basic(Packet &pack, struct snac_header snac);
void process_gen_req_versions(struct snac_header &snac, Packet &pack);
void process_gen_req_rates(struct snac_header &snac, Packet &pack);
void process_gen_ack_rates(struct snac_header &snac, Packet &pack);
void process_gen_set_status(struct snac_header &snac, Packet &pack);
void process_gen_get_info(struct snac_header &snac, Packet &pack);
void process_gen_set_idletime(struct snac_header &snac, Packet &pack);
void process_gen_service_req(struct snac_header &snac, Packet &pack);
void process_bos_get_rights(struct snac_header &snac, Packet &pack);
void process_bos_del_viscontact(struct snac_header &snac, Packet &pack);
void process_bos_add_viscontact(struct snac_header &snac, Packet &pack);
void process_bos_del_inviscontact(struct snac_header &snac, Packet &pack);
void process_bos_add_inviscontact(struct snac_header &snac, Packet &pack);

void process_blm_req_rights(struct snac_header &snac, Packet &pack);
void process_msg_req_rights(struct snac_header &snac, Packet &pack);
void process_msg_set_rights(struct snac_header &snac, Packet &pack);
void process_msg_send(struct snac_header &snac, Packet &pack);
void process_msg_ack(struct snac_header &snac, Packet &pack);
void process_txt_message(Packet &pack, struct snac_header &snac, struct online_user *user, struct msg_header &msg_hdr);
void process_adv_message(Packet &pack, struct snac_header &snac, struct online_user *user, struct msg_header &msg_hdr);
void process_sys_message(Packet &pack, struct snac_header &snac, struct online_user *user, struct msg_header &msg_hdr);

void process_loc_get_rights(struct snac_header &snac, Packet &pack);
void process_ime_multi_req(struct snac_header &snac, Packet &pack);
void send_status_info(struct snac_header &snac, Packet &pack, struct online_user *user);
void send_status_info2(struct online_user *to_user, struct online_user *user);
void process_gen_cli_ready(struct snac_header &snac, Packet &pack);

void process_mr_information_request(Packet &pack, struct online_user *user, struct snac_header &snac, struct tlv_c *tlv);
void process_mr_delete_messages(Packet &pack, struct online_user *user, struct snac_header &snac);
void process_mr_messages_request(Packet &pack, struct online_user *user, struct snac_header &snac, struct tlv_c *tlv);
void process_mr_xml_request(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, class tlv_c &tlv);
void process_mr_sms_request(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, class tlv_c &tlv);

void mr_search_by_email(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, class tlv_c &tlv);
void mr_search_by_email2(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, class tlv_c &tlv);
void mr_search_by_uin2(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, class tlv_c &tlv);
void mr_search_by_details(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, class tlv_c &tlv);
void mr_search_white(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, class tlv_c &tlv);
void mr_search_white2(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, class tlv_c &tlv);
void mr_unregister_user(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, class tlv_c &tlv);
void mr_search_random(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, class tlv_c &tlv);
void mr_search_info_req(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, class tlv_c &tlv);

void mr_set_user_home_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, class tlv_c &tlv);
void mr_set_user_work_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, class tlv_c &tlv);
void mr_set_user_more_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, class tlv_c &tlv);
void mr_set_user_pass_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, class tlv_c &tlv);
void mr_set_user_notes_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, class tlv_c &tlv);
void mr_set_user_perms_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, class tlv_c &tlv);
void mr_set_user_email_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, class tlv_c &tlv);
void mr_set_user_interests_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, class tlv_c &tlv);
void mr_set_user_affilations_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, class tlv_c &tlv);
void mr_set_user_hpcat_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, class tlv_c &tlv);
void mr_set_user_icqphone_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, class tlv_c &tlv);

void mr_send_home_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, unsigned short success, unsigned short flags);
void mr_send_short_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, unsigned short success, unsigned short flags);
void mr_send_set_ack(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, unsigned short sub_cmd, unsigned short success, unsigned short flags);
void mr_send_more_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, unsigned short success, unsigned short flags);
void mr_send_email_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, unsigned short success, unsigned short flags);
void mr_send_hpage_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, unsigned short success, unsigned short flags);
void mr_send_work_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, unsigned short success, unsigned short flags);
void mr_send_about_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, unsigned long to_uin, unsigned short flags);
void mr_send_interests_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, unsigned short success, unsigned short flags);
void mr_send_affilations_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, unsigned short success, unsigned short flags);
void mr_send_wp_found(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, unsigned short success, unsigned short flags, unsigned long last, unsigned long users_left);
void mr_send_wp_found2(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, unsigned short success, unsigned short flags, unsigned long last, unsigned long users_left);
void mr_send_offline_message(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, unsigned short flags, struct msg_header &msg_hdr, char *message);
void mr_send_utf8_found(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, unsigned short success, unsigned short flags, unsigned long last, unsigned long users_left, char status, unsigned short req_type);

void mr_send_end_of_messages(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, unsigned short flags);
void mr_send_xml_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, unsigned short success, char *xml_str, unsigned short flags);
void mr_send_blm_err(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, char errcode, char *errdesc);
void mr_set_user_info(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, class tlv_c &tlv);
void mr_utf8_set_req(Packet &pack, struct online_user *user, struct snac_header &snac, unsigned short req_seq, struct full_user_info &finfo, class tlv_c &tlv);


int v7_send_user_message(struct msg_header &msg_hdr, struct online_user &to_user, char *message);
int v7_send_user_message_x1(struct msg_header &msg_hdr, struct online_user &to_user, char *message);
int v7_send_user_message_x2(struct msg_header &msg_hdr, struct online_user *to_user, struct online_user *fuser, char *required_cap, class tlv_c *tlv2711);
int v7_send_user_message_x2(struct msg_header &msg_hdr, struct online_user *to_user, struct online_user *fuser, char *required_cap, class tlv_chain_c &tlv_chain2, BOOL ack_flag);

int v7_send_user_message_x4(struct msg_header &msg_hdr, struct online_user &to_user, char *message);
int v7_send_user_message_x4e(struct msg_header &msg_hdr, struct online_user &to_user, char *message, char *rest_data, unsigned short rest_size);
void send_missed_message(struct online_user *to_user, struct online_user *from_user, struct msg_header &msg_hdr, unsigned short missed_num, unsigned short reason);
void send_missed_message(struct online_user *from_user, struct msg_header &msg_hdr, unsigned short missed_num, unsigned short reason);
unsigned short msgext_validate_chain(class tlv_chain_c &tlv_chain2, struct msg_header &msg_hdr, Packet &pack);

void v7_send_user_offline(struct online_user &to_user, unsigned long uin);
void v7_send_user_online(struct online_user &to_user, struct online_user &user);
void v7_send_user_status(struct online_user &to_user, struct online_user &user);
void v7_send_new_uin(Packet &pack, unsigned long new_uin, unsigned long req_cookie);
void v7_send_reg_refused(Packet &pack, unsigned long req_cookie);
void v7_send_authkey(Packet &pack, char *authkey);

void stats_send_interval(struct online_user *to_user);
void send_type2_ack(struct online_user *user, struct snac_header &snac, struct msg_header &msg_hdr, unsigned long channel);

void process_ies_uin_req(struct snac_header &snac, Packet &pack);
void process_ies_auth_req(struct snac_header &snac, Packet &pack);
void process_ies_auth_login(struct snac_header &snac, Packet &pack);
void send_authmd5_fail(Packet &pack, unsigned short errcode, char *url);
void send_authmd5_cookie(Packet &pack, char *screen_name, char* bos_address, char *server_cookie, unsigned short cookie_length);
void process_ies_image_req(struct snac_header &snac, Packet &pack);

void process_ssi_get_rights(struct snac_header &snac, Packet &pack, struct online_user *user);
void process_ssi_request(struct snac_header &snac, Packet &pack, struct online_user *user);
void process_ssi_checkout(struct snac_header &snac, Packet &pack, struct online_user *user);
void process_ssi_activate(struct snac_header &snac, Packet &pack, struct online_user *user);
void change_ssi(unsigned short action_code, struct snac_header &snac, Packet &pack, struct online_user *user);
void process_ssi_auth_grant(struct snac_header &snac, Packet &pack, struct online_user *user);
void process_ssi_auth_req(struct snac_header &snac, Packet &pack, struct online_user *user);
void process_ssi_auth_rep(struct snac_header &snac, Packet &pack, struct online_user *user);
void process_ssi_ch_begin(struct snac_header &snac, Packet &pack, struct online_user *user);
void process_ssi_ch_end(struct snac_header &snac, Packet &pack, struct online_user *user);
void user_send_ssi_data(struct snac_header &snac, struct online_user *user);
void user_ssi_activate(struct online_user *user);
void user_ssi_get_mod_info(struct online_user *user, unsigned long &mod_date, unsigned short &item_num);
void user_ssi_update_mod_info(struct online_user *user, BOOL import);
void send_ssi_update_ack(struct snac_header &snac, struct online_user *user, unsigned short result_code);
void user_send_presense_part(struct online_user *user, unsigned long rid);
void user_send_presense_full(struct online_user *user);
void grant_ssi_authorization(unsigned long from_uin, unsigned long to_uin);
void send_ssi_added(unsigned long uin, unsigned long by_uin);
void process_msg_mtn(struct snac_header &snac, Packet &pack);
void ssi_send_auth_granted(unsigned long from_uin, struct online_user *to_user);
void send_item_auth_update(unsigned long from_uin, struct online_user *to_user);
void ssi_send_you_added(unsigned long from_uin, struct online_user *to_user);
void ssi_send_auth_req(unsigned long from_uin, struct online_user *to_user, char *message);
void ssi_send_auth_denied(unsigned long from_uin, struct online_user *to_user, char *message);
void process_ies_securid(struct snac_header &snac, Packet &pack);

#endif /* _SNAC_PROTOS_H_ */

