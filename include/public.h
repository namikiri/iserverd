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
/* This unit contain public variables declarations			  */
/*                                                                        */
/**************************************************************************/

extern char **real_argv;		  /* pointer to real arglist      */
extern int    real_argc;		  /* argument list count          */

extern int incoming_pipe_fd[2];		  /* incoming pipe descriptor     */
extern int outgoing_pipe_fd[2];		  /* outgoing pipe descriptor     */
extern int epacket_pipe_fd[2];		  /* epacket pipe descriptor      */
extern int euser_pipe_fd[2];              /* euser pipe descriptor        */
extern int defrag_pipe_fd[2];		  /* defrag pipe descriptor       */
extern int actions_pipe_fd[2];		  /* actions pipe descriptor      */
extern int toutgoing_pipe_fd[2];	  /* tcp outgoing pipe descriptor */
extern int wwpsockfd;		  	  /* web-pager socket handler     */
extern int aimsockfd;		          /* V7+ server tcp socket        */
extern int msnsockfd;		  	  /* MSN server tcp socket	  */

extern int tcp_sock_count;             	  /* num of active tcp sockets    */
extern int rvs_order;                     /* 1 if we have rev byteorder   */
extern struct pollfd*     sock_fds;	  /* array for handlers           */
extern struct sock_info*  sock_inf;       /* sock additional information  */
extern struct pollfd evnt_fds[2];	  /* array for handlers           */

extern struct sockaddr_un wp_serv_addr;
extern socklen_t wp_saddrlen;

extern struct sockaddr_un ip_serv_addr;
extern struct sockaddr_un ip_clnt_addr;
extern socklen_t ip_saddrlen;

extern struct sockaddr_un op_serv_addr;
extern struct sockaddr_un op_clnt_addr;
extern socklen_t op_saddrlen;

extern struct sockaddr_un top_serv_addr;
extern struct sockaddr_un top_clnt_addr;
extern socklen_t top_saddrlen;

extern struct sockaddr_un epp_serv_addr;
extern struct sockaddr_un epp_clnt_addr;
extern socklen_t epp_saddrlen;

extern struct sockaddr_un eup_serv_addr;
extern struct sockaddr_un eup_clnt_addr;
extern socklen_t eup_saddrlen;

extern struct sockaddr_un ap_serv_addr;
extern struct sockaddr_un ap_clnt_addr;
extern socklen_t ap_saddrlen;

extern struct sockaddr_un fp_serv_addr;
extern struct sockaddr_un fp_clnt_addr;
extern socklen_t fp_saddrlen;

extern int SEM_KEY;			/* key to create IPC objects	*/
extern int semaphore_id;		/* semaphores identificator	*/
extern int messqueue_id;		/* ipc messages identificator	*/
extern int sharedmem_id;		/* shared memory identificator	*/
extern int usr_shm_id;		 	/* userspace shm identificator	*/
extern int max_user_cnt;                /* max online user count        */

extern int process_role;		/* ROLE_PACKET, ROLE_SOCKET, .. */
extern int childs_check;		/* is any child death?		*/
extern int * pack_buf[MAX_PACKET_SIZE]; /* buffer for packets		*/

extern struct in_addr bind_interface;	/* udp listen interface		*/
extern struct in_addr old_interface;
extern int old_sock_port;
extern int msockfd;			/* main network socket 		*/

extern unsigned long old_time;          /* old time counter             */
extern unsigned long curr_time;         /* current time value           */
extern unsigned long increm_num;        /* number of pushed packets     */

extern int DEBUGLEVEL;
extern int LOGLEVEL;

extern FILE *pidFP;

extern int sems_ready;
extern int exit_ok;

extern BOOL no_death_messages;
extern BOOL reload_in_progress;
extern BOOL block_nprocessor;
extern BOOL drandom;

extern time_t server_started;
extern time_t config_loaded;

#ifdef __cplusplus
extern PGconn   *users_dbconn;
extern class ITranslator ITrans;
extern class Packet int_pack;
extern class Packet reply_pack;
extern class Packet reply_pack2;
extern class Packet reply_pack3;
extern class Packet pipe_pack;
extern class Packet papack;
extern class Packet spack;
extern class Packet arpack;
#endif

extern unsigned long pack_processed;

extern struct ipc_structure *ipc_vars;
extern struct online_user *usr_shm;

extern struct child_record *cl_begin;
extern struct child_record *cl_last;
extern struct child_record *cl_list;

extern unsigned long uin, uin1, uin2;
extern unsigned short rstatus;

extern struct uin_node *uins_node;
extern struct uin_node *uins_lst1;
extern struct uin_node *uins_lst2;
extern struct uin_node *uins_var;
extern struct variable_record *var1, *var2;
extern struct _rules_root rules_root;
extern struct variable_record *aim_list;
extern struct variable_record *var_list;
extern struct template_record *ptt_list;

extern int include_stack_ptr;
extern int line_number;
extern int included;
extern int rsection;
extern int astop;
extern int config_file_type;
extern unsigned long deluser;

extern FILE *yyin_temp;

extern unsigned long snumber;
extern struct _aim_root aim_root;
extern unsigned short rsindex;
extern unsigned short pvalue;
extern unsigned short family;

extern BOOL eservices[MAX_FAMILIES];

extern unsigned long contacts[512];
extern char pname[255];

extern char us_token[256];
extern char iname[255];
extern char filename[255];
extern char evt_param[255];
extern char par1[255];
extern char par2[255];
extern char par3[255];
extern char yyerr_str[255];

extern FILE *yyin, *yyout, *yyin_old;

/* static buffers */
extern char temp_buffer[MAX_PACKET_SIZE+1024];
extern char tempst[MAX_PACKET_SIZE+192];
extern char tempst3[MAX_PACKET_SIZE+192];
extern char msg_buff[MAX_PACKET_SIZE+192];
extern char msg_buff2[MAX_PACKET_SIZE+192];
