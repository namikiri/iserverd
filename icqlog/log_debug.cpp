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

#define FORMAT_DBUFR_MAX ( sizeof( format_bufr ) - 1 )

FILE   *dbgf        	= NULL;
pstring debugf     	= "";

BOOL    append_log 	= False;
BOOL    timestamp_log 	= True;
BOOL    stdout_logging  = False;

static pstring  format_bufr    = { '\0' };
static size_t   format_pos     = 0;
static int      debug_count    = 0;

/**************************************************************************/
/* Dump content of packet to log in hex and asc format.			  */
/**************************************************************************/
void log_alarm_packet(int level, Packet &pack)
{
   (void)dbghdr (level, "", "", (__LINE__) );(void)dbgtext("\n");

   DEBUGADD(level, ("[Packet size: %d bytes] [%s:%d]\n", pack.sizeVal,
                     inet_ntoa(pack.from_ip), pack.from_port));

   dbg_dump_data(level, pack.buff, pack.sizeVal);
   DEBUGADD(level, ("\n"));
}


/**************************************************************************/
/* tells us if interactive logging was requested			  */
/**************************************************************************/
BOOL dbg_interactive(void)
{
   return stdout_logging;
}

/**************************************************************************/
/* get ready for syslog stuff						  */
/**************************************************************************/
void setup_logging( char *pname, BOOL interactive )
{
   if( interactive )
   {
      stdout_logging = True;
      dbgf = stdout;
   }
}

/**************************************************************************/
/* reopen the log files							  */
/**************************************************************************/
void reopen_logs( void )
{
   pstring fname;

   (void)umask( lp_umask() );

   if( DEBUGLEVEL > 0 )
   {
      pstrcpy( fname, debugf );
      if( lp_loaded() && (*lp_dbglog_path()) )
      pstrcpy( fname, lp_dbglog_path() );

      if( !strcsequal( fname, debugf ) || !dbgf || !file_exist( debugf, NULL ) )
      {
         pstrcpy( debugf, fname );
         if( dbgf ) (void)fclose( dbgf );
         if( append_log ) dbgf = fopen( debugf, "a" );
         else dbgf = fopen( debugf, "w" );

         if (dbgf == NULL)
         {
	    LOG_SYS(0, ("FATAL ERROR: Can't open (create) debug logfile: \"%s\"\n", debugf));
	    exit(EXIT_ERROR_LOG_CREATE);
         }

         fchmod(fileno(dbgf), lp_lperms());
         force_check_log_size();
         if( dbgf ) setbuf( dbgf, NULL );
         (void)umask( lp_umask() );

      }
   }
   else
   {
      if( dbgf )
      {
         (void)fclose( dbgf );
         dbgf = NULL;
      }
   }
}

/**************************************************************************/
/* Force a check of the log size.					  */
/**************************************************************************/
void force_check_log_size( void )
{
   debug_count = 100;
}

/**************************************************************************/
/* Check to see if there is any need to check if the logfile 		  */
/* has grown too big.							  */
/**************************************************************************/
BOOL need_to_check_log_size( void )
{
   int maxlog;

   if( debug_count++ < 100 )  return( False );

   maxlog = lp_log_size() * 1024;
   if( !dbgf || maxlog <= 0 )
   {
      debug_count = 0;
      return(False);
   }

   return( True );
}

/**************************************************************************/
/* Check to see if the log has grown to be too big.			  */
/**************************************************************************/
void check_log_size( void )
{
   int    maxlog;
   struct stat st;

   if( geteuid() != 0 ) return;

   if( !need_to_check_log_size() ) return;

   maxlog = lp_log_size() * 1024;

   if( fstat( fileno( dbgf ), &st ) == 0 && st.st_size > maxlog )
   {
      (void)fclose( dbgf );
      dbgf = NULL;
      reopen_logs();
      if( dbgf && get_file_size( debugf ) > maxlog )
      {
         pstring name;

         (void)fclose( dbgf );
         dbgf = NULL;
         slprintf( name, sizeof(name)-1, "%s.old", debugf );
         (void)rename( debugf, name );
         reopen_logs();
      }
   }
  debug_count = 0;
}

/**************************************************************************/
/* Write an debug message on the debugfile.				  */
/**************************************************************************/
int Debug1( char *format_str, ... )
{
   va_list ap;
   int old_errno = errno;

   if( stdout_logging )
   {
      va_start( ap, format_str );
      if(dbgf)
      (void)vfprintf( dbgf, format_str, ap );
      va_end( ap );
      errno = old_errno;
      return( 0 );
   }

   {
      if( !dbgf )
      {
        (void)umask( lp_umask() );
        if( append_log )
           dbgf = fopen( debugf, "a" );
        else
           dbgf = fopen( debugf, "w" );

        (void)umask( lp_umask() );
        if( dbgf )
        {
           setbuf( dbgf, NULL );
        }
        else
        {
           errno = old_errno;
           return(0);
        }
     }
  }

  {
    va_start( ap, format_str );
    if(dbgf) (void)vfprintf( dbgf, format_str, ap );
    va_end( ap );
    if(dbgf) (void)fflush( dbgf );
  }

  check_log_size();
  errno = old_errno;

  return(0);

}

/**************************************************************************/
/* Print the buffer content via Debug1(), then reset the buffer.	  */
/**************************************************************************/
void bufr_print( void )
{
   format_bufr[format_pos] = '\0';
   (void)Debug1( "%s", format_bufr );
   format_pos = 0;
}

/**************************************************************************/
/* Format the debug message text.					  */
/**************************************************************************/
void format_debug_text( char *msg )
{
  size_t i;
  BOOL timestamp = (timestamp_log && !stdout_logging && (lp_timestamp_logs() || !(lp_loaded())));

  for( i = 0; msg[i]; i++ )
  {
     if(timestamp && 0 == format_pos)
     {
       format_bufr[0] = format_bufr[1] = ' ';
       format_pos = 2;
     }

     if( format_pos < FORMAT_DBUFR_MAX )
         format_bufr[format_pos++] = msg[i];

     if( '\n' == msg[i] && i>0 ) bufr_print();

     if( format_pos >= FORMAT_DBUFR_MAX )
     {
        bufr_print();
        (void)Debug1( " +>\n" );
     }
  }

  format_bufr[format_pos] = '\0';

}

/**************************************************************************/
/* Flush debug output, including the format buffer content.		  */
/**************************************************************************/
void dbgflush( void )
{
   bufr_print();
   if(dbgf) (void)fflush(dbgf);
}

/**************************************************************************/
/* Print a Debug Header.						  */
/**************************************************************************/
BOOL dbghdr( int level, char *file, char *func, int line )
{
   int old_errno = errno;
   if( stdout_logging )  return( True );

   if( timestamp_log && (lp_timestamp_logs() || !(lp_loaded()) ))
   {
      char header_str[200];
      header_str[0] = '\0';
      if (level <= DEBUGLEVEL)
       (void)Debug1( "[%s, %d%s] %s:%s(%d)\n", timestring(False),
		     level, header_str, file, func, line );
   }

   errno = old_errno;
   return( True );
}

/**************************************************************************/
/* Add text to the body of the "current" debug message via 		  */
/* the format buffer.							  */
/**************************************************************************/
BOOL dbgtext( char *format_str, ... )
{
   va_list ap;
   pstring msgbuf;

   va_start( ap, format_str );
   vslprintf( msgbuf, sizeof(msgbuf)-1, format_str, ap );
   va_end( ap );

   format_debug_text( msgbuf );

   return( True );

}


/**************************************************************************/
/* Dump content of packet to log in hex and asc format.			  */
/**************************************************************************/
void log_debug_packet(int level, Packet &pack)
{
   if (DEBUGLEVEL < level) return;

   (void)dbghdr (level, "", "", (__LINE__) );(void)dbgtext("\n");

   DEBUGADD(level, ("Received %d bytes from [%s:%d]\n", pack.sizeVal,
	                   inet_ntoa(pack.from_ip), pack.from_port));

   dbg_dump_data(level, pack.buff, pack.sizeVal);
   DEBUGADD(level, ("\n"));

}

/**************************************************************************/
/* Print hex & asc dump of mem buf1 with specified length.		  */
/**************************************************************************/
void dbg_dump_data(int level, const char *buf1, int len)
{
   uchar const *buf = (uchar const *)buf1;
   int i = 0;

   if (DEBUGLEVEL < level) return;
   if (buf == NULL)
   {
      DEBUGADD(level, ("log_dump: NULL, len=%d\n", len));
      return;
   }
   if (len < 0)	return;
   if (len == 0)
   {
      DEBUGADD(level, ("\n"));
      return;
   }

   DEBUGADD(level, ("[%03X] ", i));
   for (i = 0; i < len;)
   {
      DEBUGADD(level, ("%02X ", (int)buf[i]));
      i++;
      if (i % 8 == 0)
      DEBUGADD(level, (" "));
      if (i % 16 == 0)
      {
 	 dbg_print_asc(level, &buf[i - 16], 8);
	 DEBUGADD(level, (" "));
	 dbg_print_asc(level, &buf[i - 8], 8);
	 DEBUGADD(level, ("\n"));
	 if (i < len)  DEBUGADD(level, ("[%03X] ", i));
      }
   }

   if (i % 16 != 0)	/* finish off a non-16-char-length row */
   {
      int n;

      n = 16 - (i % 16);
      DEBUGADD(level, (" "));
      if (n > 8) DEBUGADD(level, (" "));

      while (n--) DEBUGADD(level, ("   "));

      n = MIN(8, i % 16);
      dbg_print_asc(level, &buf[i - (i % 16)], n);
      DEBUGADD(level, (" "));
      n = (i % 16) - n;
      if (n > 0)
      dbg_print_asc(level, &buf[i - n], n);
      DEBUGADD(level, ("\n"));
   }
}

/**************************************************************************/
/* Print asc char (if it can't be printed print ".").			  */
/**************************************************************************/
void dbg_print_asc(int level, uchar const *buf, int len)
{
   int i;
   for (i = 0; i < len; i++)
   {
      DEBUGADD(level, ("%c", isprint(buf[i]) ? buf[i] : '.'));
   }
}

