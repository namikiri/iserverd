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
/* This module contain snac creating/processing functions                 */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* Stats services SNAC family packets handler				  */
/**************************************************************************/
void process_snac_stats(struct snac_header &snac, Packet &pack)
{

}


/**************************************************************************/
/* This func create and send report status interval value		  */
/**************************************************************************/
void stats_send_interval(struct online_user *to_user)
{
   arpack.clearPacket();
   arpack.setup_aim();

   arpack << (unsigned short)SN_TYP_STATS
          << (unsigned short)SN_STS_SETxREPORTxINT
	  << (unsigned short)0x0000
	  << (unsigned  long)to_user->servseq2++
	  << (unsigned short)0x0001;             /* I should check this ! */

   arpack.from_ip      = to_user->ip;
   arpack.from_port    = 0x0000;
   arpack.sock_hdl     = to_user->sock_hdl;
   arpack.sock_rnd     = to_user->sock_rnd;
   arpack.sock_evt     = SOCK_DATA;
   arpack.sock_type    = SAIM;
   arpack.flap_channel = 0x02;

   tcp_writeback_packet(arpack);
}


