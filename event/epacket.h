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
/*									  */
/* This file contain definitions for packet confirm-queue structures 	  */
/*                                                                        */
/**************************************************************************/

#ifndef _CONFIRM_H_
#define _CONFIRM_H_

/* Structure to receive indirect packet */
typedef struct indirect_pack
{
   unsigned int  sizeVal;	/* data size (packet size) 	*/
   unsigned long shm_index;	/* online user record shm index */
   unsigned int  retry_num;	/* number of retries		*/
   unsigned int  ctime_out;	/* current timeout		*/
   unsigned int  time_out;	/* max time to next retry	*/  
   unsigned long ack_stamp;	/* unique packet stamp		*/
   unsigned long to_uin;	/* to whom we send pack		*/
   unsigned int  to_port;	/* destination port 		*/
   struct   in_addr to_ip;	/* destination address 		*/
   char buff[MAX_PACKET_SIZE];	/* data buffer (packet)		*/

} indirect_pack;


/* Packet with expiration information */
typedef struct ipack_s
{
   struct ipack_s *next;	/* next record pointer		*/
   struct ipack_s *prev;	/* previous record pointer      */
   unsigned int  sizeVal;	/* data size (packet size) 	*/
   unsigned int  retry_num;	/* number of retries		*/
   unsigned int  ctime_out;	/* current timeout		*/
   unsigned long ack_stamp;	/* unique packet stamp		*/
   char *buff;			/* data buffer (packet)		*/

} ipack_s;


/* User unconfirmed packets queue root */
typedef struct usr_queue_s
{
   unsigned long uin;		/* to whom we send pack         */
   unsigned long usid;		/* user session id		*/
   unsigned long ishm;		/* online user record shm index */
   struct   in_addr ip;		/* destination address 	        */
   unsigned int  port;		/* destination port 	        */
   unsigned int  time_out;	/* max time to next retry       */
   struct ipack_s *pqueue;	/* packets queue for this user  */
   struct usr_queue_s *next;	/* next record pointer  	*/
   struct usr_queue_s *prev;	/* previous record pointer      */

} usr_queue_s;

#endif

