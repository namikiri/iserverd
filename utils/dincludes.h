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
/**************************************************************************/

#ifndef _INCLUDES_H
#define _INCLUDES_H

#include "config.h"
#include "defaults.h"

#include <sys/types.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "libpq-fe.h"

#define FIELDS_NUMBER 41-1

#define OPTSTR  "d:x:p:u:a:w:"
//#define db_addr ""
//#define db_port "5432"
//#define db_user getenv("USER")
//#define db_pass ""

typedef char fld_string[255];
typedef char cmd_string[255];
typedef char fstring[255];

/* only one function definition */
void write_status(int plines, int pfound, int dead_usr,
		  int new_usr, int fullness, int progres);

/* single record for users info database (full version) */

typedef struct full_user_info
{
	unsigned long		uin; 	/* user uin number */
	int				 	auth;	/* authorization */
	int				 pemail1;	/* if email1 is public */

	int					 age;	/* user's age */
	int				  gender;	/* user's gender */
	int				disabled;	/* if user's account disabled */
	char	   can_broadcast;	/* if he can send broadcast messages */
	char		 ch_password;	/* if user need to change password */
	char 	  	  gmt_offset;	/* user gmt time offset */

	char		 	  bmonth;	/* user's birth day */
	char		  		bday;	/* user's bith month */
	char	 		   byear;	/* user's birth year */
	int				wcountry;	/* work country */
	long			 cr_date;	/* date of account creation */
	long  		   lastlogin;	/* last user login time */
	unsigned long	 ip_addr;	/* last user's ip address */
	unsigned long	 nupdate;	/* notes update time */

	int			 	hcountry;	/* home country */
	int 			   wocup;	/* work ocupation */

	char 		wcompany[32];	/* work company name */
	char 	  	  wtitle[32];	/* work title */

	char		  hphone[32];	/* home phone */
	char 			hfax[32];	/* home fax */
	char 		   hcell[32];	/* home cell phone */
	char 			hzip[32];	/* home zip code */
	char		  hpage[128];	/* www home page */
	char		   haddr[64];	/* home address */
	char		   hcity[32];	/* home city */
	char		  hstate[32];	/* home state */

	char		   waddr[64];	/* work address */
	char		   wcity[32];	/* work city */
	char		  wstate[32];	/* work state */

	char 	 	 wdepart[32];	/* work department */
	char		  wphone[32];	/* work phone */
	char	 	  wpager[32];	/* work cell phone */
	char 			wfax[32];	/* work fax */
	char 			wzip[32];	/* work zip code */
	char 		  wpage[128];	/* work www page */
	char		  passwd[32];	/* user's password */
	char			nick[32];	/* user's nickname */
	char			last[32]; 	/* user's lastname */
	char		   first[32]; 	/* user's firstname */
	char		  email1[64]; 	/* user's first email addres */
	char		  email2[64]; 	/* user's second email addres */
	char		  email3[64]; 	/* user's third email addres */
	char		  notes[255];	/* users notes */
	unsigned short	 martial;	/* user's martial status */
	char			nuin[32]; 	/* new string uin */
	char		   wzip2[11]; 	/* new wzip string */
	char		   hzip2[11]; 	/* new hzip string */
	short			bcountry;	/* born country */
	char		  bstate[32];	/* born state */
	char		   bcity[32];	/* born city */
} full_user_info;

#include "translate.h"

#endif /* _DB_CONVERT_H */

