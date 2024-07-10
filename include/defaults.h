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

#ifndef _DEFAULTS_H
#define _DEFAULTS_H

#define RANDOM_DEVICE_PATH	"@RANDOM_DEVICE_PATH@"
#define SYSTEM_UNAME		"Linux 6.1.0-22-amd64"
#define SYSTEM_NAME		"foli"
#define COMPILER_NAME		"g++"
#define ICQ_SERVER_FILE		"/usr/bin/iserverd"
#define ICQ_CONFIG_FILE		"/etc/iserverd/iserv.conf"
#define ACTIONS_CONF		"/etc/iserverd/actions.conf"
#define ICQ_SYSLOG_FILE		"/var/log/iserverd/system.log"
#define ICQ_USRLOG_FILE		"/var/log/iserverd/users.log"
#define ICQ_HCKLOG_FILE		"/var/log/iserverd/alarm.log"
#define ICQ_DBGLOG_FILE		"/var/log/iserverd/debug.log"
#define ICQ_LOG_PERMS		000664

#define ICQ_TRANSLATE_PATH	"/etc/iserverd/translate"
#define TRANSLATE_FILE		"/etc/iserverd/translate/%s"
#define WWP_SOCK_FILENAME	"/tmp/wwp_sock"
#define ICQ_VAR_DIR		"/var/run/iserverd/"
#define ICQ_PID_FILE		"/var/run/iserverd/iserverd.pid"
#define ICQ_DB_USER		"iserverd"
#define SYSLOG_SIDENT		"IServer"
#define DEFAULT_PING_TIME	120
#define ADMIN_UIN		1
#define WWP_TYPE		0x0D
#define CLUE_CHAR		"\xFE"
#define MAX_ARGS		255
#define MAX_AEMAIL		10000
#define MAX_AMSG		1024
#define KQNEVENTS		256	/* number of kqueue events */

/* department list version */
#define DEP_LIST_VERS		1
/* externals number */
#define EXTERNALS_NUM		0

#define V3_PING_TIME		45
#define V3_MAX_SEARCH		50
#define V3_TIMEOUT		10
#define V3_RETRIES		3
#define V3_MAX_MESSAGE		2025
#define V3_PACKET_MTU		500

#define V5_PING_TIME		120
#define V5_TIMEOUT		10
#define V5_RETRIES		5
#define V5_MAX_SEARCH		50
#define V5_MAX_MESSAGE          450

#define V7_CONN_TIMEOUT		20
#define V7_COOKIE_TIMEOUT	120
#define V7_MAX_PROFLEN		1024
#define V7_MAX_CONTACT_SIZE     600
#define V7_MAX_WATCHERS_SIZE	750
#define V7_MAX_VISIBLE_SIZE     160
#define V7_MAX_INVISIBLE_SIZE   160
#define MAX_CAPS		14
#define MAX_FAMILIES		32
#define MAX_CONTACTS		511
#define MAX_ICBM_CHANNELS	4

#define UDP_PORT		4000
#define AIM_PORT		-1
#define MSN_PORT		-1
#define LOG_UNLIMITED		-1
#define MAX_PACKET_SIZE		8192
#define MAX_BIND_TRY		3
#define WATCHDOG_TIMEOUT	10
#define MAX_TCP_CONNECTIONS	512
#define RES_SLOTS		11

#define VALIDATE_CACHE_TIMEOUT  3600
#define VACUUM_DB_TIMEOUT	1800
#define DEFRAG_TIMEOUT		3600
#define ONLINE_TIMEOUT		3600

#define WWP_TYPE		0x0D

#endif /*DEFAULTS_H*/
