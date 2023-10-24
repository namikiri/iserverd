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
/* Public variables declarations 					  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

char **real_argv;		 /* pointer to real arglist     */
int    real_argc;		 /* argument list count		*/

int SEM_KEY;			 /* key to create IPC objects   */
int semaphore_id;		 /* semaphores identificator	*/
int messqueue_id;		 /* ipc messages identificator	*/
int sharedmem_id;		 /* shared memory identificator	*/
int usr_shm_id;		 	 /* userspace shm identificator	*/
int max_user_cnt;		 /* max online user count       */

int msockfd    = -1;		 /* main socket handler		*/
int wwpsockfd  = -1;		 /* web-pager socket handler	*/
int aimsockfd  = -1;		 /* V7+ server tcp socket       */
int msnsockfd  = -1;		 /* MSN server tcp socket	*/

int tcp_sock_count;		 /* num of active tcp sockets   */
int rvs_order;			 /* 1 if we have rev byteorder  */
struct pollfd     *sock_fds;	 /* array for socket handlers	*/
struct sock_info  *sock_inf;	 /* sock additional information */
struct pollfd evnt_fds[2];	 /* array for event handlers	*/

struct sockaddr_un ip_serv_addr;
struct sockaddr_un ip_clnt_addr;
socklen_t ip_saddrlen;

struct sockaddr_un op_serv_addr;
struct sockaddr_un op_clnt_addr;
socklen_t op_saddrlen;

struct sockaddr_un top_serv_addr;
struct sockaddr_un top_clnt_addr;
socklen_t top_saddrlen;

struct sockaddr_un epp_serv_addr;
struct sockaddr_un epp_clnt_addr;
socklen_t epp_saddrlen;

struct sockaddr_un eup_serv_addr;
struct sockaddr_un eup_clnt_addr;
socklen_t eup_saddrlen;

struct sockaddr_un fp_serv_addr;
struct sockaddr_un fp_clnt_addr;
socklen_t fp_saddrlen;

struct sockaddr_un ap_serv_addr;
struct sockaddr_un ap_clnt_addr;
socklen_t ap_saddrlen;

struct sockaddr_un wp_serv_addr;
socklen_t wp_saddrlen;

int process_role;		 /* ROLE_PACKET, ROLE_SOCKET, ROLE_EVENT */
int childs_check;		 /* is any child death? */
unsigned long deluser = 0;

class Packet spack;
struct child_record *cl_begin = NULL;
struct child_record *cl_last  = NULL;
struct child_record *cl_list  = NULL;

struct in_addr bind_interface;	/* main interface value     */
struct in_addr old_interface;	/* old interface value      */

unsigned long old_time;		/* old time counter         */
unsigned long curr_time;	/* current time value       */
unsigned long increm_num;	/* number of pushed packets */

int old_sock_port;
int sems_ready		= 0;
int exit_ok		= 1;	 /* set it to 1 to forbid exit on SIGINT */

BOOL no_death_messages   = False;
BOOL reload_in_progress  = False;
BOOL block_nprocessor    = False;
BOOL drandom		 = False;

time_t server_started;		 /* time when server was started 	  */
time_t config_loaded;		 /* last time when config was loaded 	  */

unsigned long pack_processed;    /* total packets processed 		  */

struct ipc_structure *ipc_vars;  /* structure located in shared memory 	  */
struct online_user  *usr_shm;    /* shm array for online users info       */

FILE *pidFP = NULL;		 /* system pidfile handler 		  */

int     DEBUGLEVEL 	= 1;
int     LOGLEVEL 	= 1;

PGconn   *users_dbconn  = NULL;  /* database connection structure	  */
ITranslator ITrans;		 /* string translator			  */

class Packet int_pack;
class Packet reply_pack;
class Packet reply_pack2;
class Packet reply_pack3;
class Packet arpack;		 /* aim reply packet                      */
class Packet pipe_pack;
class Packet papack;

BOOL eservices[MAX_FAMILIES];    /* services enabled/disabled flag	  */

/* static buffers. I plan to remove all this stuff soon */
unsigned long contacts[512];
char evt_param[255];
int * pack_buf[MAX_PACKET_SIZE];
char temp_buffer[MAX_PACKET_SIZE+1024];
char tempst[MAX_PACKET_SIZE+192];
char tempst3[MAX_PACKET_SIZE+192];
char msg_buff[MAX_PACKET_SIZE+192];
char msg_buff2[MAX_PACKET_SIZE+192];
