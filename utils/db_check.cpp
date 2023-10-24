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
/* This utility used to check database integrity and fix if it contain 	  */
/* structure errors. Also it can create missing tables			  */
/*									  */
/**************************************************************************/

#include "includes.h"

extern pstring debugf; 
extern pstring systemf; 
extern pstring alarmf;
extern pstring usersf;
extern BOOL append_log;

/**************************************************************************/
/* UTILITY: Check database integrity					  */
/**************************************************************************/
int main(int argc, char **argv)
{

  /* Process command line options */
  init_globals();
  process_command_line_opt(argc, argv);
  pstring configf;
  slprintf(configf, sizeof(configf)-1, lp_config_file());

  /* Global parameters initialization */
  setup_usrlogging( "", True );
  slprintf(debugf,  sizeof(debugf), "/dev/null");
  setup_alrlogging( "", True );
  setup_syslogging( "", True );
  
  /* Loading configuration file */
  if (!lp_load(configf,False,False,True))
  {
    printf("FATAL ERROR: Can't find config file: \"%s\"\n", configf);
    exit(EXIT_ERROR_CONFIG_OPEN);
  }
  
  check_and_fix_database(lp_db_users(), lp_db_user(), lp_db_pass(), 
                         lp_db_addr(), lp_db_port());
  
}

