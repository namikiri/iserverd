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
/* This file contain functions for scheduled operations (database 	  */
/* vacuuming, online_cache validating).                                   */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

unsigned long counter1;		/* counter for database vacuumdb 	  */
unsigned long counter2;		/* counter for online_db validate 	  */
unsigned long counter3;		/* counter for defrag_db validate 	  */

/**************************************************************************/
/* Scheduler initialization						  */
/**************************************************************************/
void scheduler_init()
{
   counter1 = 1;
   counter2 = 1;
   counter3 = 1;
}

/**************************************************************************/
/* This function called every second by event processor			  */
/**************************************************************************/
void scheduler_timer()
{
   if (counter1 == (unsigned long)abs(lp_sched_vacuum()))
   {
      call_scheduler_vacuum();
      counter1 = 1;
   }

   if (counter2 == (unsigned long)abs(lp_online_check()))
   {
      call_online_check();
      counter2 = 1;
   }

   if (counter3 == (unsigned long)abs(lp_defrag_check()))
   {
      call_defrag_check();
      counter3 = 1;
   }

   counter1 += 1;
   counter2 += 1;
   counter3 += 1;
}

/**************************************************************************/
/* This function called for database vacuuming 				  */
/**************************************************************************/
void call_scheduler_vacuum()
{
    LOG_SYS(0, ("It is time to vacuum database...\n"));
    vacuum_online_tables();

    return;
}


/**************************************************************************/
/* This function called to cleanup fragment storage table		  */
/**************************************************************************/
void call_defrag_check()
{
    struct defrag_pack dpack;

    LOG_SYS(0, ("It is time to cleanup defragment table...\n"));

    dpack.mtype   = MESS_FRGCHECK;
    dpack.uin_num = 0;
    dpack.seq	  = 0;

    defrag_send_pipe(dpack);

    return;
}


/**************************************************************************/
/* This function called to cleanup fragment storage table		  */
/**************************************************************************/
void call_online_check()
{
    LOG_SYS(0, ("It is time to check online_users table...\n"));
    check_online_table();

    return;
}

