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
/*                                                                        */
/**************************************************************************/

#ifndef _V7_PROTO_H
#define _V7_PROTO_H

#include "snac_families/snac_protos.h"

void flap_send_packet(Packet &pack);
void flap_recv_packet(int index, Packet &pack);
void flap_init();
void handle_aim_proto(Packet &pack);
void send_connection_accepted(Packet &pack);
void log_unknown_packet(Packet &pack);
void decrypt_password(char *password, unsigned short passlen);
void get_bos_server(char *server_addr);
unsigned short v7_convert_message_type(unsigned short type);
char *v7_convert_message_text(unsigned long from_uin, unsigned short type, char *message);
BOOL user_has_cap(struct online_user *to_user, char *required_cap);
BOOL caps_match(char *caps1, char *caps2);
BOOL isIcq(char *profile);
void v7_proto_init();
void init_aim_services();
unsigned long read_buin(Packet &pack);
unsigned short tlv_read_string(Packet &pack, char *string, int max);
void tlv_read_short(Packet &pack, unsigned short &dest);
void tlv_read_long(Packet &pack, unsigned long &dest);
BOOL v7_extract_string(char *dst_str, class tlv_c &spack, int max_len, char *fname, struct online_user &user, Packet &pack);
BOOL v7_extract_string(char *dst_str, class tlv_c &spack, int max_len, char *fname, Packet &pack);
BOOL v7_extract_string(char *dst_str, class tlv_c &spack, int max_len);
BOOL v7_extract_string(char *dst_str, Packet &spack, int max_len, char *fname);
void v7_disconnect_user(struct online_user *user);

void pcopy(Packet &dpack, Packet &spack);
void close_connection(Packet &pack);
void close_connection(struct online_user *user);
void process_authorize_packet(Packet &pack);
void process_cookie_packet(Packet &pack);
void process_snac_packet(Packet &pack);
void send_authorize_fail(Packet &pack, char *error_text, char *scr_name, unsigned short error_code);
void send_session_fail(Packet &pack, char *error_text, unsigned short error_code);
void send_auth_cookie(Packet &pack, char *srv_addr, char *screen_name, char *cookie, unsigned short cookie_len);
void send_srv_families(Packet &pack, struct online_user &user);
void send_srv_motd(struct snac_header &snac, Packet &pack);
void send_srv_pause(struct online_user *user);
void send_srv_migrate(struct online_user *user, char *srv_addr, char *cookie, unsigned short cookie_len, unsigned short *families, unsigned short fam_number);
void send_snac_error(unsigned short family, unsigned short errcode, unsigned long reqid, Packet &pack);
void send_snac_error(unsigned short family, unsigned short errcode, unsigned long reqid, struct online_user *user);

void generate_cookie(Packet &pack, char *result, char *screen_name);
void generate_key(char *result);
BOOL aim_check_digest(char *md5_digest, unsigned short md5_method, char *passwd, char *authkey);

void v7_send_srv_disconnect(struct online_user &user, unsigned short err_code, char *err_string);

#endif /* _V7_PROTO_H */

