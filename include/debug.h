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
/**************************************************************************/

#ifndef DBG_H
#define DBG_H

extern int DBG_LEVEL;

int  Debug1( char *, ... )
     __attribute__ ((format (printf, 1, 2)))
;
BOOL dbgtext( char *, ... )
     __attribute__ ((format (printf, 1, 2)))
;


#ifdef HAVE_FILE_MACRO
#define FILE_MACRO (__FILE__)
#else
#define FILE_MACRO ("")
#endif

#ifdef HAVE_FUNCTION_MACRO
#define FUNCTION_MACRO  (__FUNCTION__)
#else
#define FUNCTION_MACRO  ("")
#endif

#ifdef NO_DEBUG_REPORTING
#define DEBUGLVL( level ) (True)
#define DEBUG( level , body ) (void)(False)
#define DEBUGADD( level , body ) (void)(False)
#else

#define DEBUGLVL( level ) \
  ( (DEBUGLEVEL >= (level)) \
   && dbghdr( level, FILE_MACRO, FUNCTION_MACRO, (__LINE__) ) )

#define DEBUG( level, body ) \
  (void)( (DEBUGLEVEL >= (level)) \
       && (dbghdr( level, FILE_MACRO, FUNCTION_MACRO, (__LINE__) )) \
       && (dbgtext body) )

#define DEBUGADD( level, body ) \
  (void)( (DEBUGLEVEL >= (level)) && (dbgtext body) )

#endif

typedef enum
{

   dbg_null = 0,
   dbg_ignore,
   dbg_header,
   dbg_timestamp,
   dbg_level,
   dbg_sourcefile,
   dbg_function,
   dbg_lineno,
   dbg_message,
   dbg_eof

}  dbg_Token;

#endif
