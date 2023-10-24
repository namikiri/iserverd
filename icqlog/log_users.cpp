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
#define USRFORMAT_BUFR_MAX ( sizeof( usrformat_bufr ) - 1 )

FILE   *usrf        		  = NULL;
static pstring  usrformat_bufr    = { '\0' };
static size_t   usrformat_pos     = 0;
extern BOOL    stdout_logging;


/**************************************************************************/
/* tells us if interactive logging was requested			  */
/**************************************************************************/
BOOL usr_interactive(void)
{
   return stdout_logging;
}

/**************************************************************************/
/* get ready for usrlog stuff						  */
/**************************************************************************/
void setup_usrlogging( char *pname, BOOL interactive )
{
   if( interactive )
   {
      stdout_logging = True;
      usrf = stdout;
   }
}

/**************************************************************************/
/* reopen the log files							  */
/**************************************************************************/
void init_syslog_logs( void )
{
   int params = 0;
   closelog();
  
   /* if user want to log process pid */
   if (lp_pid_in_logs()) params |= LOG_PID;
  
   /* now time to open syslog facility... :) */
   openlog(SYSLOG_SIDENT, params, LOG_LOCAL0); 
}

/**************************************************************************/
/* Write an users message on the users log file.			  */
/**************************************************************************/
int usrDebug1( char *format_str, ... )
{
   va_list ap;  
   int old_errno = errno;

   if( stdout_logging )
   {
      va_start( ap, format_str );
      if(usrf)
      (void)vfprintf( usrf, format_str, ap );
      va_end( ap );
      errno = old_errno;
      return( 0 );
   }
   else
   {  
      va_start( ap, format_str );
      vsyslog( LOG_INFO , format_str, ap );
      va_end( ap );
   }
  
   return(0);

}


/**************************************************************************/
/* Print the buffer content via Debug1(), then reset the buffer.	  */
/**************************************************************************/
void usrbufr_print( void )
{
   usrformat_bufr[usrformat_pos] = '\0';
   (void)usrDebug1( "%s", usrformat_bufr );
   usrformat_pos = 0;
}

/*************************************************************************
Format the users message text.
**************************************************************************/
void format_users_text( char *msg )
{
   size_t i;

   for( i = 0; msg[i]; i++ )
   {
      if( usrformat_pos < USRFORMAT_BUFR_MAX )
          usrformat_bufr[usrformat_pos++] = msg[i];

      if( '\n' == msg[i] ) usrbufr_print();

      if( usrformat_pos >= USRFORMAT_BUFR_MAX )
      {
         usrbufr_print();
         (void)usrDebug1( " +>\n" );
      }
   }
   usrformat_bufr[usrformat_pos] = '\0';
}

/**************************************************************************/
/* Add text to the body of the "current" users message via 		  */
/* the format buffer.							  */
/**************************************************************************/
BOOL usrtext( char *format_str, ... )
{
   va_list ap;
   pstring msgbuf;

   va_start( ap, format_str ); 
   vslprintf( msgbuf, sizeof(msgbuf)-1, format_str, ap );
   va_end( ap );

   format_users_text( msgbuf );

   return( True );

} 

