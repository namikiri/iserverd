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
/* This unit contain functions for etimer process, that send time tics    */
/* every second to epacket-processor and euser-processor                  */
/*									  */
/**************************************************************************/

#include "includes.h"

#ifndef HAVE_UALARM
u_int ualarm (u_int value, u_int interval)
{
  struct itimerval timer, otimer;

  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = value;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = interval;

  if (setitimer (ITIMER_REAL, &timer, &otimer) < 0) return (u_int)-1;

  return (otimer.it_value.tv_sec * 1000000) + otimer.it_value.tv_usec;
}
#endif

void process_etimer()
{
   LOG_SYS(10, ("Init: ETimer processor initialization success\n"));

   /* signal initialization */
   rsignal(SIGALRM,  (void(*)(int))etimerSIGALRMHandler);
   ualarm(1000000, 1000000);   	/* we need alarm each 1 sec 	*/

   while(1)
   {
      /* all code is in SIGALRM handler */
      pause();
   }
}


/**************************************************************************/
/* Event processor SIGALRM handler 					  */
/**************************************************************************/
RETSIGTYPE etimerSIGALRMHandler(int param)
{
   send_timeout2cache();	/* notify euser processor about timeout   */
   send_timeout2epacket();	/* notify epacket processor about timeout */
   return;
}


/**************************************************************************/
/* This function send timeout message to online cache.                    */
/**************************************************************************/
void send_timeout2cache()
{
   struct event_pack pmessage;

   pmessage.mtype      = MESS_TIMEOUT;
   pmessage.uin_number = 0;
   pmessage.ack_stamp  = 0;
   pmessage.ttl	       = 0;

   euser_send_event(pmessage);
}


/**************************************************************************/
/* This function send timeout message to epacket processor.               */
/**************************************************************************/
void send_timeout2epacket()
{
   struct event_pack pmessage;

   pmessage.mtype      = MESS_TIMEOUT;
   pmessage.uin_number = 0;
   pmessage.ack_stamp  = 0;
   pmessage.ttl	       = 0;

   epacket_send_event(pmessage);
}


