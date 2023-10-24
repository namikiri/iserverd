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

#ifndef _INCLUDES_H
#define _INCLUDES_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_GD
#include <gd.h>
#include <gdfontg.h>
#endif

#ifdef HAVE_ICONV
#include <iconv.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h> 
#endif

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifndef WCOREDUMP
/*I know this is not good.... but who cares ? */
#define WCOREDUMP(s) ((s)&80)
#endif

#ifdef HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif

#ifdef HAVE_SYS_SEM_H
#include <sys/sem.h>
#endif

#ifdef HAVE_SYS_MSG_H
#include <sys/msg.h>
#endif

#ifdef HAVE_SYS_SHM_H
#include <sys/shm.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef USE_POLL
#define DISABLE_ADD_OBJECT 1
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef __cplusplus
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif

#include <sys/un.h>
#if defined HAVE_POLL_H      
#include <poll.h>            
#elif defined HAVE_SYS_POLL_H
#include <sys/poll.h>        
#endif                       

#ifdef HAVE_SYSLOG_H
#include <syslog.h> 
#endif

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#include "mystdint.h"

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_SYS_EPOLL_H
#include <sys/epoll.h>
#endif

#ifdef HAVE_SYS_EVENT_H
#include <sys/event.h>
#endif

#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

/* Defines for u_intxx_t */
#ifndef u_int8_t
#define u_int8_t uint8_t
#endif

#ifndef u_int16_t
#define u_int16_t uint16_t
#endif

#ifndef u_int32_t
#define u_int32_t uint32_t
#endif

#ifdef __cplusplus
#ifndef u_int64_t
#ifdef uint64_t
#define u_int64_t uint64_t
#else
#ifndef HAVE_TYPE_U_INT64_T
typedef unsigned long long u_int64_t;
#endif
#endif
#endif
#endif

#if defined(HAVE_SIGACTION) && defined(HAVE_SIGPROCMASK) && defined(HAVE_SIGADDSET)
# include <signal.h>
#endif

#include "defaults.h"

#ifdef __cplusplus
#include "talloc.h"
#include "dlinklist.h"
#include "md5.h"
#endif

#ifndef _BOOL
#define _BOOL
#define False (0)
#define True (1)
#define Auto (2)
typedef int BOOL;
#endif

#ifndef _SMODE
#define _SMODE
#define MOD_STANDALONE (0)
#define MOD_DAEMON (1)
#define MOD_INETD  (2)

#define ORDER_FORWARD  (0)
#define ORDER_BACKWARD (1)
#define ORDER_DEFAULT  (0)

typedef int SMODE;
#endif

#ifndef _PSTRING                  
#define PSTRING_LEN 1025
#define FSTRING_LEN 257
#define FIELDST_LEN 65
#define CSTRING_LEN 1025
#define MSTRING_LEN 2049
typedef char bigstring[MAX_PACKET_SIZE+1024];
typedef char pstring[PSTRING_LEN];
typedef char fstring[FSTRING_LEN];
typedef char cstring[CSTRING_LEN];
typedef char mstring[MSTRING_LEN];
typedef char ffstring[FIELDST_LEN];
#define _PSTRING
#endif                            

#ifndef uint32
#define uint32 unsigned int
#endif

#ifndef uchar
#define uchar unsigned char
#endif

#if !HAVE_WORKING_VFORK
# define vfork fork
#endif

#define LIST_SEP " \t,;:\n\r"
enum case_handling {CASE_LOWER,CASE_UPPER};

#ifdef __cplusplus
#include "safe_string.h"
#include "macros.h"
#include "debug.h"
#endif

#include "log_sys.h"

#ifdef __cplusplus
#include "log_alr.h"
#include "log_usr.h"
#include "lock.h"
#include "process.h"
#include "buffer.h"
#include "packet.h"
#include "database/init_db.h"
#include "database/database.h"
#include "database/ssi_db.h"
#include "event/epacket.h"
#include "event/euser.h"
#include "event/event.h"
#include "fprocess.h"
#include "isdcore/sys_proto/vsys_defines.h"
#include "isdcore/v7_proto/tlv.h"
#endif

#include "config/actions/parse_tree.h"

#ifdef __cplusplus
#include "pactions/actions.h"

#ifdef HAVE_POSTGRESDB
#include "libpq-fe.h"
#include "libpq-fs.h"
#endif
#endif

#include "proto.h"
#include "proto_defines.h"

#ifdef __cplusplus
#include "interfaces.h"
#include "translate.h"
#include "protos.h"
#endif

#ifndef O_ACCMODE                               
#define O_ACCMODE (O_RDONLY | O_WRONLY | O_RDWR)
#endif                                          

#define ZERO_ARRAY(x) memset((char *)(x), 0, sizeof(x))
#define SLAB(LEN, POS)	LEN - POS >= 4 ? 4 : LEN - POS
#define ZERO_STRUCTPN(x) memset((char *)(x), 0, sizeof(*(x)))
#define ip_equal(ip1,ip2) ((ip1).s_addr == (ip2).s_addr)
#define putip(dest,src) memcpy(dest,src,4)

#ifndef HAVE_MEMCPY
#define	memcpy(s1, s2, n)	bcopy(s2, s1, n)
#endif

/* Ok some systems do not have vsyslog function...            */
/* lets make it by hands... Here we can define only prototype */

#ifndef HAVE_VSYSLOG
void vsyslog(int priority, const char *message, va_list args);
#endif

/**************************************************************************/
/* find the difference in milliseconds between two struct timeval	  */
/* values								  */
/**************************************************************************/

#define TvalDiff(tvalold,tvalnew) \
  (((tvalnew)->tv_sec - (tvalold)->tv_sec)*1000 +  \
	 ((int)(tvalnew)->tv_usec - (int)(tvalold)->tv_usec)/1000)

#define OPTSTR  		"r:loVd:c:p:m:"

#ifndef HAVE_STRUCT_SEMUN
typedef union semun 
{
    int val;                           /* <= value for SETVAL */
    struct semid_ds *buf;              /* <= buffer for IPC_STAT & IPC_SET */
    unsigned short int *array;         /* <= array for GETALL & SETALL */
    struct seminfo *__buf;             /* <= buffer for IPC_INFO */

} semun;
#endif

#ifndef __IPC_STRUCTURE
#define __IPC_STRUCTURE
typedef struct ipc_structure
{

   unsigned long  pack_in_queue;	/* number of packets in pipe          */
   unsigned long  byte_in_queue;	/* number of bytes in pipe            */
   unsigned long  max_queue_size;	/* maximum bytes was in queue         */
   unsigned long  max_queue_pack;	/* max number of packets in queue     */
   unsigned long  queue_send_errors;	/* number of errors in queue          */
   unsigned long  incw_packet_seq;	/* inc write pipe packet sequence     */
   unsigned long  incr_packet_seq;	/* inc read pipe packet sequence      */   
   unsigned short magic_num1;		/* random number1 for system injector */
   unsigned short magic_num2;		/* random number2 for system injector */
   unsigned long  online_usr_num;	/* number of records in shm userspace */

} ipc_structure;

#endif

#ifndef HAVE_SOCKLEN_T
#ifdef RECVFROM_WITH_INT
#define socklen_t int
#else
typedef unsigned int unint_def;
#define socklen_t unint_def
#endif
#endif

#ifdef __cplusplus
#include "isdcore/v7_proto/v7_defines.h"
#include "isdcore/v7_proto/capabilities.h"
#endif

#include "public.h"

#ifndef __CHILD_LIST
#define __CHILD_LIST

/* child record for childs list */
struct child_process
{

    pid_t child_pid;		 /* child pid number */
    int   child_role;		 /* child function   */
    
};

/* childs list element record */
typedef struct child_record
{
    struct child_record  *prior; 
    struct child_record  *next;
    struct child_process child;
    
} child_record;
#endif

#ifndef __SERVINFO
#define __SERVINFO

typedef struct TServinfo
{
  unsigned long  lutime;
  unsigned long  bind_addr;
  unsigned long  bind_port;
  unsigned long  pack_processed;
  unsigned long  pack_in_queue;
  unsigned long  queue_size;
  unsigned long  max_queue_size;
  unsigned long  max_queue_pack;
  unsigned long  queue_send_errors;
  unsigned long  used_memory;
  unsigned long  config_loaded;
  unsigned long	 server_started;
  unsigned long  sp_pid;
  unsigned long  ep_pid;
  unsigned long  pp_num;
  unsigned long  bp_num;
  long  lbtime;

} TServinfo;

#endif


#endif /* INCLUDES_H */
