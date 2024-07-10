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
#define SYSFORMAT_BUFR_MAX ( sizeof( sysformat_bufr ) - 1 )

FILE   *sysf        		  = NULL;
static pstring  sysformat_bufr    = { '\0' };
static size_t   sysformat_pos     = 0;
extern BOOL    stdout_logging;


/**************************************************************************/
/* tells us if interactive logging was requested			  */
/**************************************************************************/
BOOL sys_interactive(void)
{
   return stdout_logging;
}

/**************************************************************************/
/* get ready for syslog stuff						  */
/**************************************************************************/
void setup_syslogging( char *pname, BOOL interactive )
{
   if( interactive )
   {
      stdout_logging = True;
      sysf = stdout;
   }
}


/**************************************************************************/
/* Write an system message on the system log file.			  */
/**************************************************************************/
int sysDebug1( char *format_str, ... )
{
   va_list ap;
   int old_errno = errno;

   if( stdout_logging )
   {
      va_start( ap, format_str );
      if(sysf) (void)vfprintf( sysf, format_str, ap );
      va_end( ap );
      errno = old_errno;
      return( 0 );
   }

   va_start( ap, format_str );
   vsyslog(LOG_NOTICE, format_str, ap );
   va_end( ap );

   return(0);

}

/**************************************************************************/
/* Print the buffer content via Debug1(), then reset the buffer.	  */
/**************************************************************************/
void sysbufr_print( void )
{
   sysformat_bufr[sysformat_pos] = '\0';
   (void)sysDebug1( "%s", sysformat_bufr );
   sysformat_pos = 0;

}

/*************************************************************************
Format the system message text.
**************************************************************************/
void format_system_text( char *msg )
{
   size_t i;

   for( i = 0; msg[i]; i++ )
   {
      if( sysformat_pos < SYSFORMAT_BUFR_MAX )
          sysformat_bufr[sysformat_pos++] = msg[i];

      if( '\n' == msg[i] ) sysbufr_print();

      if( sysformat_pos >= SYSFORMAT_BUFR_MAX )
      {
         sysbufr_print();
         (void)sysDebug1( " +>\n" );
      }
   }

  sysformat_bufr[sysformat_pos] = '\0';
}


/**************************************************************************/
/* Add text to the body of the "current" system message via 		  */
/* the format buffer.							  */
/**************************************************************************/
BOOL systext( char *format_str, ... )
{
   va_list ap;
   pstring msgbuf;

   va_start( ap, format_str );
   vslprintf( msgbuf, sizeof(msgbuf)-1, format_str, ap );
   va_end( ap );

   format_system_text( msgbuf );

   return( True );

}

