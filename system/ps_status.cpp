/*------------------------------------------------------------------------*/
/* ps_status.c (I took this from PostgreSQL 7.1 :)			  */
/*									  */
/* Routines to support changing the ps display of IServerd processes	  */
/* to contain some useful information. Differs wildly across		  */
/* platforms.								  */
/*									  */
/* Copyright 2000 by PostgreSQL Global Development Group		  */
/* various details abducted from various places				  */
/*									  */
/*------------------------------------------------------------------------*/

#include "includes.h"

#ifdef HAVE_SYS_PSTAT_H
#include <sys/pstat.h>		/* for HP-UX */
#endif

#ifdef HAVE_PS_STRINGS
#include <machine/vmparam.h>	/* for old BSD */
#include <sys/exec.h>
#endif

extern char **environ;

/*
 * Alternative ways of updating ps display:
 *
 * PS_USE_SETPROCTITLE
 *	   use the function setproctitle(const char *, ...)
 *	   (newer BSD systems)
 * PS_USE_PSTAT
 *	   use the pstat(PSTAT_SETCMD, )
 *	   (HPUX)
 * PS_USE_PS_STRINGS
 *	   assign PS_STRINGS->ps_argvstr = "string"
 *	   (some BSD systems)
 * PS_USE_CHANGE_ARGV
 *	   assign argv[0] = "string"
 *	   (some other BSD systems)
 * PS_USE_CLOBBER_ARGV
 *	   write over the argv and environment area
 *	   (most SysV-like systems)
 * PS_USE_NONE
 *	   don't update ps display
 *	   (This is the default, as it is safest.)
 */

#if defined(HAVE_SETPROCTITLE)
#define PS_USE_SETPROCTITLE
#elif defined(HAVE_PSTAT) && defined(PSTAT_SETCMD)
#define PS_USE_PSTAT
#elif defined(HAVE_PS_STRINGS)
#define PS_USE_PS_STRINGS
#elif defined(BSD) || defined(__bsdi__) || defined(__hurd__)
#define PS_USE_CHANGE_ARGV
#elif defined(__linux__) || defined(_AIX4) || defined(_AIX3) || defined(__sgi) || (defined(sun) && !defined(BSD)) || defined(ultrix) || defined(__ksr__) || defined(__osf__) || defined(__QNX__) || defined(__svr4__) || defined(__svr5__)
#define PS_USE_CLOBBER_ARGV
#else
#define PS_USE_NONE
#endif

/* Different systems want the buffer padded differently */
#if defined(_AIX3) || defined(__linux__) || defined(__QNX__) || defined(__svr4__)
#define PS_PADDING '\0'
#else
#define PS_PADDING ' '
#endif

#ifndef PS_USE_CLOBBER_ARGV
/* all but one options need a buffer to write their ps line in */
#define PS_BUFFER_SIZE 256
static char ps_buffer[PS_BUFFER_SIZE];
static const size_t ps_buffer_size = PS_BUFFER_SIZE;

#else					/* PS_USE_CLOBBER_ARGV 		*/
static char *ps_buffer;			/* will point to argv area 	*/
static size_t ps_buffer_size;		/* space determined at run time */

#endif	 /* PS_USE_CLOBBER_ARGV */

static size_t ps_buffer_fixed_size;	/* size of the constant prefix  */

/**************************************************************************/
/* Call this once at server process start. 				  */
/**************************************************************************/
void init_ps_display(int argc, char *argv[])
{
   char prole[32];

   strncpy(prole, "", 31);

#ifndef PS_USE_NONE

#ifdef PS_USE_CHANGE_ARGV
   argv[0] = ps_buffer;
   argv[1] = NULL;
#endif	 /* PS_USE_CHANGE_ARGV */

#ifdef PS_USE_CLOBBER_ARGV
   /* If we're going to overwrite the argv area, count the space. */
   {
      char  *end_of_area = NULL;
      char  **new_environ;
      int   i;

      /* check for contiguous argv strings */
      for (i = 0; i < argc; i++)
      	 if (i == 0 || end_of_area + 1 == argv[i])
            end_of_area = argv[i] + strlen(argv[i]);

      /* check for contiguous environ strings following argv */
      for (i = 0; end_of_area != NULL && environ[i] != NULL; i++)
         if (end_of_area + 1 == environ[i])
            end_of_area = environ[i] + strlen(environ[i]);

      if (end_of_area == NULL)
      {
         ps_buffer = NULL;
         ps_buffer_size = 0;
      }
      else
      {
         ps_buffer = argv[0];
         ps_buffer_size = end_of_area - argv[0] - 1;
      }
      argv[1] = NULL;

      /* move the environment out of the way */
      for (i = 0; environ[i] != NULL; i++);

      new_environ = (char **)malloc(sizeof(char *) * (i + 1));

      for (i = 0; environ[i] != NULL; i++)
         new_environ[i] = strdup(environ[i]);

      new_environ[i] = NULL;
      environ = new_environ;
   }

#endif	 /* PS_USE_CLOBBER_ARGV */
   /* Make fixed prefix */
#ifdef PS_USE_SETPROCTITLE
   /* setproctitle() already adds a `progname:' prefix to the ps line */
   snprintf(ps_buffer, ps_buffer_size, "%s", prole);
#else
   snprintf(ps_buffer, ps_buffer_size, "IServerd: %s", prole);
#endif
   ps_buffer_fixed_size = strlen(ps_buffer);
#endif	 /* not PS_USE_NONE */

}


/**************************************************************************/
/* Call this to update the ps status display to a fixed prefix plus an	  */
/* indication of what you're currently doing passed in the argument.	  */
/**************************************************************************/
void set_ps_display(int process_role, const char *value)
{
   char prole[32];
   char rvalue[64];

   switch (process_role)
   {
      case ROLE_PACKET : strncpy(prole, "[packet  processor] ", 31); break;
      case ROLE_SOCKET : strncpy(prole, "[socket  processor] ", 31); break;
      case ROLE_EPACKET: strncpy(prole, "[epacket processor] ", 31); break;
      case ROLE_ETIMER : strncpy(prole, "[etimer  processor] ", 31); break;
      case ROLE_EUSER  : strncpy(prole, "[euser   processor] ", 31); break;
      case ROLE_DEFRAG : strncpy(prole, "[defrag  processor] ", 31); break;
      case ROLE_ACTIONS: strncpy(prole, "[actions processor] ", 31); break;
      case ROLE_BUSY   : strncpy(prole, "[busy    processor] ", 31); break;
      case ROLE_PAUSED : strncpy(prole, "paused "             , 31); break;
      case ROLE_EMPTY  : strncpy(prole, ""                    , 31); break;
      default:	         strncpy(prole, "[unknown role] "     , 31); break;
   }

   snprintf(rvalue, 63, "%s%s", prole, value);

#ifndef PS_USE_NONE
#ifdef PS_USE_CLOBBER_ARGV
   /* If ps_buffer is a pointer, it might still be null */
   if (!ps_buffer) return;
#endif

   /* Update ps_buffer to contain both fixed part and rvalue */
   strncpy(ps_buffer + ps_buffer_fixed_size, rvalue,
           ps_buffer_size - ps_buffer_fixed_size);

   /* Transmit new setting to kernel, if necessary */

#ifdef PS_USE_SETPROCTITLE
   setproctitle("%s", ps_buffer);
#endif

#ifdef PS_USE_PSTAT
   {
      union pstun pst;
      pst.pst_command = ps_buffer;
      pstat(PSTAT_SETCMD, pst, strlen(ps_buffer), 0, 0);
   }
#endif	 /* PS_USE_PSTAT */

#ifdef PS_USE_PS_STRINGS
   PS_STRINGS->ps_nargvstr = 1;
   PS_STRINGS->ps_argvstr = ps_buffer;
#endif	 /* PS_USE_PS_STRINGS */

#ifdef PS_USE_CLOBBER_ARGV
   {
      char   *cp;

      /* pad unused memory */
      for (cp = ps_buffer + strlen(ps_buffer);
           cp < ps_buffer + ps_buffer_size; cp++)

          *cp = PS_PADDING;
   }
#endif	 /* PS_USE_CLOBBER_ARGV */

#endif	 /* not PS_USE_NONE */

}

