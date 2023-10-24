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

#include "includes.h"
#include "loadparm.h"

BOOL bLoaded = False;

#define GLOBAL_NAME "Globals";

#define pSERVICE(i) ServicePtrs[i]
#define iSERVICE(i) (*pSERVICE(i))
#define LP_SNUM_OK(iService) (((iService) >= 0) && \
	((iService) < iNumServices) && \
	iSERVICE(iService).valid)
#define VALID(i) iSERVICE(i).valid


/* structure to keep all conf data */
typedef struct
{
   char *szInterface;
   int   udp_port;
   char *szconfig_path;
   char *sztranslate_path;
   char *szdbglog_path;
   char *sztranslate_tbl;
   char *szact_config;
   char *szwwp_filename;
	
   int   max_log_size;
   BOOL  timestamp_log;
   BOOL  pid_in_logs;
   BOOL  append_log;
   BOOL  enable_actions;
   BOOL  all_ifaces;
   BOOL  realtime_odb;
   BOOL  proclist;
   int   loglevel;
   int   lumask;
   int   logperms;
   int   aim_port;
   int   msn_port;
   int   shared_mem_size;
   int   max_tcp_connections;

   int   max_childs;
   int   min_childs;
   int   defrag_timeout;
   int   online_timeout;

   char *szvar_path;
   char *szpid_path;
   char *szadmin_email;

   char *szv3_admin_notes;
   char *szv3_admin_notes_path;
   char *szv3_post_registration;
   char *szv3_post_reg_path;

   char *szdb_user;
   char *szdb_pass;
   char *szdb_addr;
   char *szdb_port;
   char *szdb_users;

   char *szinfo_pass;
    
   BOOL enable_watchdog;
   BOOL degradated_mode;
   BOOL restrict2luip;
   int  watchdog_timeout;
   int  cache_timeout;
   int  vacuum_timeout;

   BOOL v3_enabled;
   BOOL v3_reg_enabled;
   BOOL v3_autoregister;
   int v3_retries;
   int v3_timeout;
   int v3_pingtime;
   int default_ping;
   int v3_max_search;
   int v3_max_msgsize;
   int v3_packet_mtu;

   BOOL v5_enabled;	
   BOOL v5_reg_enabled;
   BOOL v5_autoregister;
   int v5_retries;
   int v5_timeout;
   int v5_pingtime;
   int v5_max_search;
   int v5_max_msgsize;
   int v3_split_order;

   BOOL v7_enabled;  
   BOOL v7_reg_enabled;
   BOOL v7_create_groups;
   BOOL v7_enable_import;
   int  v7_conn_timeout;
   int  v7_cookie_timeout;
   int  v7_max_search;
   int  v7_max_proflen;
   int  v7_max_contact_size;
   int  v7_max_watchers_size;
   int  v7_max_visible_size;
   int  v7_max_invisible_size;
   int  v7_max_ssi_groups;
   int  v7_max_ssi_ignore;
   int  v7_max_ssi_nonicq;
   int  v7_max_ssi_avatars;
   
   int  v7_dmax_channel;
   int  v7_dmax_msgsize;
   int  v7_dmax_sevil;
   int  v7_dmax_revil;
   int  v7_dmin_msg_interval;
   
   char *szv7_proto_config;
   char *szv7_bos_address;
   BOOL v7_accept_concurent;
   BOOL v7_direct_v3_connect;
   BOOL v7_direct_v5_connect;
   char *szv7_table_charset;
   
   char *szInclude;
   int   server_mode;
   int   deplist_vers;
   int   externals_num;
  	
} global;

static global Globals;

typedef struct
{

   BOOL valid;
   BOOL autoloaded;
   char *szService;
   BOOL *copymap;    
   char dummy[3];		/* for alignment */

}  service;


static service sDefault = {
   True,
   False,
   NULL,
   NULL,
   ""			/* dummy */
};

/* Local variables */
static service **ServicePtrs = NULL;
static int iNumServices = 0;
static int iServiceIndex = 0;
static BOOL bInGlobalSection = True;
static BOOL bGlobalOnly = True;

#define NUMPARAMETERS (sizeof(parm_table) / sizeof(struct parm_struct))

/* prototypes for the special type handlers */

BOOL handle_include(char *pszParmValue, char **ptr);

static struct enum_list enum_server_mode[] = { {MOD_STANDALONE, "STANDALONE"}, 
{MOD_DAEMON, "DAEMON"}, {-1, NULL} };

static struct enum_list enum_split_order[] = { {ORDER_FORWARD, "FORWARD"}, 
{ORDER_BACKWARD, "BACKWARD"}, {ORDER_DEFAULT, NULL} };

/* note that we do not initialise the defaults union - */
/* it is not allowed in ANSI C */
static struct parm_struct parm_table[] =
{
   {"Bind interface",   	P_STRING,  P_GLOBAL, &Globals.szInterface, NULL, NULL, 0},
   {"Listen port",      	P_INTEGER, P_GLOBAL, &Globals.udp_port, NULL, NULL, 0},
   {"Config path",      	P_STRING,  P_GLOBAL, &Globals.szconfig_path, NULL, NULL, 0},
   {"Enable AIM port",      	P_INTEGER, P_GLOBAL, &Globals.aim_port, NULL, NULL, 0},
   {"Enable MSN port",      	P_INTEGER, P_GLOBAL, &Globals.msn_port, NULL, NULL, 0},
   {"Shared memory size",	P_INTEGER, P_GLOBAL, &Globals.shared_mem_size, NULL, NULL, 0},
   {"Max tcp connections",	P_INTEGER, P_GLOBAL, &Globals.max_tcp_connections, NULL, NULL, 0},

   {"Translate path",   	P_STRING,  P_GLOBAL, &Globals.sztranslate_path, NULL, NULL, 0},
   {"Translate table",  	P_STRING,  P_GLOBAL, &Globals.sztranslate_tbl, handle_translate, NULL, 0},
   {"Actions config file",      P_STRING,  P_GLOBAL, &Globals.szact_config, NULL, NULL, 0},
   {"WWP socket filename",	P_STRING,  P_GLOBAL, &Globals.szwwp_filename, NULL, NULL, 0},
   {"Server mode",		P_ENUM,	   P_GLOBAL, &Globals.server_mode, NULL, enum_server_mode, 0},
   {"Admin email",      	P_STRING,  P_GLOBAL, &Globals.szadmin_email, NULL, NULL, 0},	
   {"Timestamp logs",   	P_BOOL,    P_GLOBAL, &Globals.timestamp_log, NULL, NULL, 0},
   {"Log process pid",  	P_BOOL,    P_GLOBAL, &Globals.pid_in_logs, NULL, NULL, 0},	
   {"Restrict access to LUIP",  P_BOOL,    P_GLOBAL, &Globals.restrict2luip, NULL, NULL, 0},	
   	
   {"Append logs",      	P_BOOL,    P_GLOBAL, &Globals.append_log, NULL, NULL, 0},	
   {"Realtime online db",      	P_BOOL,    P_GLOBAL, &Globals.realtime_odb, NULL, NULL, 0},	
   {"Log umask",        	P_INTEGER, P_GLOBAL, &Globals.lumask, NULL, NULL, 0},
   {"Maxlog size",      	P_INTEGER, P_GLOBAL, &Globals.max_log_size, NULL, NULL, 0},
   {"Debug level",      	P_INTEGER, P_GLOBAL, &DEBUGLEVEL,  NULL, NULL, 0},
   {"Log level",        	P_INTEGER, P_GLOBAL, &LOGLEVEL, NULL, NULL, 0},

   {"Enable watchdog",      	P_BOOL,    P_GLOBAL, &Globals.enable_watchdog, NULL, NULL, 0},
   {"Start without RDBMS", 	P_BOOL,    P_GLOBAL, &Globals.degradated_mode, NULL, NULL, 0},
   {"Enable actions",           P_BOOL,    P_GLOBAL, &Globals.enable_actions, NULL, NULL, 0},
   
   {"Bind on all interfaces",   P_BOOL,    P_GLOBAL, &Globals.all_ifaces, NULL, NULL, 0},
   
   {"Watchdog timeout",       	P_INTEGER, P_GLOBAL, &Globals.watchdog_timeout, NULL, NULL, 0},

   {"Var dir path",     	P_STRING,  P_GLOBAL, &Globals.szvar_path, NULL, NULL, 0},
   {"Pid file path",    	P_STRING,  P_GLOBAL, &Globals.szpid_path, NULL, NULL, 0},
      
   {"Max childs",       	P_INTEGER, P_GLOBAL, &Globals.max_childs, NULL, NULL, 0},
   {"Min childs",       	P_INTEGER, P_GLOBAL, &Globals.min_childs, NULL, NULL, 0},
   	
   {"Database user",    	P_STRING,  P_GLOBAL, &Globals.szdb_user, NULL, NULL, 0},
   {"Database password",	P_STRING,  P_GLOBAL, &Globals.szdb_pass, NULL, NULL, 0},
   {"Database addr",    	P_STRING,  P_GLOBAL, &Globals.szdb_addr, NULL, NULL, 0},	
   {"Database port",   		P_STRING,  P_GLOBAL, &Globals.szdb_port, NULL, NULL, 0},
   {"Users db name",    	P_STRING,  P_GLOBAL, &Globals.szdb_users, NULL, NULL, 0},
   	
   {"Info password",    	P_STRING,  P_GLOBAL, &Globals.szinfo_pass, NULL, NULL, 0},
   {"Vacuumdb timeout", 	P_INTEGER, P_GLOBAL, &Globals.vacuum_timeout, NULL, NULL, 0},
   {"Validate-cache timeout", 	P_INTEGER, P_GLOBAL, &Globals.cache_timeout, NULL, NULL, 0},
   {"Defrag db check period", 	P_INTEGER, P_GLOBAL, &Globals.defrag_timeout, NULL, NULL, 0}, 
   {"Online db check period", 	P_INTEGER, P_GLOBAL, &Globals.online_timeout, NULL, NULL, 0}, 
      
   {"V5 proto enabled", 	P_BOOL,    P_GLOBAL, &Globals.v5_enabled, NULL, NULL, 0},
   {"V5 max retries",   	P_INTEGER, P_GLOBAL, &Globals.v5_retries, NULL, NULL, 0},
   {"V5 max timeout",   	P_INTEGER, P_GLOBAL, &Globals.v5_timeout, NULL, NULL, 0},
   {"V5 ping time",     	P_INTEGER, P_GLOBAL, &Globals.v5_pingtime, NULL, NULL, 0},
   {"V5 max search",    	P_INTEGER, P_GLOBAL, &Globals.v5_max_search, NULL, NULL, 0},
   {"V5 max msgsize",   	P_INTEGER, P_GLOBAL, &Globals.v5_max_msgsize, NULL, NULL, 0},
   {"V3 msg split order", 	P_ENUM,	   P_GLOBAL, &Globals.v3_split_order, NULL, enum_split_order, 0},
   {"V5 registration enabled", 	P_BOOL,    P_GLOBAL, &Globals.v5_reg_enabled, NULL, NULL, 0},
   {"V5 auto registration",    	P_BOOL,    P_GLOBAL, &Globals.v5_autoregister, NULL, NULL, 0},

   {"V3 proto enabled", 	P_BOOL,    P_GLOBAL, &Globals.v3_enabled, NULL, NULL, 0},
   {"V3 registration enabled", 	P_BOOL,    P_GLOBAL, &Globals.v3_reg_enabled, NULL, NULL, 0},
   {"V3 auto registration",    	P_BOOL,    P_GLOBAL, &Globals.v3_autoregister, NULL, NULL, 0},
   {"V3 max retries",   	P_INTEGER, P_GLOBAL, &Globals.v3_retries, NULL, NULL, 0},
   {"V3 max timeout",   	P_INTEGER, P_GLOBAL, &Globals.v3_timeout, NULL, NULL, 0},
   {"V3 ping time",     	P_INTEGER, P_GLOBAL, &Globals.v3_pingtime, NULL, NULL, 0},
   {"V3 max search",    	P_INTEGER, P_GLOBAL, &Globals.v3_max_search, NULL, NULL, 0},
   {"V3 max msgsize",   	P_INTEGER, P_GLOBAL, &Globals.v3_max_msgsize, NULL, NULL, 0},
   {"V3 packet mtu",    	P_INTEGER, P_GLOBAL, &Globals.v3_packet_mtu, NULL, NULL, 0},
   {"V3 admin notes",         	P_STRING,  P_GLOBAL, &Globals.szv3_admin_notes_path, handle_v3_adm_notes, NULL, 0},
   {"V3 post-register info",  	P_STRING,  P_GLOBAL, &Globals.szv3_post_reg_path, handle_v3_post_reg, NULL, 0},

   {"V7 proto enabled", 	P_BOOL,    P_GLOBAL, &Globals.v7_enabled, NULL, NULL, 0},
   {"V7 registration enabled", 	P_BOOL,    P_GLOBAL, &Globals.v7_reg_enabled, NULL, NULL, 0},
   {"V7 connection timeout",   	P_INTEGER, P_GLOBAL, &Globals.v7_conn_timeout, NULL, NULL, 0},
   {"V7 cookie timeout",   	P_INTEGER, P_GLOBAL, &Globals.v7_cookie_timeout, NULL, NULL, 0},
   {"V7 accept concurent", 	P_BOOL,    P_GLOBAL, &Globals.v7_accept_concurent, NULL, NULL, 0},
   {"V7 create default groups", P_BOOL,    P_GLOBAL, &Globals.v7_create_groups, NULL, NULL, 0},
   {"V7 enable ssi import", 	P_BOOL,    P_GLOBAL, &Globals.v7_enable_import, NULL, NULL, 0},
   {"V7 direct v3 connect", 	P_BOOL,    P_GLOBAL, &Globals.v7_direct_v3_connect, NULL, NULL, 0},
   {"V7 direct v5 connect", 	P_BOOL,    P_GLOBAL, &Globals.v7_direct_v5_connect, NULL, NULL, 0},
   {"V7 proto config",  	P_STRING,  P_GLOBAL, &Globals.szv7_proto_config, NULL, NULL, 0},
   {"V7 BOS address",  		P_STRING,  P_GLOBAL, &Globals.szv7_bos_address, NULL, NULL, 0},
   {"V7 max search",    	P_INTEGER, P_GLOBAL, &Globals.v7_max_search, NULL, NULL, 0},
   {"V7 max profile length",   	P_INTEGER, P_GLOBAL, &Globals.v7_max_proflen, NULL, NULL, 0},
   {"V7 max contact size",    	P_INTEGER, P_GLOBAL, &Globals.v7_max_contact_size, NULL, NULL, 0},
   {"V7 max watchers size",    	P_INTEGER, P_GLOBAL, &Globals.v7_max_watchers_size, NULL, NULL, 0},
   {"V7 max visible size",    	P_INTEGER, P_GLOBAL, &Globals.v7_max_visible_size, NULL, NULL, 0},
   {"V7 max invisible size",   	P_INTEGER, P_GLOBAL, &Globals.v7_max_invisible_size, NULL, NULL, 0},
   {"V7 max ssi groups",   	P_INTEGER, P_GLOBAL, &Globals.v7_max_ssi_groups, NULL, NULL, 0},
   {"V7 max ssi ignore",   	P_INTEGER, P_GLOBAL, &Globals.v7_max_ssi_ignore, NULL, NULL, 0},   
   {"V7 max ssi non-icq",   	P_INTEGER, P_GLOBAL, &Globals.v7_max_ssi_nonicq, NULL, NULL, 0},   
   {"V7 max ssi avatars",   	P_INTEGER, P_GLOBAL, &Globals.v7_max_ssi_avatars, NULL, NULL, 0},

   {"V7 default max channel",  	P_INTEGER, P_GLOBAL, &Globals.v7_dmax_channel, NULL, NULL, 0},
   {"V7 default max msgsize",  	P_INTEGER, P_GLOBAL, &Globals.v7_dmax_msgsize, NULL, NULL, 0},
   {"V7 default max sevil",  	P_INTEGER, P_GLOBAL, &Globals.v7_dmax_sevil, NULL, NULL, 0},
   {"V7 default max revil",  	P_INTEGER, P_GLOBAL, &Globals.v7_dmax_revil, NULL, NULL, 0},
   {"V7 default mm interval",  	P_INTEGER, P_GLOBAL, &Globals.v7_dmin_msg_interval, NULL, NULL, 0},
   {"V7 table charset",		P_STRING,  P_GLOBAL, &Globals.szv7_table_charset, NULL, NULL, 0},

   {"Default ping time",	P_INTEGER, P_GLOBAL, &Globals.default_ping, NULL, NULL, 0},
   {"Depart list version", 	P_INTEGER, P_GLOBAL, &Globals.deplist_vers, NULL, NULL, 0},
   {"Externals number", 	P_INTEGER, P_GLOBAL, &Globals.externals_num, NULL, NULL, 0},

   {"Include", 	     		P_STRING,  P_GLOBAL, &Globals.szInclude, handle_include, NULL, 0},

   {NULL, P_BOOL, P_NONE, NULL, NULL, NULL, 0}

};

/**************************************************************************/
/* Standart substitutions in given string 				  */
/**************************************************************************/
void standard_sub_basic(char *str)
{
/* COMMENTED_OUT: This part is not used yet */

#if 0
   char *p, *s;
   fstring pidstr;

   for (s=str; (p=strchr(s, '%'));s=p)
   {
      int l = sizeof(pstring) - (int)(p-str);
		
      switch (*(p+1))
      {
         case 'I' : string_sub(p,"%I", client_addr(),l); break;
	 case '$' : p += expand_env_var(p,l); break;
	 case '\0': p++; break;

         default  : p+=2; break;
      }
   } 
#endif

}

/**************************************************************************/
/* Used to substitute %U% and %P% with uin number and password		  */
/**************************************************************************/
void subst_post_register(char *str, unsigned long uin, char* passwd)
{
   char *p, *s;
   char suin[32];
	
   snprintf(suin, 32, "%lu", uin);
	
   for (s=str; (p=strchr(s, '%'));s=p)
   {
      int l = sizeof(pstring) - (int)(p-str);
	
      switch (*(p+1))
      {
         case 'U' : string_sub(p,"%U%", suin,l); break;
	 case 'P' : string_sub(p,"%P%", passwd,l); break;
	 case '\0': p++; break;

         default  : p+=2; break;
      }
   } 
}


/**************************************************************************/
/* Initialise the global parameter structure.				  */
/**************************************************************************/
void init_globals(void)
{
   static BOOL done_init = False;

   if (!done_init)
   {
      int i;
      memset((void *)&Globals, '\0', sizeof(Globals));

      for (i = 0; parm_table[i].label; i++)
         if ((parm_table[i].type == P_STRING) && parm_table[i].ptr)
            string_set((char **)parm_table[i].ptr, "");

      done_init = True;
   }

   string_set(&Globals.szconfig_path, 	  ICQ_CONFIG_FILE);
   string_set(&Globals.sztranslate_path,  ICQ_TRANSLATE_PATH);
   string_set(&Globals.szdbglog_path, 	  ICQ_DBGLOG_FILE);
   string_set(&Globals.sztranslate_tbl,  "DEFAULT");

   string_set(&Globals.szvar_path, 	  ICQ_VAR_DIR);
   string_set(&Globals.szadmin_email, 	 "None specified");
   string_set(&Globals.szpid_path,  	  ICQ_PID_FILE);
   string_set(&Globals.szdb_user,  	  ICQ_DB_USER);
   string_set(&Globals.szinfo_pass, 	 "DEFAULT");
   string_set(&Globals.szact_config, 	  ACTIONS_CONF);
   string_set(&Globals.szwwp_filename, 	  WWP_SOCK_FILENAME);
   string_set(&Globals.szv7_proto_config, "aim_proto.conf");
   string_set(&Globals.szv7_bos_address,  "127.0.0.1:5190");
   string_set(&Globals.szv7_table_charset,  "koi8-r");

   string_set(&Globals.szv3_admin_notes, "There are no notes... :(");
   string_set(&Globals.szv3_post_registration, "Registration complete...");
	
   Globals.default_ping     	= DEFAULT_PING_TIME;
   Globals.cache_timeout    	= VALIDATE_CACHE_TIMEOUT;
   Globals.vacuum_timeout 	= VACUUM_DB_TIMEOUT;
   Globals.defrag_timeout 	= DEFRAG_TIMEOUT;
   Globals.online_timeout 	= ONLINE_TIMEOUT;
   Globals.enable_watchdog 	= False;
   Globals.watchdog_timeout	= WATCHDOG_TIMEOUT;
   Globals.max_tcp_connections  = MAX_TCP_CONNECTIONS;
   Globals.degradated_mode	= True;
   Globals.enable_actions  	= False;
   Globals.all_ifaces	   	= False;
   Globals.restrict2luip	= False;
	
   Globals.v3_enabled       	= True;
   Globals.v3_pingtime      	= V3_PING_TIME;
   Globals.v3_max_search    	= V3_MAX_SEARCH;
   Globals.v3_timeout       	= V3_TIMEOUT;
   Globals.v3_retries       	= V3_RETRIES;
   Globals.v3_max_msgsize   	= V3_MAX_MESSAGE;
   Globals.v3_packet_mtu    	= V3_PACKET_MTU;
   Globals.v3_reg_enabled   	= True;
   Globals.v3_autoregister  	= False;
   Globals.deplist_vers     	= DEP_LIST_VERS;
   Globals.externals_num    	= EXTERNALS_NUM;
   Globals.pid_in_logs      	= False;
   Globals.shared_mem_size      = 512000;
   Globals.realtime_odb		= False;
   
   Globals.v5_enabled       	= True;
   Globals.v5_reg_enabled   	= True;
   Globals.v5_autoregister  	= False;
   Globals.v5_pingtime      	= V5_PING_TIME;
   Globals.v5_timeout       	= V5_TIMEOUT;
   Globals.v5_retries       	= V5_RETRIES;
   Globals.v5_max_search    	= V5_MAX_SEARCH;
   Globals.v5_max_msgsize   	= V5_MAX_MESSAGE;
   Globals.v3_split_order   	= ORDER_FORWARD;

   Globals.v7_enabled		= True;
   Globals.v7_reg_enabled   	= False;
   Globals.v7_conn_timeout	= V7_CONN_TIMEOUT;
   Globals.v7_cookie_timeout	= V7_COOKIE_TIMEOUT;
   Globals.v7_accept_concurent	= False;
   Globals.v7_direct_v3_connect	= False;
   Globals.v7_direct_v5_connect	= True;
   Globals.v7_max_search    	= V5_MAX_SEARCH;
   Globals.v7_max_proflen    	= V7_MAX_PROFLEN;
   Globals.v7_max_contact_size 	= V7_MAX_CONTACT_SIZE;
   Globals.v7_max_watchers_size	= V7_MAX_WATCHERS_SIZE;
   Globals.v7_max_visible_size 	= V7_MAX_VISIBLE_SIZE;
   Globals.v7_max_invisible_size= V7_MAX_INVISIBLE_SIZE;
   Globals.v7_max_ssi_ignore    = 128;
   Globals.v7_max_ssi_groups    = 51;
   Globals.v7_max_ssi_nonicq    = 200;
   Globals.v7_max_ssi_avatars   = 0; /* not ready yet - disabled */
   Globals.v7_create_groups     = True;
   Globals.v7_enable_import     = True;

   Globals.v7_dmax_channel	= 0;
   Globals.v7_dmax_msgsize	= 512;
   Globals.v7_dmax_sevil	= 999;
   Globals.v7_dmax_revil	= 999;
   Globals.v7_dmin_msg_interval	= 0;
   
   Globals.szdb_user        	= NULL;
   Globals.szdb_pass        	= NULL;
   Globals.szdb_addr        	= NULL;
   Globals.szdb_port        	= NULL;
   Globals.szdb_users       	= NULL;
   Globals.logperms         	= ICQ_LOG_PERMS;
   Globals.udp_port         	= UDP_PORT;
   Globals.aim_port	    	= AIM_PORT;
   Globals.msn_port	    	= MSN_PORT;
   Globals.max_log_size     	= LOG_UNLIMITED;
   Globals.server_mode      	= MOD_DAEMON;
   Globals.loglevel         	= 0;

   Globals.timestamp_log    	= False;
   Globals.append_log       	= True;
   Globals.lumask           	= 022;
   Globals.max_childs       	= 5;
   Globals.min_childs       	= 1;
   Globals.proclist		= False;

}


TALLOC_CTX *lp_talloc;


/**************************************************************************/
/* Translate table initialization.                             	  	  */
/**************************************************************************/
void init_translate()
{
   char temp[128];
   
   snprintf(temp, 128, "%s/%s", lp_translate_file(), lp_translate_tbl());
   ITrans.setTranslationMap(temp);
}


/**************************************************************************/
/* free up temporary memory - called from the main loop			  */
/**************************************************************************/
void lp_talloc_free(void)
{
   if (!lp_talloc) return;
   talloc_destroy(lp_talloc);
   lp_talloc = NULL;
}

/**************************************************************************/
/* convenience routine to grab string parameters into temporary 	  */
/* memory and run standard_sub_basic on them. The buffers can be  	  */
/* written to by callers without affecting the source string.	  	  */
/**************************************************************************/
char *lp_string(const char *s)
{
   size_t len = s ? strlen(s) : 0;
   char *ret;

   if (!lp_talloc)  lp_talloc = talloc_init();

   /* leave room for substitution */
   ret = (char *)talloc(lp_talloc, len + 100);	

   if (!ret) return NULL;

   if (!s)
      *ret = 0;
   else
      StrnCpy(ret, s, len);

   trim_string(ret, "\"", "\"");

   standard_sub_basic(ret);
   return (ret);
}


/* In this section all the functions that are used to access the */
/* parameters from the rest of the program are defined 		 */

#define FN_GLOBAL_STRING(fn_name,ptr) char *fn_name(void) {return(lp_string(*(char **)(ptr) ? *(char **)(ptr) : ""));}
#define FN_GLOBAL_BOOL(fn_name,ptr) BOOL fn_name(void) {return(*(BOOL *)(ptr));}
#define FN_GLOBAL_CHAR(fn_name,ptr) char fn_name(void) {return(*(char *)(ptr));}
#define FN_GLOBAL_INTEGER(fn_name,ptr) int fn_name(void) {return(*(int *)(ptr));}

/*------------------- lp wrappers definitions ---------------*/
FN_GLOBAL_STRING(lp_config_file,     &Globals.szconfig_path)
FN_GLOBAL_STRING(lp_translate_file,  &Globals.sztranslate_path)
FN_GLOBAL_STRING(lp_translate_tbl,   &Globals.sztranslate_tbl)
FN_GLOBAL_STRING(lp_dbglog_path,     &Globals.szdbglog_path)
FN_GLOBAL_STRING(lp_admin_email,     &Globals.szadmin_email)
FN_GLOBAL_STRING(lp_actions_conf,    &Globals.szact_config)
FN_GLOBAL_STRING(lp_wwp_socketname,  &Globals.szwwp_filename)
FN_GLOBAL_STRING(lp_v7_proto_config, &Globals.szv7_proto_config)

FN_GLOBAL_BOOL(lp_timestamp_logs,    &Globals.timestamp_log)
FN_GLOBAL_BOOL(lp_append_logs, 	     &Globals.append_log)
FN_GLOBAL_BOOL(lp_pid_in_logs, 	     &Globals.pid_in_logs)

FN_GLOBAL_BOOL(lp_bind_all_ifaces,   &Globals.all_ifaces)
FN_GLOBAL_BOOL(lp_realtime_odb,      &Globals.realtime_odb)
FN_GLOBAL_BOOL(lp_restrict2luip,     &Globals.restrict2luip)

FN_GLOBAL_INTEGER(lp_sched_vacuum,   &Globals.vacuum_timeout)
FN_GLOBAL_INTEGER(lp_sched_cvalid,   &Globals.cache_timeout)
FN_GLOBAL_INTEGER(lp_shm_size,       &Globals.shared_mem_size)
FN_GLOBAL_INTEGER(lp_max_tcp_connections, &Globals.max_tcp_connections)

FN_GLOBAL_INTEGER(lp_umask,  	     &Globals.lumask)
FN_GLOBAL_INTEGER(lp_lperms,  	     &Globals.logperms)

FN_GLOBAL_INTEGER(lp_max_childs,     &Globals.max_childs)
FN_GLOBAL_INTEGER(lp_min_childs,     &Globals.min_childs)

FN_GLOBAL_STRING(lp_var_path,        &Globals.szvar_path)
FN_GLOBAL_STRING(lp_pid_path,        &Globals.szpid_path)
FN_GLOBAL_STRING(lp_interface,       &Globals.szInterface)

FN_GLOBAL_STRING(lp_db_user,    &Globals.szdb_user)
FN_GLOBAL_STRING(lp_db_pass,    &Globals.szdb_pass)
FN_GLOBAL_STRING(lp_db_addr,    &Globals.szdb_addr)
FN_GLOBAL_STRING(lp_db_port,    &Globals.szdb_port)
FN_GLOBAL_STRING(lp_db_users,   &Globals.szdb_users)
FN_GLOBAL_STRING(lp_info_passwd,&Globals.szinfo_pass)

FN_GLOBAL_INTEGER(lp_default_ping,  &Globals.default_ping)
FN_GLOBAL_INTEGER(lp_deplist_vers,  &Globals.deplist_vers)
FN_GLOBAL_INTEGER(lp_externals_num, &Globals.externals_num)
FN_GLOBAL_INTEGER(lp_defrag_check, &Globals.defrag_timeout)
FN_GLOBAL_INTEGER(lp_online_check, &Globals.online_timeout)

FN_GLOBAL_BOOL   (lp_watchdog_enabled, &Globals.enable_watchdog)
FN_GLOBAL_BOOL   (lp_degradated_mode, &Globals.degradated_mode)
FN_GLOBAL_BOOL   (lp_actions_enabled, &Globals.enable_actions)
FN_GLOBAL_BOOL   (lp_proclist, &Globals.proclist)
FN_GLOBAL_INTEGER(lp_watchdog_timeout, &Globals.watchdog_timeout)

/*--------V3 proto specific variables ------------------*/

FN_GLOBAL_BOOL   (lp_v3_enabled,     &Globals.v3_enabled)
FN_GLOBAL_BOOL   (lp_v3_registration_enabled, &Globals.v3_reg_enabled)
FN_GLOBAL_BOOL   (lp_v3_autoregister, &Globals.v3_autoregister)

FN_GLOBAL_INTEGER(lp_v3_retries,     &Globals.v3_retries)
FN_GLOBAL_INTEGER(lp_v3_timeout,     &Globals.v3_timeout)
FN_GLOBAL_INTEGER(lp_v3_pingtime,    &Globals.v3_pingtime)
FN_GLOBAL_INTEGER(lp_v3_maxsearch,   &Globals.v3_max_search)
FN_GLOBAL_INTEGER(lp_v3_max_msgsize, &Globals.v3_max_msgsize)
FN_GLOBAL_INTEGER(lp_v3_packet_mtu,  &Globals.v3_packet_mtu)
FN_GLOBAL_STRING (lp_v3_admin_notes, &Globals.szv3_admin_notes)
FN_GLOBAL_STRING (lp_v3_post_registration, &Globals.szv3_post_registration)

/*--------V5 proto specific variables -------------------*/

FN_GLOBAL_BOOL   (lp_v5_enabled,     &Globals.v5_enabled)
FN_GLOBAL_BOOL   (lp_v5_registration_enabled, &Globals.v5_reg_enabled)
FN_GLOBAL_BOOL   (lp_v5_autoregister, &Globals.v5_autoregister)

FN_GLOBAL_INTEGER(lp_v5_retries,     &Globals.v5_retries)
FN_GLOBAL_INTEGER(lp_v5_timeout,     &Globals.v5_timeout)
FN_GLOBAL_INTEGER(lp_v5_pingtime,    &Globals.v5_pingtime)
FN_GLOBAL_INTEGER(lp_v5_maxsearch,   &Globals.v5_max_search)
FN_GLOBAL_INTEGER(lp_v5_max_msgsize, &Globals.v5_max_msgsize)
FN_GLOBAL_INTEGER(lp_v3_split_order, &Globals.v3_split_order)

/*--------V7 proto specific variables -------------------*/
FN_GLOBAL_BOOL   (lp_v7_enabled,          &Globals.v7_enabled)
FN_GLOBAL_BOOL   (lp_v7_registration_enabled, &Globals.v7_reg_enabled)
FN_GLOBAL_BOOL   (lp_v7_accept_concurent, &Globals.v7_accept_concurent)
FN_GLOBAL_BOOL   (lp_v7_direct_v3_connect,&Globals.v7_direct_v3_connect)
FN_GLOBAL_BOOL   (lp_v7_direct_v5_connect,&Globals.v7_direct_v5_connect)
FN_GLOBAL_BOOL   (lp_v7_create_groups,    &Globals.v7_create_groups)
FN_GLOBAL_BOOL   (lp_v7_enable_ssi_import,&Globals.v7_enable_import)

FN_GLOBAL_INTEGER(lp_v7_conn_timeout,     &Globals.v7_conn_timeout)
FN_GLOBAL_INTEGER(lp_v7_cookie_timeout,   &Globals.v7_cookie_timeout)
FN_GLOBAL_INTEGER(lp_v7_maxsearch,        &Globals.v7_max_search)
FN_GLOBAL_INTEGER(lp_v7_max_proflen,      &Globals.v7_max_proflen)
FN_GLOBAL_INTEGER(lp_v7_max_contact_size, &Globals.v7_max_contact_size)
FN_GLOBAL_INTEGER(lp_v7_max_watchers_size,&Globals.v7_max_watchers_size)
FN_GLOBAL_INTEGER(lp_v7_max_visible_size, &Globals.v7_max_visible_size)
FN_GLOBAL_INTEGER(lp_v7_max_invisible_size,&Globals.v7_max_invisible_size)
FN_GLOBAL_INTEGER(lp_v7_max_ssi_ignore,   &Globals.v7_max_ssi_ignore)
FN_GLOBAL_INTEGER(lp_v7_max_ssi_groups,   &Globals.v7_max_ssi_groups)
FN_GLOBAL_INTEGER(lp_v7_max_ssi_nonicq,   &Globals.v7_max_ssi_nonicq)
FN_GLOBAL_INTEGER(lp_v7_max_ssi_avatars,  &Globals.v7_max_ssi_avatars)
FN_GLOBAL_STRING (lp_v7_bos_address,      &Globals.szv7_bos_address)

FN_GLOBAL_INTEGER(lp_v7_dmax_channel, &Globals.v7_dmax_channel)
FN_GLOBAL_INTEGER(lp_v7_dmax_msgsize, &Globals.v7_dmax_msgsize)
FN_GLOBAL_INTEGER(lp_v7_dmax_sevil,   &Globals.v7_dmax_sevil)
FN_GLOBAL_INTEGER(lp_v7_dmax_revil,   &Globals.v7_dmax_revil)
FN_GLOBAL_INTEGER(lp_v7_dmin_msg_interval, &Globals.v7_dmin_msg_interval)

FN_GLOBAL_INTEGER(lp_log_size,       &Globals.max_log_size)
FN_GLOBAL_INTEGER(lp_udp_port,       &Globals.udp_port)
FN_GLOBAL_INTEGER(lp_aim_port,       &Globals.aim_port)
FN_GLOBAL_INTEGER(lp_msn_port,       &Globals.msn_port)

FN_GLOBAL_INTEGER(lp_log_level,	     &Globals.loglevel)
FN_GLOBAL_INTEGER(lp_server_mode,    &Globals.server_mode)

FN_GLOBAL_STRING (lp_v7_table_charset,	&Globals.szv7_table_charset)

/* local prototypes */
int strwicmp(char *psz1, char *psz2);
int map_parameter(char *pszParmName);
BOOL set_boolean(BOOL *pb, char *pszParmValue);
int getservicebyname(char *pszServiceName, service * pserviceDest);
void copy_service(service * pserviceDest,  service * pserviceSource, BOOL *pcopymapDest);
BOOL service_ok(int iService);
BOOL do_parameter(char *pszParmName, char *pszParmValue);
BOOL do_section(char *pszSectionName);
void init_copymap(service * pservice);


/**************************************************************************/
/* initialise a service to the defaults					  */
/**************************************************************************/
void init_service(service * pservice)
{
   memset((char *)pservice, '\0', sizeof(service));
   copy_service(pservice, &sDefault, NULL);
}

/**************************************************************************/
/* free the dynamically allocated parts of a service struct		  */
/**************************************************************************/
void free_service(service * pservice)
{
   int i;
   if (!pservice) return;
   if (pservice->szService)

   string_free(&pservice->szService);
   if (pservice->copymap)
   {
      free(pservice->copymap);
      pservice->copymap = NULL;
   }
   
   for (i = 0; parm_table[i].label; i++)
      if ((parm_table[i].type == P_STRING) && parm_table[i].pclass == P_LOCAL)
         string_free((char **)(((char *)pservice) +
         PTR_DIFF(parm_table[i].ptr, &sDefault)));
}

/**************************************************************************/
/* add a new service to the services array initialising it with		  */
/* the given section                                                      */
/**************************************************************************/
int add_a_service(service * pservice, char *name)
{
   int i;
   service tservice;
   int num_to_alloc = iNumServices + 1;

   tservice = *pservice;

   /* it might already exist */
   if (name)
   {
      i = getservicebyname(name, NULL);
      if (i >= 0)  return (i);
   }

   /* find an invalid one */
   for (i = 0; i < iNumServices; i++)
      if (!pSERVICE(i)->valid) break;

   /* if not, then create one */
   if (i == iNumServices)
   {
      ServicePtrs = (service **) Realloc(ServicePtrs, sizeof(service *) *
		     num_to_alloc);

      if (ServicePtrs) pSERVICE(iNumServices) =
		       (service *) malloc(sizeof(service));

      if (!ServicePtrs || !pSERVICE(iNumServices))
			return (-1);

      iNumServices++;
   }
   else
      free_service(pSERVICE(i));

   pSERVICE(i)->valid = True;
   init_service(pSERVICE(i));
   copy_service(pSERVICE(i), &tservice, NULL);

   if (name)
   {
      string_set(&iSERVICE(i).szService, name);
   }

   return (i);
}

/**************************************************************************/
/* add a new service, based on an old one				  */
/**************************************************************************/
int lp_add_service(char *pszService, int iDefaultService)
{
   return (add_a_service(pSERVICE(iDefaultService), pszService));
}


/**************************************************************************/
/* Do a case-insensitive, whitespace-ignoring string compare.		  */
/**************************************************************************/
int strwicmp(char *psz1, char *psz2)
{
   /* if BOTH strings are NULL, return TRUE, if ONE is NULL return */
   /* appropriate value. */
   if (psz1 == psz2) return (0);
   else if (psz1 == NULL) return (-1);
   else if (psz2 == NULL) return (1);

   /* sync the strings on first non-whitespace */
   while (1)
   {
      while (isspace(*psz1)) psz1++;
      while (isspace(*psz2)) psz2++;

      if (toupper(*psz1) != toupper(*psz2) ||
          *psz1 == '\0' ||
          *psz2 == '\0')  break;

      psz1++;
      psz2++;
   }
   return (*psz1 - *psz2);
}

/**************************************************************************/
/* Map a parameter's string representation to something we can use. 	  */
/* Returns False if the parameter string is not recognised, else TRUE.	  */
/**************************************************************************/
int map_parameter(char *pszParmName)
{
   int iIndex;

   if (*pszParmName == '-') return (-1);

   for (iIndex = 0; parm_table[iIndex].label; iIndex++)
      if (strwicmp(parm_table[iIndex].label, pszParmName) == 0)
         return (iIndex);

   LOG_SYS(0, ("Error: Unknown parameter in server config: \"%s\"\n", pszParmName));

   return (-1);
}


/**************************************************************************/
/* Set a boolean variable from the text value stored in the passed string */
/* Returns True in success, False if the passed string does not correctly */
/* represent a boolean.							  */
/**************************************************************************/
BOOL set_boolean(BOOL *pb, char *pszParmValue)
{
   BOOL bRetval;

   bRetval = True;
   if (strwicmp(pszParmValue, "yes") == 0 ||
       strwicmp(pszParmValue, "true") == 0 ||
       strwicmp(pszParmValue, "1") == 0)  *pb = True;
   else
   if (strwicmp(pszParmValue, "no") == 0 ||
       strwicmp(pszParmValue, "False") == 0 ||
       strwicmp(pszParmValue, "0") == 0) *pb = False;
   else
   {
      LOG_SYS(0, ("Error: Bad boolean in config file: \"%s\".\n", 
                   pszParmValue));

		bRetval = False;
   }
   return (bRetval);
}


/**************************************************************************/
/* Find a service by name. Otherwise works like get_service.		  */
/**************************************************************************/
int getservicebyname(char *pszServiceName, service * pserviceDest)
{
   int iService;

   for (iService = iNumServices - 1; iService >= 0; iService--)
      if (VALID(iService) &&
         strwicmp(iSERVICE(iService).szService, pszServiceName) == 0)
      {
         if (pserviceDest != NULL)
            copy_service(pserviceDest, pSERVICE(iService), NULL);
         break;
      }

   return (iService);
}

/****************************************************************************/
/* Copy a service structure to another                                      */
/* If pcopymapDest is NULL then copy all fields                             */
/****************************************************************************/
void copy_service(service * pserviceDest, service * pserviceSource, 
		  BOOL *pcopymapDest)
{
   int i;
   BOOL bcopyall = (pcopymapDest == NULL);

   for (i = 0; parm_table[i].label; i++)
      if (parm_table[i].ptr && parm_table[i].pclass == P_LOCAL &&
         (bcopyall || pcopymapDest[i]))
      {
         void *def_ptr  = parm_table[i].ptr;
         void *src_ptr  = ((char *)pserviceSource)
                        + PTR_DIFF(def_ptr, &sDefault);
	 void *dest_ptr = ((char *)pserviceDest)
	                + PTR_DIFF(def_ptr, &sDefault);
         switch (parm_table[i].type)
         {
             case P_BOOL:     *(BOOL *)dest_ptr = *(BOOL *)src_ptr;
                              break;
             case P_INTEGER:
             case P_ENUM:     *(int *)dest_ptr = *(int *)src_ptr;
                              break;
             case P_CHAR:     *(char *)dest_ptr = *(char *)src_ptr;
                              break;
             case P_STRING:   string_set((char **)dest_ptr,
                              *(char **)src_ptr); break;

             default:         break;
         }
      }

   if (bcopyall)
   {
      if (pserviceSource->copymap)
          memcpy((void *)pserviceDest->copymap,
                 (void *)pserviceSource->copymap,
                 sizeof(BOOL) * NUMPARAMETERS);
      }
}


/**************************************************************************/
/* Check a service for consistency. Return False if the service is in     */
/* any way incomplete or faulty, else True.				  */
/***************************************************************************/
BOOL service_ok(int iService)
{
   BOOL bRetval;

   bRetval = True;
   if (iSERVICE(iService).szService[0] == '\0')
   {
      bRetval = False;
   }

   return (bRetval);
}


static struct file_lists
{
   struct file_lists *next;
   char *name;
   time_t modtime;
}


*file_lists = NULL;


/**************************************************************************/
/* Keep a linked list of all config files so we know when one has 	  */
/* changed it's date and needs to be reloaded				  */
/**************************************************************************/
void add_to_file_list(char *fname)
{
   struct file_lists *f = file_lists;

   while (f)
   {
      if (f->name && !strcmp(f->name, fname)) break;
      f = f->next;
   }

   if (!f)
   {
      f = (struct file_lists *)malloc(sizeof(file_lists[0]));
      if (!f) return;
      f->next = file_lists;
      f->name = strdup(fname);
      if (!f->name)
      {
         free(f);
         return;
      }

      file_lists = f;
   }

   {
      pstring n2;
      pstrcpy(n2, fname);
      standard_sub_basic(n2);
      f->modtime = file_modtime(n2);
   }
}

/**************************************************************************/
/*  Check if a config file has changed date				  */
/**************************************************************************/
BOOL lp_file_list_changed(void)
{
   struct file_lists *f = file_lists;

   DEBUG(6, ("lp_file_list_changed\n"));
	
   while (f)
   {
      pstring n2;
      time_t mod_time;

      pstrcpy(n2, f->name);
      standard_sub_basic(n2);
      DEBUGADD(6, ("file %s -> %s  last mod_time: %s\n",
      		    f->name, n2, ctime(&f->modtime)));


      mod_time = file_modtime(n2);

      if (f->modtime != mod_time)
      {
         DEBUGADD(6, ("file %s modified: %s\n", n2, ctime(&mod_time)));

	 f->modtime = mod_time;
	 return (True);
      }

      f = f->next;
   }
   return (False);
}


/**************************************************************************/
/* Do the work of sourcing in environment variable/value pairs.		  */
/**************************************************************************/
BOOL source_env(char **lines)
{
   char *varval;
   size_t len;
   int i;
   char *p;

   for (i = 0; lines[i]; i++)
   {
      char *line = lines[i];
      if ((len = strlen(line)) == 0) continue;

      if (line[len - 1] == '\n') line[--len] = '\0';

      if ((varval = (char *)malloc(len + 1)) == NULL)
      {
         DEBUG(10, ("ERROR: Source_env: Not enough memory!\n"));
	 return (False);
      }

      DEBUG(4, ("Source_env: Adding to environment: %s\n", line));
      strncpy(varval, line, len);
      varval[len] = '\0';

      p = strchr(line, (int)'=');
      if (p == NULL)
      {
         DEBUG(4, ("Source_env: missing '=': %s\n", line));
	 continue;
      }

      if (putenv(varval))
      {
         DEBUG(10, ("Source_env: Failed to put environment variable %s\n", varval));
	 continue;
      }

      *p = '\0';
      p++;
      DEBUG(4, ("Source_env: getting var %s = %s\n", line, getenv(line)));

   }

   return (True);
}


/**************************************************************************/
/* initialise a copymap							  */
/**************************************************************************/
void init_copymap(service * pservice)
{
   unsigned int i;
   if (pservice->copymap) free(pservice->copymap);
       pservice->copymap = (BOOL *)malloc(sizeof(BOOL) * NUMPARAMETERS);

   if (!pservice->copymap) { }
   else for (i = 0; i < NUMPARAMETERS; i++) pservice->copymap[i] = True;
}


/**************************************************************************/
/* return the local pointer to a parameter given the service number and   */
/* the pointer into the default structure				  */
/**************************************************************************/
void *lp_local_ptr(int snum, void *ptr)
{
   return (void *)(((char *)pSERVICE(snum)) + PTR_DIFF(ptr, &sDefault));
}

/**************************************************************************/
/* Process a parameter for a particular service number. If snum < 0	  */
/* then assume we are in the globals					  */
/**************************************************************************/
BOOL lp_do_parameter(int snum, char *pszParmName, char *pszParmValue)
{
   int parmnum, i;
   void *parm_ptr = NULL;	/* where we are going to store the result */
   void *def_ptr = NULL;
   
   parmnum = map_parameter(pszParmName);

   if (parmnum < 0)
   {
      DEBUG(10,("Ignoring unknown parameter \"%s\"\n", pszParmName));
      return (True);
   }

   def_ptr = parm_table[parmnum].ptr;

   /* we might point at a service, the default service or a global */
   if (snum < 0)
   {
      parm_ptr = def_ptr;
   }
   else
   {
      if (parm_table[parmnum].pclass == P_GLOBAL)
      {
         return (True);
      }
      
      parm_ptr = ((char *)pSERVICE(snum)) + PTR_DIFF(def_ptr, &sDefault);
   }

   if (snum >= 0)
   {
      if (!iSERVICE(snum).copymap) init_copymap(pSERVICE(snum));
      /* this handles the aliases - set the copymap for other entries with
         the same data pointer */
      for (i = 0; parm_table[i].label; i++)
         if (parm_table[i].ptr == parm_table[parmnum].ptr)
	    iSERVICE(snum).copymap[i] = False;
   }

   /* if it is a special case then go ahead */
   if (parm_table[parmnum].special)
   {
      parm_table[parmnum].special(pszParmValue, (char **)parm_ptr);
      return (True);
   }

   /* now switch on the type of variable it is */
   switch (parm_table[parmnum].type)
   {
      case P_BOOL:    set_boolean((BOOL *)parm_ptr, pszParmValue);
                      break;
      case P_INTEGER: *(int *)parm_ptr = atoi(pszParmValue);
                      break;
      case P_CHAR:    *(char *)parm_ptr = *pszParmValue;
                      break;
      case P_STRING:  string_set((char **)parm_ptr, pszParmValue);
                      break;
      case P_ENUM:    for (i = 0; parm_table[parmnum].enum_list[i].name; i++)
                      {
                         if (strequal(pszParmValue,
				      parm_table[parmnum].enum_list[i].name))
			 {
			    *(int *)parm_ptr =  parm_table[parmnum].
						enum_list[i].value;
			    break;
			 }
		      }
		      break;
      case P_SEP:     break;
   }

   return (True);
}


/**************************************************************************/
/* Process a parameter.							  */
/**************************************************************************/
BOOL do_parameter(char *pszParmName, char *pszParmValue)
{
   if (!bInGlobalSection && bGlobalOnly) return (True);
   DEBUGADD(4, ("doing parameter %s = %s\n", pszParmName, pszParmValue));

   return (lp_do_parameter(bInGlobalSection ? -2 : iServiceIndex,
           pszParmName, pszParmValue));
}


/**************************************************************************/
/* print a parameter of the specified type				  */
/**************************************************************************/
void print_parameter(struct parm_struct *p, void *ptr, FILE * f)
{
   int i;
   switch (p->type)
   {
      case P_ENUM:    for (i = 0; p->enum_list[i].name; i++)
                      {
                         if (*(int *)ptr == p->enum_list[i].value)
	                 {
	                    fprintf(f, "%s", p->enum_list[i].name); break;
	                 }
                      }

                      break;

      case P_BOOL:    fprintf(f, "%s", BOOLSTR(*(BOOL *)ptr));
		      break;

      case P_INTEGER: fprintf(f, "%d", *(int *)ptr);
		      break;

      case P_CHAR:    fprintf(f, "%c", *(char *)ptr);
		      break;

      case P_STRING:  if (*(char **)ptr) fprintf(f, "%s", *(char **)ptr);
		      break;

      case P_SEP:     break;
   }
}


/**************************************************************************/
/* check if two parameters are equal					  */
/**************************************************************************/
BOOL equal_parameter(parm_type type, void *ptr1, void *ptr2)
{
   switch (type)
   {
      case P_BOOL:    return (*((BOOL *)ptr1) == *((BOOL *)ptr2));
      case P_INTEGER:
      case P_ENUM:    return (*((int *)ptr1) == *((int *)ptr2));
      case P_CHAR:    return (*((char *)ptr1) == *((char *)ptr2));
      case P_STRING:
		      {
		 	 char *p1 = *(char **)ptr1, *p2 = *(char **)ptr2;
			 if (p1 && !*p1) p1 = NULL;
			 if (p2 && !*p2) p2 = NULL;

			 return (p1 == p2 || strequal(p1, p2));
		      }
      case P_SEP:    break;
   }
   return (False);
}


/*****************************************************************************/
/* Process a new section (service). At this stage all sections are services. */
/* Later we'll have special sections that permit server parameters to be set.*/
/* Returns True on success, False on failure.				     */
/*****************************************************************************/
BOOL do_section(char *pszSectionName)
{
   BOOL bRetval;
   BOOL isglobal = True;
   bRetval = False;

   /* if we've just struck a global section, note the fact. */
   bInGlobalSection = isglobal;

   /* check for multiple global sections */
   if (bInGlobalSection)
   {
      DEBUG(3, ("Processing section \"[%s]\"\n", pszSectionName));
      return (True);
   }

   if (!bInGlobalSection && bGlobalOnly) return (True);

   /* if we have a current service, tidy it up before moving on */
   bRetval = True;

   if (iServiceIndex >= 0) bRetval = service_ok(iServiceIndex);

   /* if all is still well, move to the next record in the services array */
   if (bRetval)
   {
      /* We put this here to avoid an odd message order if messages are */
      /* issued by the post-processing of a previous section. */

      if ((iServiceIndex = add_a_service(&sDefault, pszSectionName)) < 0)
      {
         return (False);
      }
   }
   return (bRetval);
}

/**************************************************************************/
/* return info about the next service  in a service. snum==-1 gives the   */
/* globals return NULL when out of parameters				  */
/**************************************************************************/
struct parm_struct *lp_next_parameter(int snum, int *i, int allparameters)
{
   if (snum == -1)
   {
      /* do the globals */
      for (; parm_table[*i].label; (*i)++)
      {
         if (parm_table[*i].pclass == P_SEPARATOR) return &parm_table[(*i)++];
         if (!parm_table[*i].ptr || (*parm_table[*i].label == '-')) continue;
	 if ((*i) > 0 && (parm_table[*i].ptr == parm_table[(*i) - 1].ptr))
            continue;

         return &parm_table[(*i)++];
      }
   }
   else
   {
      service *pService = pSERVICE(snum);

      for (; parm_table[*i].label; (*i)++)
      {
         if (parm_table[*i].pclass == P_SEPARATOR) return &parm_table[(*i)++];
         if (parm_table[*i].pclass == P_LOCAL &&
             parm_table[*i].ptr &&
            (*parm_table[*i].label != '-') &&
	    ((*i) == 0 ||
	     (parm_table[*i].ptr != parm_table[(*i) - 1].ptr)))
	 {
	    int pdiff = PTR_DIFF(parm_table[*i].ptr, &sDefault);
            if (allparameters || !equal_parameter(parm_table[*i].type,
	       ((char *)pService) + pdiff, ((char *)&sDefault) + pdiff))
            {
               return &parm_table[(*i)++];
            }
         }
      }
   }
   return NULL;
}


/**************************************************************************/
/* Return TRUE if the passed service number is within range.              */
/**************************************************************************/
BOOL lp_snum_ok(int iService)
{
   return (LP_SNUM_OK(iService));
}

/**************************************************************************/
/* have we loaded a config file yet?					  */
/**************************************************************************/
BOOL lp_loaded(void)
{
	return (bLoaded);
}

/**************************************************************************/
/* Load the config from config file. Return True on success, 		  */
/* False on failure.							  */
/**************************************************************************/
BOOL lp_load(char *pszFname, BOOL global_only, BOOL save_defaults,
	     BOOL add_ipc)
{
   pstring n2;
   BOOL bRetval;
   add_to_file_list(pszFname);
   bRetval = False;
   bInGlobalSection = True;
   bGlobalOnly = global_only;
   pstrcpy(n2, pszFname);
   standard_sub_basic(n2);
   
   iServiceIndex = -1;
   bRetval = pm_process(n2, do_section, do_parameter);
   DEBUG(4, ("pm_process() returned %s\n", BOOLSTR(bRetval)));
	
   config_loaded = time(NULL);
   if (bRetval) if (iServiceIndex >= 0) bRetval = service_ok(iServiceIndex);
   bLoaded = True;
   return (bRetval);
}


/**************************************************************************/
/* reset the max number of services					  */
/**************************************************************************/
void lp_resetnumservices(void)
{
   iNumServices = 0;
}


/**************************************************************************/
/* return the max number of services					  */
/**************************************************************************/
int lp_numservices(void)
{
   return (iNumServices);
}


/**************************************************************************/
/* remove a service							  */
/**************************************************************************/
void lp_remove_service(int snum)
{
   pSERVICE(snum)->valid = False;
}


/**************************************************************************/
/* Read post-reg file into special variable				  */
/**************************************************************************/
BOOL handle_v3_post_reg(char *pszParmValue, char **ptr)
{
   FILE *post_file;
   off_t fsize;
   pstring fname;
   pstrcpy(fname, pszParmValue);

   add_to_file_list(fname);
   string_set(ptr, fname);

   if (file_exist(fname, NULL)) 
   {
      fsize = get_file_size(fname);
      if (fsize > 0)
      {
         if (fsize > 512) fsize = 512;
	 post_file = fopen(fname, "r");
	       
	 string_free(&Globals.szv3_post_registration);
	 Globals.szv3_post_registration = (char*)malloc(fsize+1);
	 fsize = fread(Globals.szv3_post_registration, 1, fsize, post_file);
	 fclose(post_file);
	       
	 Globals.szv3_post_registration[fsize] = 0;
	       
	 ITrans.translateToClient(Globals.szv3_post_registration);
	       
	 DEBUGADD(2, ("Reading data from post-registration file: %s\n", fname));

         return(True);
      }    
   }

   LOG_SYS(0, ("WARNING: Can't find post-registration file %s\n", fname));

   return (False);
}


/**************************************************************************/
/* We should init translator after receiving table name 		  */
/**************************************************************************/
BOOL handle_translate(char *pszParmValue, char **ptr)
{
   char temp[128];
   
   snprintf(temp, 128, "%s/%s", Globals.sztranslate_path, pszParmValue);
   ITrans.setTranslationMap(temp);
   string_set(&Globals.sztranslate_tbl, pszParmValue);

   return(True);
}

/**************************************************************************/
/* Open and read into string v3 admin notes file 			  */
/**************************************************************************/
BOOL handle_v3_adm_notes(char *pszParmValue, char **ptr)
{
   FILE *adm_file;
   off_t fsize;
   pstring fname;
   pstrcpy(fname, pszParmValue);

   add_to_file_list(fname);
   string_set(ptr, fname);

   if (file_exist(fname, NULL)) 
   {
      fsize = get_file_size(fname);
      if (fsize > 0)
      {
         if (fsize > 512) fsize = 512;
	 adm_file = fopen(fname, "r");
	      
	 string_free(&Globals.szv3_admin_notes);
	 Globals.szv3_admin_notes = (char*)malloc(fsize+1);
	 fsize = fread(Globals.szv3_admin_notes, 1, fsize, adm_file);
	 fclose(adm_file);
	       
	 Globals.szv3_admin_notes[fsize] = 0;
	       
	 ITrans.translateToClient(Globals.szv3_admin_notes);
	       
	 DEBUGADD(2, ("Reading data from v3_admin notes: %s\n", fname));

         return(True);
      }    
   }

   LOG_SYS(0, ("WARNING: Can't find V3 admin notes file %s\n", fname));

   return (False);
}


/**************************************************************************/
/* handle the include operation						  */
/**************************************************************************/
BOOL handle_include(char *pszParmValue, char **ptr)
{
   pstring fname;
   pstrcpy(fname, pszParmValue);
   add_to_file_list(fname);
   standard_sub_basic(fname);
   string_set(ptr, fname);

   if (file_exist(fname, NULL)) 
   {
      DEBUGADD(2, ("Grab parameters from includes: %s\n", fname));
      return (pm_process(fname, do_section, do_parameter));
   }

   LOG_SYS(0, ("WARNING: Can't find include file %s\n", fname));
   return (False);
}


/**************************************************************************/
/* print usage on the program					   	  */
/**************************************************************************/
void usage()
{
   if (process_role < 100)
   {
      printf("Usage:  iserverd [-oh?V] [-d dbg name] [-p port] [-c config]\n");
      printf("\t-o                    Overwrite log file, don't append\n");
      printf("\t-?(h)                 Print usage\n");
      printf("\t-V                    Print version\n");
      printf("\t-d <debug logname>    Set the debug filename\n");
      printf("\t-c <config name>      Set the config filename\n");
      printf("\t-p <port>             Listen on the specified port\n");
      printf("\n");
   }
   
   if (process_role == ROLE_DUSER)
   {   
      printf("Usage: disconnect -r uin [-c config] [-h?V]\n");
      printf("\t-r <uin>              User to disconnect from server\n");
      printf("\t-?(h)                 Print usage\n");
      printf("\t-V                    Print version\n");
      printf("\t-c <config name>      Set the config filename\n");
      printf("\n");
   }
   
   if (process_role == ROLE_STATUS)
   {   
      printf("Usage: server_status [-c config] [-lh?V]\n");
      printf("\t-?(h)                 Print usage\n");
      printf("\t-V                    Print version\n");
      printf("\t-c <config name>      Set the config filename\n");
      printf("\t-l                    Print process list\n");
      printf("\n");
   }
   
   exit(EXIT_NORMAL);
}


/**************************************************************************/
/* Process command line options.                             	  	  */
/**************************************************************************/
void process_command_line_opt(int argc, char **argv)
{
   char         flag;          
   extern char  *optarg;       

    /* OPTSTR  "loVd:c:p:" */
    while ((flag = getopt(argc, argv, OPTSTR)) != (int)EOF)
    {
        switch (flag)
        {
	    /* port number */
            case 'p':
		Globals.udp_port = atoi(optarg);
                break;
	
	    case 'r':
	        if (process_role == ROLE_DUSER)
		{
		   deluser = atoul(optarg);
		   break;
		}
	    
	    /* debug log filename */
	    case 'd':
		string_set(&Globals.szdbglog_path, optarg);
		break;

	    /* config log filename */
	    case 'c':
		string_set(&Globals.szconfig_path, optarg);
		break;

	    /* if we should overwrite logfiles */	
            case 'o':
		Globals.append_log = False;
                break;
	    
	    /* version request */
            case 'V':
		printf("Iserverd version: %s\n\n", Iversion);
		exit(EXIT_NORMAL);
                break;
	    
	    case 'l':
	        Globals.proclist = True;
		break;

	    /* default action */
            case '?':
	    case 'h':
            default:  usage();
        }
    }   
    
    if ((process_role == ROLE_DUSER) && (deluser == 0)) usage();
}

