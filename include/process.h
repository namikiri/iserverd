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

#ifndef _PROCESS_H
#define _PROCESS_H

#define ROLE_NONE    -1
#define ROLE_SOCKET   1
#define ROLE_PACKET   2
#define ROLE_EPACKET  3
#define ROLE_BUSY     4
#define ROLE_DEFRAG   5
#define ROLE_EUSER    6
#define ROLE_ETIMER   7
#define ROLE_ACTIONS  8
#define ROLE_EMPTY    9
#define ROLE_PAUSED  10

#define ROLE_DUSER   100
#define ROLE_STATUS  101

#define P_READ 0
#define P_WRIT 1

#define SUDP 0
#define SWWP 1
#define SAIM 2
#define SMSN 3
#define STOG 4

#define SOUT 0
#define SEVT 1

#define SOCK_NEW  0
#define SOCK_DATA 1
#define SOCK_TERM 2
#define SOCK_RDY  3

#define INCOMING_PIPE 0
#define OUTGOING_PIPE 1

#define EXIT_NORMAL			0
/* IServerd just begin daemonization */
#define EXIT_FORK			0
/* IServerd can't find config/can't create log */
#define EXIT_CONFIG			1
/* IServerd can't find iface, add "interfaces" line to config   */
#define EXIT_ERROR_INTERFACES		2
/* Possibly another IServerd is running on same port		*/
#define EXIT_ERROR_ANOTHER_PROCESS	3
/* some fatal error ocupied - server can't work 	        */
#define EXIT_ERROR_FATAL		4
/* IServerd can't initialize IPC objects (semaphores,pipes,shm) */
#define EXIT_ERROR_IPC			5
/* IServerd can't create debug log file. Check dir permissions  */
#define EXIT_ERROR_LOG_CREATE		6
/* IServerd can't start because config file is missing          */
#define EXIT_ERROR_CONFIG_OPEN		7
/* process can't connect to database. Check if RDBMS is running */
#define EXIT_ERROR_DB_CONNECT		8
/* SP can't check db integrity because database doesn't exist   */
#define EXIT_ERROR_DB_NOT_FOUND		9
/* database have structure problem, socket processor should     */
/* kill all childs and then run database recovery procedure     */
#define EXIT_ERROR_DB_STRUCTURE		10
/* sql query aborted for some reason, SP should fork another xP */
#define EXIT_ERROR_DB_UNKNOWN		11
/* socket processor on this exit code should kill all childs    */
/* and then sleep for several seconds waiting for DBMS recovery */
#define EXIT_ERROR_DB_FAILURE		12
/* we can't work on pdp byteorder :( */
#define EXIT_ERROR_BYTEORDER		13

/* structure for information about tcp sockets */
typedef struct sock_info
{
   unsigned short aim_srv_seq;
   unsigned short aim_cli_seq;
   unsigned long  rnd_id;
   unsigned long  lutime;
   unsigned short sock_type;
   unsigned short aim_started;
   struct sockaddr_in cli_addr;

   /* TODO: information about packet rate of this socket */

} sock_info;

#endif /*_PROCESS_H*/

