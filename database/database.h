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
/* This file contain definitions for all database functions (i.e. user	  */
/* info structures, online structures)					  */
/*                                                                        */
/**************************************************************************/

#define ONLINE_DB_FNUM 10	/* num of fields in online db 		  */
#define USERS_DB_FNUM 84	/* num of fields in users db 		  */
#define DEPS_MAX_NUM 250	/* max number of departments 		  */

/* types of contact lists (for contact database) */
#define NORMAL_CONTACT     1	/* normal contact list  		  */
#define INVISIBLE_CONTACT  2	/* invisible to users   		  */
#define VISIBLE_CONTACT    3	/* visible to users     		  */
#define IGNORE_CONTACT     4	/* user's ignore list   		  */

/* variables for set web page function */
#define HOME  1
#define WORK  2

/* cookie types */
#define TYPE_COOKIE 1		/* normal cookie to login with            */
#define TYPE_AUTH   2		/* authorization key sent to user         */

/* ssi perms record type */
#define PERMS_AUTH  1		/* authorization record                   */
#define PERMS_ADDED 2		/* 'You were added' record                */

/* online profiles types */
#define OPRF_PROFILE 1		/* online profile - used by AIM client    */
#define OPRF_AWAY    2		/* away text - used by ICQ/AIM clients    */

typedef struct icbm_params
{
   unsigned short max_msglen;	/* max message snac length		  */
   unsigned short max_sevil;	/* max sender warnings		          */
   unsigned short max_revil;	/* max receiver evil		          */
   unsigned short min_interval; /* min interval between messages          */
   unsigned long  icbm_flags;	/* icbm flags 		 	   	  */
   unsigned long  last_msg;	/* last msg time (for min_interval)  	  */

} icbm_params;

/* This is single record for online database  */
typedef struct online_user
{
   unsigned long  uin;		/* online user uin number 		  */
   unsigned long  usid;		/* user session id number (random)	  */
   in_addr  	  ip;		/* user's ip 				  */
   in_addr	  int_ip;	/* users's internal ip 			  */
   unsigned long  udp_port;	/* user's back udp port 		  */
   unsigned long  tcp_port;	/* user's tcp server port 		  */
   unsigned long  web_port;	/* users web front port			  */
   unsigned long  cli_futures;  /* futures like activated web-server	  */
   char 	  dc_type;      /* direct connection type		  */
   char           dc_perms;	/* dc permissions                         */
   unsigned long  dc_cookie;	/* direct connection cookie 		  */
   unsigned short uclass;	/* user class				  */
   unsigned short status;	/* user current status 			  */
   unsigned long  uptime;	/* last login time 			  */
   unsigned long  crtime;	/* account creation time                  */
   unsigned long  lutime;	/* last update time 			  */
   unsigned long  info_utime;	/* last info update time		  */
   unsigned long  more_utime;	/* last icq more info update (i.e. phone) */
   unsigned long  stat_utime;	/* last icq ext status update		  */
   unsigned long  idle_time;	/* client idle time 			  */
   unsigned long  idle_perms;	/* idle permissions                       */
   unsigned long  ttl;		/* max live-time without update 	  */
   unsigned long  ttlv;		/* current live-time 			  */
   unsigned short protocol;	/* protocol number 			  */
   unsigned long  servseq;	/* current server sequence num		  */
   unsigned long  servseq2;	/* v5 proto server sequence 2		  */
   unsigned long  session_id;	/* v5 session random number (id)	  */
   unsigned short state;	/* client state 			  */
   unsigned short tcpver;	/* client TCP protocol version		  */
   unsigned short estat;	/* client extended status 		  */
   unsigned long  llocked;	/* date when record was locked by backend */
   unsigned short active;	/* 0 - client not ready, 1 - client ready */
   unsigned short cloaded;	/* 1 - contact was loaded, 0 - not loaded */
   unsigned long  sock_hdl;	/* tcp socket handler num                 */
   unsigned long  sock_rnd;	/* tcp socket handler second id           */
   unsigned long  shm_index;	/* this record index in shm memory segm   */
   char           caps_num;	/* number of capability CLSIDs            */
   char		  migration;	/* flag: 1 - migration started		  */
   char 	  disable_blm;	/* flag: 1 - client should use SSC only   */
   char 	  enable_ssi;	/* flag: 1 - enable SSI for this user     */
   char 	  ssi_trans;	/* flag: 1 - ssi transaction in progress  */
   char 	  import_mode;	/* flag: 1 - import ssi trans in progres  */
   unsigned short ssi_version;  /* client ssi service handler version     */

   unsigned short warn_level;	/* user warning level 			  */

   struct icbm_params mopt[MAX_ICBM_CHANNELS]; /* icbm params per channel */

   char           caps[MAX_CAPS][16];	/* caps 2D array for CLSID values */

} online_user;



/* single record for users info database (full version) size: ~2000 bytes */
/* It used when user want to receive full info and in db convertation     */
typedef struct full_user_info
{
    unsigned long        uin; 	/* user uin number 			  */
    short	     pemail1;   /* if email1 is public 			  */
    short	        auth;   /* authorization 			  */
    short                age;	/* user's age 				  */
    char              gender;	/* user's gender 			  */
    short	    disabled;   /* if user's account disabled 		  */
    short      can_broadcast;   /* if he can send broadcast messages 	  */
    int          ch_password;   /* if user need to change password 	  */
    char 	  gmt_offset;   /* user gmt time offset 		  */
    char	      e1publ;   /* if user don't want to publish email1   */
    char              bmonth;   /* user's birth day 			  */
    char      		bday;   /* user's bith month 			  */
    char 	      iphide;   /* if user doesn't want to show his ip 	  */
    char 	    webaware;   /* if user doesn't want to show webstatus */
    short    	       byear;   /* user's birth year 			  */
    long 	        wzip;   /* work cell phone 			  */
    short 	       wocup;   /* work ocupation 			  */
    long 	     wdepart;   /* work department 			  */
    short	    wcountry;	/* work country 			  */

    long    	     cr_date;   /* date of account creation 		  */
    long  	   lastlogin;   /* last user login time 		  */
    unsigned long    ip_addr;   /* last user's ip address 		  */
    unsigned long    nupdate;   /* notes update time 			  */

    short	    hcountry;	/* home country 			  */
    long 	        hzip;   /* home cell phone 			  */

    char 	       lang1;   /* speaking language #1 		  */
    char 	       lang2;   /* speaking language #2 		  */
    char 	       lang3;   /* speaking language #3 		  */
    char 	    hpage_cf;   /* hpage category flag (enabled/disabled) */
    short 	   hpage_cat;   /* homepage category index 		  */

    char 	    past_num;   /* number of filled past entries	  */
    short 	   past_ind1;   /* 1st entry past category index	  */
    short 	   past_ind2;   /* 2nd entry past category index	  */
    short 	   past_ind3;   /* 3rd entry past category index	  */

    char 	     int_num;   /* number of filled interests entries	  */
    short 	    int_ind1;   /* 1st entry interests category index	  */
    short 	    int_ind2;   /* 2nd entry interests category index	  */
    short 	    int_ind3;   /* 3rd entry interests category index	  */
    short 	    int_ind4;   /* 4th entry interests category index	  */

    char 	     aff_num;   /* number of filled affilations entries	  */
    short 	    aff_ind1;   /* 1st entry affilations category index	  */
    short 	    aff_ind2;   /* 2nd entry affilations category index	  */
    short 	    aff_ind3;   /* 3rd entry affilations category index	  */

    char          hpage[128];   /* www home page 			  */
    char      hpage_txt[128];   /* homepage description			  */
    char          hphone[64];	/* home phone 				  */
    char 	    hfax[64];   /* home fax 				  */
    char 	   hcell[64];   /* home cell phone 			  */

    char	   waddr[64];   /* work address 			  */
    char           wcity[32];	/* work city 				  */
    char          wstate[32];	/* work state 				  */
    char 	wcompany[64];   /* work company name 			  */

    char 	  wtitle[64];   /* work title 				  */
    char          wphone[64];	/* work phone 				  */
    char 	  wpager[64];   /* work cell phone 			  */
    char 	    wfax[64];   /* work fax 				  */
    char 	  wpage[128];	/* work www page 			  */
    char          passwd[32];	/* user's password 			  */
    char            nick[32];	/* user's nickname 			  */
    char            last[32]; 	/* user's lastname 			  */
    char           first[32]; 	/* user's firstname 			  */
    char          email1[64]; 	/* user's first email addres 		  */
    char          email2[64]; 	/* user's second email addres 		  */
    char          email3[64]; 	/* user's third email addres 		  */
    char	   haddr[64];   /* home address 			  */
    char           hcity[32];	/* home city 				  */
    char          hstate[32];	/* home state 				  */

    char 	int_key1[64];   /* interests entry #1 keywords		  */
    char 	int_key2[64];   /* interests entry #2 keywords		  */
    char 	int_key3[64];   /* interests entry #3 keywords		  */
    char 	int_key4[64];   /* interests entry #4 keywords		  */

    char 	aff_key1[64];   /* interests entry #1 keywords		  */
    char 	aff_key2[64];   /* interests entry #2 keywords		  */
    char 	aff_key3[64];   /* interests entry #3 keywords		  */

    char       past_key1[64];   /* past entry #1 keywords		  */
    char       past_key2[64];   /* past entry #2 keywords		  */
    char       past_key3[64];   /* past entry #3 keywords		  */

    char	wdepart2[64];	/* work department in string format	  */
    char          notes[255];	/* users notes 				  */

    unsigned short   martial;	/* user's martial status		  */
    char            nuin[32];	/* new string uin			  */
    char           wzip2[11];	/* new string wzip			  */
    char           hzip2[11];	/* new string hzip			  */
    short	    bcountry;	/* born country 			  */
    char          bstate[32];	/* born state 				  */
    char           bcity[32];	/* born city 				  */

} full_user_info;

/* partial user information - used in login handler */
typedef struct login_user_info
{
    unsigned long        uin; 	/* user uin number 			  */
    unsigned long    ip_addr;   /* last login ip address                  */
    int		     pemail1;   /* if email1 is public 			  */
    int		    disabled;   /* if user's account disabled 		  */
    int        can_broadcast;   /* if he can send broadcast messages 	  */
    int          ch_password;   /* if user need to change password 	  */
    char          passwd[32];	/* user's password 			  */
    short	        auth;   /* authorization 			  */

} login_user_info;


/* partial user information - used in send notes */
typedef struct notes_user_info
{
    unsigned long 	 uin;	/* notes owner uin number 		  */
    unsigned long    nupdate;   /* notes last update time 		  */
    unsigned long  lastlogin;   /* user last login time 		  */
    unsigned long    ip_addr;   /* user's last ip addr 			  */
    char          notes[1025];	/* user's notes 			  */

} notes_user_info;

/* information about single department */
typedef struct depart_info
{
    int		     depcode;	/* department unique code 		  */
    char           depmin[8];	/* department short name 		  */
    char	depname[255];   /* department full name 		  */

} depart_info;


/* information about found user */
typedef struct found_info
{
    unsigned long        uin; 	/* user uin number 			  */
    int		     pemail1;   /* if email1 is public 			  */
    int		        auth;   /* authorization 			  */
    char            nick[32];	/* user's nickname 			  */
    char            last[32]; 	/* user's lastname 			  */
    char           first[32]; 	/* user's firstname 			  */
    char          email1[64]; 	/* user's first email addres 		  */
    char          email2[64]; 	/* user's second email addres 		  */
    char          email3[64]; 	/* user's third email addres 		  */

} found_info;

/* for db_user_lookup function to get extended V5+ info from database */
typedef struct ext_user_info
{
    char 	       lang1;   /* speaking language #1 		  */
    char 	       lang2;   /* speaking language #2 		  */
    char 	       lang3;   /* speaking language #3 		  */
    short 	    hpage_cf;   /* hpage category flag (enabled/disabled) */
    short 	   hpage_cat;   /* homepage category index 		  */

    char 	    past_num;   /* number of filled past entries	  */
    short 	   past_ind1;   /* 1st entry past category index	  */
    short 	   past_ind2;   /* 2nd entry past category index	  */
    short 	   past_ind3;   /* 3rd entry past category index	  */

    char 	     int_num;   /* number of filled interests entries	  */
    short 	    int_ind1;   /* 1st entry interests category index	  */
    short 	    int_ind2;   /* 2nd entry interests category index	  */
    short 	    int_ind3;   /* 3rd entry interests category index	  */
    short 	    int_ind4;   /* 4th entry interests category index	  */

    char 	     aff_num;   /* number of filled affilations entries	  */
    short 	    aff_ind1;   /* 1st entry affilations category index	  */
    short 	    aff_ind2;   /* 2nd entry affilations category index	  */
    short 	    aff_ind3;   /* 3rd entry affilations category index	  */
    char      hpage_txt[128];   /* homepage description			  */

    char 	int_key1[64];   /* interests entry #1 keywords		  */
    char 	int_key2[64];   /* interests entry #2 keywords		  */
    char 	int_key3[64];   /* interests entry #3 keywords		  */
    char 	int_key4[64];   /* interests entry #4 keywords		  */

    char 	aff_key1[64];   /* interests entry #1 keywords		  */
    char 	aff_key2[64];   /* interests entry #2 keywords		  */
    char 	aff_key3[64];   /* interests entry #3 keywords		  */

    char       past_key1[64];   /* past entry #1 keywords		  */
    char       past_key2[64];   /* past entry #2 keywords		  */
    char       past_key3[64];   /* past entry #3 keywords		  */
    char	wdepart2[32];	/* work department in string format	  */

} ext_user_info;


#ifndef _MSG_HEADER
#define _MSG_HEADER
typedef struct msg_header
{
   unsigned short mkind;	/* message kind         */
   unsigned short mtype;	/* message type         */
   unsigned long  touin;	/* target uin number    */
   unsigned long  fromuin;      /* sender uin number    */
   unsigned long  fromindex;	/* sender shm index     */
   unsigned short seq2;		/* v3/v5 sequence2      */
   unsigned long  mtime;	/* send date/time       */
   unsigned short from_ver;	/* sender protocol ver  */
   char msg_cookie[8];		/* aim message cookie   */
   unsigned short fabt_ack;	/* abort/ack flag       */
   unsigned short f_abort;	/* file abort value     */
   unsigned short fok_ack;	/* file ok/ack flag     */
   unsigned long  int_ip;	/* user internal ipaddr */
   unsigned long  ext_ip;	/* user external ipaddr */
   unsigned short tport;	/* user dc tcp port     */
   unsigned short msgsize;	/* received snac size   */
   unsigned short mcharset;	/* message charset      */
   unsigned short mcsubset;     /* message char subset  */
   unsigned short msglen;	/* message str length   */

} msg_header;
#endif /* _MSG_HEADER */

