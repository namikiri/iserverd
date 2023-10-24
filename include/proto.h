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

#ifndef _PROTO_H_
#define _PROTO_H_

#ifdef __cplusplus
/* prototypes from netlib/tcpserver.cpp */
int aim_tcp_server_start(int port_number);
int msn_tcp_server_start(int port_number);
void tcp_receive_packet(int index, Packet &pack);
BOOL ready_data(int sock_hdl);
int wait4write(int sock_hdl, int time);
int wait4read(int sock_hdl, int time);
#endif

/* prototypes from config/actions/ .cpp */
#include "config/actions/actions_proto.h"

#ifdef __cplusplus
/* prototypes from system/util_shm.cpp */
BOOL lock_user_record(struct online_user &temp_user);
BOOL unlock_user_record(struct online_user &temp_user);
BOOL shm_add(struct online_user &user);
BOOL shm_activate_user(unsigned long uin);
BOOL shm_lookup(unsigned long to_uin, struct online_user &temp_user);
BOOL shm_delete(unsigned long to_uin);
BOOL shm_update_sseq(struct online_user &temp_user);
BOOL shm_add_servseq(unsigned long uin, int number);
BOOL shm_setstatus(struct online_user &temp_user);
BOOL shm_setstate(struct online_user &temp_user, int state);
BOOL shm_touch(struct online_user &temp_user);
struct online_user *shm_get_user(unsigned long uin);
struct online_user *shm_iget_user(unsigned long uin, unsigned long shm_index);
struct online_user *shm_get_user(unsigned long sock_hdl, unsigned long sock_rnd);

unsigned long shm_get_ip(unsigned long uin);
void shm_user_update(unsigned long uin, unsigned short ttl);
BOOL shm_user_exist(unsigned long uin);

/* prototypes from system/util_pipe.cpp */
void actions_send_packet(Packet &pack);
int  actions_receive_packet(Packet &pack);
int  pipe_send_packet(Packet &pack);
int  toutgoing_receive_packet(Packet &pack);
void tcp_writeback_packet(Packet &pack);
void pipe_receive_packet(Packet &pack);
void epacket_receive_packet(indirect_pack &inpack);
void epacket_receive_event(event_pack &inpack);
void euser_receive_event(event_pack &inpack);
void defrag_receive_pipe(defrag_pack &inpack);
void event_send_packet(Packet &pack, unsigned long to_uin, unsigned long shm_index, unsigned long retry_num, unsigned long rtimeout, unsigned long ack_timestamp);
void epacket_send_event(event_pack &pack);
void euser_send_event(event_pack &pack);
void defrag_send_pipe(defrag_pack &pack);
void incoming_pipe_init();
void actions_pipe_init();
void wwp_socket_init();
void outgoing_pipe_init();
void toutgoing_pipe_init();
void epacket_pipe_init();
void euser_pipe_init();
void defrag_pipe_init();

/* prototypes from event/euser.cpp */
void process_actions();
void process_euser();
void euser_process_event(event_pack &inpack);
void online_cache_update(unsigned long uin, unsigned short ttl);
unsigned long cache_ip_lookup(unsigned long uin);
void online_cache_decrement();
void move_user_online(struct online_user &user);
void move_user_offline(unsigned long uin);
void send_offline2cache(unsigned long uin);
void send_online2cache(unsigned long uin, unsigned short ttl, unsigned long ip);
void send_update2cache(unsigned long uin, unsigned short ttl);

/* prototypes from event/etimer.cpp */
void process_etimer();
RETSIGTYPE etimerSIGALRMHandler(int param);
void send_timeout2cache();
void send_timeout2epacket();

/* prototypes from event/epacket.cpp */
void process_epacket();
void epacket_process_packet(indirect_pack &inpack);
void epacket_process_event(event_pack &inpack);
void init_packets_list();
void add_packet_to_list(indirect_pack &inpack, struct online_user *user);
void remove_packet_from_list(unsigned long ack_uin, unsigned long ack_stamp);
void check_ack_list();
struct usr_queue_s *create_user_queue(struct online_user *user, indirect_pack &inpack);
void delete_user_queue(struct usr_queue_s *uqueue);
void delete_user_pqueue(struct usr_queue_s *uqueue);
void user_queue_add_packet(struct usr_queue_s *uqueue, indirect_pack &inpack);
void user_queue_del_packet(struct usr_queue_s *uqueue, unsigned long ack_stamp);
void user_pqueue_check(struct usr_queue_s *uqueue);
void packet_expire(struct usr_queue_s *uqueue, struct ipack_s *pack);
unsigned long generate_ack_number(unsigned long puin, unsigned long pseq1, unsigned long pseq2);
void process_ack_event(unsigned long puin, unsigned long pstamp);

/* prototypes from event/scheduler.cpp */
void scheduler_init();
void scheduler_timer();
void call_scheduler_vacuum();
void call_scheduler_cvalid();
void call_defrag_check();
void call_online_check();

/* prototypes from isdcore/handle.cpp */
void disconnect_user(struct online_user *user);
void handle_icq_packet(Packet &pack);
void init_all_protocols();
int get_ping_time(int protocol);
void send_user_online(struct online_user &to_user, struct online_user &user);
void send_user_offline(struct online_user &to_user, unsigned long uin);
void send_user_status(struct online_user &to_user, struct online_user &user);
void send_online_message(struct msg_header &msg_hdr, struct online_user &to_user, char *message);
void send_user_disconnect(struct online_user &user, unsigned short err_code, char *err_string);
void process_message(struct msg_header &msg_hdr, char *message);

/* prototypes from config/loadparm.cpp */
void standard_sub_basic(char *str);
void subst_post_register(char *str, unsigned long uin, char* passwd);
void init_globals(void);
void lp_talloc_free(void);
/* static char *lp_string(const char *s); */
char * lp_config_file();
char * lp_admin_email();
char * lp_dbglog_path();
char * lp_translate_file();
char * lp_translate_tbl();
char * lp_actions_conf();
char * lp_wwp_socketname();
char * lp_v7_proto_config();
char * lp_v7_bos_address();
char * lp_v7_table_charset();

int lp_v7_dmax_channel();
int lp_v7_dmax_msgsize();
int lp_v7_dmax_sevil();
int lp_v7_dmax_revil();
int lp_v7_dmin_msg_interval();
int lp_max_tcp_connections();

int lp_v7_max_ssi_ignore();
int lp_v7_max_ssi_groups();
int lp_v7_max_ssi_nonicq();
int lp_v7_max_ssi_avatars();
BOOL lp_v7_create_groups();
BOOL lp_v7_enable_ssi_import();

BOOL   lp_timestamp_logs();
BOOL   lp_append_logs();
BOOL   lp_pid_in_logs();
BOOL   lp_bind_all_ifaces();
BOOL   lp_realtime_odb();
BOOL   lp_proclist();
BOOL   lp_restrict2luip();

int lp_umask();
int lp_lperms();

int lp_max_childs();
int lp_min_childs();

int lp_defrag_check();
int lp_online_check();

char * lp_var_path();
char * lp_pid_path();
char * lp_lock_text_file();
char * lp_db_user();
char * lp_db_pass();
char * lp_interface();
char * lp_info_passwd();

int lp_log_size();
int lp_shm_size();
int lp_udp_port();
int lp_aim_port();
int lp_msn_port();
int lp_log_level();
int lp_server_mode();

char *lp_db_user();
char *lp_db_pass();
char *lp_db_addr();
char *lp_db_port();
char *lp_db_users();

int lp_sched_vacuum();
int lp_sched_cvalid();

BOOL lp_watchdog_enabled();
BOOL lp_degradated_mode();
BOOL lp_actions_enabled();
int  lp_watchdog_timeout();

BOOL lp_v5_enabled();
BOOL lp_v5_registration_enabled();
BOOL lp_v5_autoregister();

int  lp_v5_retries();
int  lp_v5_timeout();
int  lp_v5_pingtime();
int  lp_v5_maxsearch();
int  lp_v5_max_msgsize();
int  lp_v3_split_order();

BOOL lp_v3_enabled();
BOOL lp_v3_registration_enabled();
BOOL lp_v3_autoregister();

BOOL lp_v7_enabled();
BOOL lp_v7_registration_enabled();
BOOL lp_v7_accept_concurent();
BOOL lp_v7_direct_v5_connect();
BOOL lp_v7_direct_v3_connect();
int lp_v7_conn_timeout();
int lp_v7_cookie_timeout();
int lp_v7_maxsearch();
int lp_v7_max_contact_size();
int lp_v7_max_proflen();
int lp_v7_max_watchers_size();
int lp_v7_max_visible_size();
int lp_v7_max_invisible_size();

char *lp_v3_admin_notes();
char *lp_v3_post_registration();
int  lp_v3_retries();
int  lp_v3_timeout();
int  lp_v3_pingtime();
int  lp_v3_maxsearch();
int  lp_v3_max_msgsize();
int  lp_v3_packet_mtu();
int  lp_default_ping();
int  lp_deplist_vers();
int  lp_externals_num();

int lp_add_service(char *pszService, int iDefaultService);
int strwicmp(char *psz1, char *psz2);
int map_parameter(char *pszParmName);
BOOL set_boolean(BOOL *pb, char *pszParmValue);
BOOL service_ok(int iService);
void add_to_file_list(char *fname);
BOOL lp_file_list_changed(void);
BOOL source_env(char **lines);
void *lp_local_ptr(int snum, void *ptr);
BOOL lp_do_parameter(int snum, char *pszParmName, char *pszParmValue);
BOOL do_parameter(char *pszParmName, char *pszParmValue);
void print_parameter(struct parm_struct *p, void *ptr, FILE * f);
struct parm_struct *lp_next_parameter(int snum, int *i, int allparameters);
BOOL lp_snum_ok(int iService);
BOOL lp_loaded(void);
BOOL lp_load(char *pszFname, BOOL global_only, BOOL save_defaults,
	     BOOL add_ipc);
void lp_resetnumservices(void);
int lp_numservices(void);
void lp_remove_service(int snum);
BOOL handle_include(char *pszParmValue, char **ptr);
BOOL handle_v3_adm_notes(char *pszParmValue, char **ptr);
BOOL handle_translate(char *pszParmValue, char **ptr);
BOOL handle_v3_post_reg(char *pszParmValue, char **ptr);
void process_command_line_opt(int argc, char **argv);

/* prototypes from system/util_str.cpp */
char *n2str(unsigned long num);
int count_subs(char *s,const char *pattern);
int hexencode(char *rstr, char *buf, int buf_size);
int hexdecode(char *buf, char *sstr, int buflen);
BOOL string_init(char **dest,const char *src);
void set_ps_display(int process_role, const char *value);
void init_ps_display(int argc, char *argv[]);
#endif

#ifdef __cplusplus
extern "C" {
#endif
unsigned long atoul(char *string);
unsigned long ahextoul(char *string);
#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
BOOL islen_valid(int str_len, int limit, char *fname, struct online_user &user);
BOOL islen_valid(int str_len, int limit, char *fname, Packet &pack);
void convert_to_postgres(char *target_string, int tsize, char *string);
void convert_to_postgres2(char *target_string, int tsize, char *string);
void convert_to_postgres(char *string, int tsize);
void convert_to_unicode(char *string, int tsize);
void convert_from_unicode(char *string, int tsize);
void set_first_token(char *ptr);
BOOL next_token(char **ptr,char *buff,char *sep, size_t bufsize);
char **toktocliplist(int *ctok, char *sep);
int StrCaseCmp(const char *s, const char *t);
int StrnCaseCmp(const char *s, const char *t, size_t n);
BOOL strequal(const char *s1, const char *s2);
BOOL strnequal(const char *s1,const char *s2,size_t n);
BOOL strcsequal(const char *s1,const char *s2);
void strlower(char *s);
void strupper(char *s);
void strnorm(char *s);
BOOL strisnormal(char *s);
void string_replace(char *s,char oldc,char newc);
char *skip_string(char *buf,size_t n);
size_t str_charnum(const char *s);
BOOL trim_string(char *s,const char *front,const char *back);
BOOL strhasupper(const char *s);
BOOL strhaslower(const char *s);
size_t count_chars(const char *s,char c);
BOOL str_is_all(const char *s,char c);
char *safe_strcpy(char *dest,const char *src, size_t maxlength);
char *safe_strcat(char *dest, const char *src, size_t maxlength);
char *alpha_strcpy(char *dest, const char *src, size_t maxlength);
char *StrnCpy(char *dest,const char *src,size_t n);
char *strncpyn(char *dest, const char *src,size_t n, char c);
size_t strhex_to_str(char *p, size_t len, const char *strhex);
void strhex_to_arr(char *p, int len, const char *strhex);
BOOL in_list(char *s,char *list,BOOL casesensitive);
void string_free(char **s);
BOOL string_set(char **dest,const char *src);
void string_sub(char *s,const char *pattern,const char *insert, size_t len);
void fstring_sub(char *s,const char *pattern,const char *insert);
void pstring_sub(char *s,const char *pattern,const char *insert);
void all_string_sub(char *s,const char *pattern,const char *insert, size_t len);
void split_at_first_component(char *path, char *front, char sep, char *back);
void split_at_last_component(char *path, char *front, char sep, char *back);
char *octal_string(int i);
char *string_truncate(char *s, int length);
/* prototypes from system/talloc.c */

TALLOC_CTX *talloc_init(void);
void *talloc(TALLOC_CTX *t, size_t size);
void talloc_destroy(TALLOC_CTX *t);

/* prototypes from system/lock.c */

int                  Lock(FILE *fp);
int                  Unlock(FILE *fp);

/* prototypes from system/signals.c */

void (*rsignal(int signo, void (*hndlr)(int)))(int);
void block_signal(int signo);
void unblock_signal(int signo);

RETSIGTYPE      mySIGHUPHandler(int param);
RETSIGTYPE      mySIGALRMHandler(int param);
RETSIGTYPE      mySIGINTHandler(int param);
RETSIGTYPE      mySIGCHLDHandler(int param);
RETSIGTYPE      mySIGSEGVHandler(int param);
RETSIGTYPE      childSIGINTHandler(int param);
RETSIGTYPE      myAPSIGCHLDHandler(int param);

/* prototypes from icqlog/log_debug.cpp */

void alr_dump_data(int level, const char *buf1, int len);
void alr_print_asc(int level, uchar const *buf, int len);
void dbg_dump_data(int level, const char *buf1, int len);
void dbg_print_asc(int level, uchar const *buf, int len);
BOOL dbg_interactive(void);
void setup_logging( char *pname, BOOL interactive );
void reopen_logs( void );
void force_check_log_size( void );
BOOL need_to_check_log_size( void );
void check_log_size( void );
int Debug1( char *format_str, ... );
void bufr_print( void );
void format_debug_text( char *msg );
void dbgflush( void );
BOOL dbghdr( int level, char *file, char *func, int line );
BOOL dbgtext( char *format_str, ... );
void log_debug_packet(int level, Packet &pack);

/* prototypes from  icqlog/log_alr.c */

BOOL alr_interactive(void);
void setup_alrlogging( char *pname, BOOL interactive );
void init_syslog_logs( void );
int alrDebug1( char *format_str, ... );
void alrbufr_print( void );
void format_alarm_text( char *msg );
BOOL alrtext( char *format_str, ... );
void log_alarm_packet(int level, Packet &pack);

/* prototypes from  icqlog/log_sys.c */

BOOL sys_interactive(void);
void setup_syslogging( char *pname, BOOL interactive );
int sysDebug1( char *format_str, ... );
void sysbufr_print( void );
void format_system_text( char *msg );
BOOL systext( char *format_str, ... );

/* prototypes from  icqlog/log_usr.c */

BOOL usr_interactive(void); 
void setup_usrlogging( char *pname, BOOL interactive );
int usrDebug1( char *format_str, ... );
void usrbufr_print( void );
void format_users_text( char *msg );
BOOL usrtext( char *format_str, ... );

/* prototypes from system/util.cpp */
BOOL setNonBlocking(int fd);
FILE * dpopen(char *program, char *type);
int setup_byteorder();
BOOL process_exists(pid_t pid);
void results_delay(int mseconds);
void msleep(int t);
unsigned long arb(long param);
int sys_select(int maxfd, fd_set *fds,struct timeval *tval);
void generate_passwd(char *pass, int chars);
void init_random();
void random_fill(char *buffer, int size);
void random_check();

unsigned short random_num();
unsigned long lrandom_num();
void print_asc(int level, uchar const *buf, int len);
void dump_data(int level, const char *buf1, int len);
char *tmpdir(void);
BOOL in_group(gid_t group, gid_t current_gid, int ngroups, gid_t * groups);
pid_t sys_getpid(void);
BOOL file_exist(char *fname, struct stat * sbuf);
uint32 get_number(const char *tmp);
time_t file_modtime(char *fname);
void dos_clean_name(char *s);
void unix_clean_name(char *s);
BOOL reduce_name(char *s, char *dir, BOOL widelinks);
pid_t sys_fork(void);
char *Realloc(void *p, size_t size);
BOOL memcpy_zero(void *to, const void *from, size_t size);
void safe_free(void *p);
int str_checksum(const char *s);
void zero_free(void *p, size_t size);
void *memdup(const void *p, size_t size);

/* prototypes from system/slprintf.cpp */

int vslprintf(char *str, int n, char *format, va_list ap);
int slprintf(char *str, int n, char *format, ...);
int fdprintf(int fd, char *format, ...);

/* prototypes from system/icqtime.cpp */

unsigned long timeToLong(time_t ltime);
time_t longToTime(unsigned long ltime);
char *time2str(time_t stime);
char *time2str1(time_t stime);

/* prototypes from system/time.cpp */ 

void GetTimeOfDay(struct timeval *tval);
int tm_diff(struct tm *a, struct tm *b);
int TimeZone(time_t t);
void TimeInit(void);
int TimeZoneFaster(time_t t);
int TimeDiff(time_t t);
int LocTimeDiff(time_t lte);
struct tm *LocalTime(time_t *t);
BOOL null_mtime(time_t mtime);
char *timestr();
char *timestring(BOOL hires);
time_t get_create_time(struct stat  *st,BOOL fake_dirs);

/* prototypes from system/util_file.cpp */ 

off_t get_file_size(char *file_name);
off_t get_file_size(int fd);
BOOL file_modified_date(const char *filename, time_t *lastmodified);
BOOL file_modified(const char *filename, time_t *lastmodified);
char *file_load(char *fname, size_t *size);

/* prototypes from config/parse.cpp */ 
BOOL pm_process( char *FileName, BOOL (*sfunc)(char *),
	                 BOOL (*pfunc)(char *, char *) );

/* prototypes from system/daemon.cpp */ 
pid_t pidfile_pid();
BOOL write_pid();
int  become_daemon(void);
BOOL Signals_Init(void);
void childSignals_Init(void);

/* prototypes from netlib/udpserver.cpp */

void udp_receive_packet(Packet &pack);
BOOL udp_recv_pack(Packet &pack);
void udp_send_direct_packet(Packet &pack);
int udpserver_start(int sock_port, int log);
int udpserver_stop();
int udpserver_restart(int);
void wwp_receive_packet(Packet &pack);

/* prototypes from interface.cpp */

void load_interfaces(void);
BOOL interfaces_changed(void);
BOOL ismyip(struct in_addr ip);
BOOL is_local_net(struct in_addr from);
int iface_count(void);
BOOL we_are_multihomed(void);
struct interface *get_interface(int n);
struct in_addr *iface_n_ip(int n);
struct in_addr *iface_n_bcast(int n);
unsigned iface_hash(void);
struct in_addr *iface_bcast(struct in_addr ip);
struct in_addr *iface_ip(struct in_addr ip);
int get_interfaces(struct iface_struct *ifaces, int max_interfaces);
void init_bindinterface();

/* prototypes from netlib/network.cpp */

uint32 interpret_addr(char *str);
struct in_addr *interpret_addr2(char *str);
BOOL zero_ip(struct in_addr ip);
BOOL same_net(struct in_addr ip1, struct in_addr ip2, struct in_addr mask);
struct hostent *Get_Hostbyname(const char *name);

struct in_addr icqToIp(unsigned long l);
unsigned long ipToIcq(struct in_addr ltemp);

struct in_addr icqToIp2(unsigned long l);
unsigned long ipToIcq2(struct in_addr ltemp);

/* prototypes from system/ms_fnmatch.cpp */

int ms_fnmatch(char *pattern, char *string);

/* prototypes from isdcore/vx_proto/xxxx.cpp */

#include "isdcore/v3_proto/v3_proto.h"
#include "isdcore/v5_proto/v5_proto.h"
#include "isdcore/v7_proto/v7_proto.h"

#include "isdcore/sys_proto/sys_proto.h"

/* prototypes from database/xxxxx.cpp */

#include "database/db_proto.h"

/* prototypes from isdcore/confirm.cpp */

/* prototypes from isdcore/process.cpp */
void poll_table_init();
void accept_add_object(int index);
void close_socket_index(int index, unsigned long rnd_id);
void close_socket_index_r(int index, unsigned long rnd_id);
void close_socket(long sock_hdl, unsigned long sock_rnd);
void aim_accept_connect();
void check_sockets_timeout();
void msn_accept_connect();
void check_accepted_connections();
void udp_process(Packet &upacket);
int  tog_process(Packet &tpacket);
void wwp_process(Packet &wpacket);
void tcp_process(int index);
void watchdog_check();
void watchdog_init();
void prchilds_check();
void process_server();
void process_socket();
void process_packet();
void process_defrag();
void process_busy();
void init_childs_list();
void child_insert(pid_t child_pid, int role);
void child_delete(pid_t child_pid);
int  child_lookup(pid_t child_pid);
int  childs_num(int role);
int  geteppid();
void check_packet_processors();
void check_actions_processor();
void check_event_processors();
void check_defrag_processor();
void Server_cleanup();
BOOL isready_data(int number);
BOOL isready_data2(int number);
BOOL isready_error(int number);
void handle_wwp_mess(Packet &pack);
void spipe_send_packet(Packet &pack);

/* prototypes from isdcore/process_init.cpp */
void ipc_objects_init();
void lock_pipe(int pipe);
void unlock_pipe(int pipe);
void lock_ushm();
void unlock_ushm();

BOOL lock_incw();
BOOL unlock_incw();

BOOL lock_ipcw();
BOOL unlock_ipcw();

void fork_actions_processor();
void fork_packet_processors(int num);
void fork_epacket_processor();
void fork_euser_processor();
void fork_etimer_processor();
void fork_defrag_processor();
void fork_childs();
void switch_to_server_root(char *bin_path);
void init_translate();

/* prototypes from isdcore/broadcast.cpp */

void broadcast_online( struct online_user &auser );
void broadcast_offline( unsigned long uin );
void broadcast_status( struct online_user &auser, unsigned long old_status );
void broadcast_message(BOOL online_only, unsigned long department, char *message, unsigned short mess_type, unsigned long from_uin);
#endif

#endif
