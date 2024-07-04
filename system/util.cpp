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
/* Misc utility functions						  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

int case_default = CASE_LOWER;

char *daynames[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
char *daynames_short[] = { "M", "Tu", "W", "Th", "F", "Sa", "Su" };

/* union to get access to word bytes					  */
typedef union bo_test
{
  unsigned long complex;
  char simplex[4];
} bo_test;


/**************************************************************************/
/* direct popen function (don't use shell) 				  */
/**************************************************************************/
FILE * dpopen(char *program, char *type)
{
   char *cp;
   FILE *iop = NULL;
   pid_t pid;
   int argc, pdes[2];
   char *argv[MAX_ARGS + 1];

   if ((*type != 'r') && (*type != 'w')) return(NULL);
   if (pipe(pdes) < 0) return(NULL);

   /* break up string into pieces */
   for (argc = 0, cp = program; argc < MAX_ARGS; cp = NULL)
   {
      if (!(argv[argc++] = strtok(cp, " \t\n"))) break;
   }

   argv[MAX_ARGS] = NULL;

   switch(pid = vfork())
   {
      case -1:			/* error */
          close(pdes[0]);
	  close(pdes[1]);
	  goto pfree;

      case 0:			/* child */

#ifdef HAVE_SETSID
         /* setsid() is the preferred way to disassociate from the */
         /* controlling terminal                                   */
         setsid();
#else
         int ttyfd;
         /* Open /dev/tty to access our controlling tty (if any) */
         if( (ttyfd = open("/dev/tty",O_RDWR)) != -1)
         {
            if(ioctl(ttyfd,TIOCNOTTY,NULL) == -1)
            {
               LOG_SYS(0, ("Can't detach from controling terminal (ioctl error).\n"));
               exit(EXIT_CONFIG);
            }

            close(ttyfd);
         }
#endif /* HAVE_SETSID */

	 if (*type == 'r')
	 {
	     /* Do not share our parent's stdin */
	     close(fileno(stdin));
	     if (pdes[1] != 1)
	     {
	        dup2(pdes[1], 1);
		dup2(pdes[1], 2);	/* stderr, too! */
		close(pdes[1]);
	     }

	     close(pdes[0]);
         }
	 else
	 {
	    if (pdes[0] != 0)
	    {
	       dup2(pdes[0], 0);
	       close(pdes[0]);
	    }

	    close(fileno(stdout));
	    close(fileno(stderr));
	    close(pdes[1]);
	 }

	 execvp(argv[0], argv);
        _exit(1);
      }

      /* parent; assume fdopen can't fail...  */
      if (*type == 'r')
      {
         iop = fdopen(pdes[0], type);
	 close(pdes[1]);
      }
      else
      {
         iop = fdopen(pdes[1], type);
	 close(pdes[0]);
      }

pfree:
      return(iop);
}


/**************************************************************************/
/* Enable non-blocking mode on given fd					  */
/**************************************************************************/
BOOL setNonBlocking(int fd)
{
   int flags;
   int blank = 0;

   if ((flags = fcntl(fd, F_GETFL, blank)) < 0)
   {
      DEBUG(10, ("Error: fd=%d (fcntl F_GETFL err=%s)\n", fd, strerror(errno)));
      return(False);
   }

   if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
   {
      DEBUG(10, ("Error: fd=%d (fcntl F_SETFL err=%s)\n", fd, strerror(errno)));
      return(False);
   }

   return(True);
}


/**************************************************************************/
/* test byteorder 							  */
/**************************************************************************/
int setup_byteorder()
{
   union bo_test bt_order;

   bt_order.complex = 0x11223344;

   if ((bt_order.simplex[0] == 0x44) &&
       (bt_order.simplex[1] == 0x33))
   {
      LOG_SYS(0, ("Init: checking byteorder... LITTLE_ENDIAN\n"));
      return(0);
   }


   if ((bt_order.simplex[0] == 0x11) &&
       (bt_order.simplex[1] == 0x22))
   {
      LOG_SYS(0, ("Init: checking byteorder... BIG_ENDIAN\n"));
      return(1);
   }

   LOG_SYS(0, ("Init: checking byteorder... PDP_ENDIAN\n"));
   LOG_SYS(0, ("Init: sorry, I can't work with such byteorder\n"));
   exit(EXIT_ERROR_BYTEORDER);
}


/***************************************************************************/
/* check if a process exists. Does this work on all unixes?                */
/***************************************************************************/
BOOL process_exists(pid_t pid)
{
	return(kill(pid,0) == 0 || errno != ESRCH);
}


/**************************************************************************/
/* This used because of flow control (they call it rate limit) in 99a,99b */
/**************************************************************************/
void results_delay(int mseconds)
{
   msleep(mseconds);
}


/**************************************************************************/
/* This function fill buffer with random values  			  */
/**************************************************************************/
void fill_buff_random(unsigned short uin, char *buff, unsigned short length)
{

}


/**************************************************************************/
/* This is wrapper for poll			 			  */
/**************************************************************************/
int sys_select(int maxfd, fd_set *fds,struct timeval *tval)
{
  struct pollfd pfd[256];
  int i;
  int maxpoll;
  int timeout;
  int pollrtn;

  maxpoll = 0;
  for( i = 0; i < maxfd; i++)
  {
    if(FD_ISSET(i,fds))
    {
      struct pollfd *pfdp = &pfd[maxpoll++];
      pfdp->fd = i;
      pfdp->events = POLLIN;
      pfdp->revents = 0;
    }
  }

  timeout = (tval != NULL) ? (tval->tv_sec * 1000) + (tval->tv_usec/1000) :
                -1;
  errno = 0;
  do
  {
     pollrtn = poll( &pfd[0], maxpoll, timeout);
  }
  while (pollrtn<0 && errno == EINTR);

  FD_ZERO(fds);

  for( i = 0; i < maxpoll; i++)
    if( pfd[i].revents & POLLIN )
      FD_SET(pfd[i].fd,fds);

  return pollrtn;
}


#ifndef HAVE_VSYSLOG
/**************************************************************************/
/* Some systems (i.e cygwin) do not have vsyslog function                 */
/**************************************************************************/
void vsyslog(int priority, const char *message, va_list args)
{
   char sl_message[1024];

   vsnprintf(sl_message, 1023, message, args);

   /* BUGBUG: I think I should strip % from this string before    */
   /* passing it to syslog - this can cause format bufer overflow */
   /* I'll add this later                                         */
   syslog(priority, sl_message);
}
#endif

/**************************************************************************/
/* This is random generator initializer					  */
/**************************************************************************/
void init_random()
{
#ifdef HAVE_SRANDOMDEV
   srandomdev();
#else
   srandom((unsigned long)(time(NULL)+sys_getpid()));
#endif
}


/**************************************************************************/
/* This is unsigned-signed converter					  */
/**************************************************************************/
unsigned long arb(long param)
{
   if (param < 0) return( 0 );
   return( param );
}


/**************************************************************************/
/* This is generate first login password for new users 			  */
/**************************************************************************/
void generate_passwd(char *pass, int chars)
{
   snprintf(pass, chars+1, "%X", (unsigned int)random());

}


/**************************************************************************/
/* This is generate random number for magic ipc variables		  */
/**************************************************************************/
unsigned short random_num()
{
   return ((unsigned short)random());
}


unsigned long lrandom_num()
{
   return ((unsigned long)random());
}


/**************************************************************************/
/* find a suitable temporary directory. The result should be copied 	  */
/* immediately as it may be overwritten by a subsequent call		  */
/**************************************************************************/
char *tmpdir(void)
{
   char *p;
   if ((p = getenv("TMPDIR")))
   {
      return p;
   }
   return "/tmp";
}


BOOL in_group(gid_t group, gid_t current_gid, int ngroups, gid_t * groups)
{
   int i;

   if (group == current_gid)  return (True);

   for (i = 0; i < ngroups; i++)
   if (group == groups[i])  return (True);

   return (False);
}


static pid_t mypid = (pid_t)-1;


pid_t sys_getpid(void)
{
   if (mypid == (pid_t)-1)  mypid = getpid();
   return mypid;
}


/**************************************************************************/
/*  check if a file exists						  */
/**************************************************************************/
BOOL file_exist(char *fname, struct stat * sbuf)
{
   struct stat st;
   if (!sbuf)
   sbuf = &st;

   if (stat(fname, sbuf) != 0)
   return (False);

   return (S_ISREG(sbuf->st_mode));
}


/**************************************************************************/
/* Gets either a hex number (0xNNN) or decimal integer (NNN).		  */
/**************************************************************************/
uint32 get_number(const char *tmp)
{
   uint32 ret;
   if (strnequal(tmp, "0x", 2))
   {
      ret = strtoul(tmp, (char **)NULL, 16);
   }
   else
   {
      ret = strtoul(tmp, (char **)NULL, 10);
   }

   return ret;
}


/**************************************************************************/
/* Check a files mod time						  */
/**************************************************************************/
time_t file_modtime(char *fname)
{
   struct stat st;

   if (stat(fname, &st) != 0)  return (0);

   return (st.st_mtime);
}



/**************************************************************************/
/* Reduce a file name, removing .. elements.				  */
/**************************************************************************/
void dos_clean_name(char *s)
{
   char *p = NULL;

   /* remove any double slashes */
   all_string_sub(s, "\\\\", "\\", 0);

   while ((p = strstr(s, "\\..\\")) != NULL)
   {
      pstring s1;

      *p = 0;
      pstrcpy(s1, p + 3);

      if ((p = strrchr(s, '\\')) != NULL)
         *p = 0;
      else
         *s = 0;

      pstrcat(s, s1);
   }

   trim_string(s, NULL, "\\..");
   all_string_sub(s, "\\.\\", "\\", 0);
}


/**************************************************************************/
/* Reduce a file name, removing .. elements. 				  */
/**************************************************************************/
void unix_clean_name(char *s)
{
   char *p = NULL;

   /* remove any double slashes */
   all_string_sub(s, "//", "/", 0);

   /* Remove leading ./ characters */
   if (strncmp(s, "./", 2) == 0)
   {
      trim_string(s, "./", NULL);
      if (*s == 0)
      pstrcpy(s, "./");
   }

   while ((p = strstr(s, "/../")) != NULL)
   {
      pstring s1;

      *p = 0;
      pstrcpy(s1, p + 3);

      if ((p = strrchr(s, '/')) != NULL)
         *p = 0;
      else
         *s = 0;

      pstrcat(s, s1);
   }

   trim_string(s, NULL, "/..");
}


/**************************************************************************/
/* Reduce a file name, removing .. elements and checking that it is 	  */
/* below dir in the heirachy. This uses sys_getwd() and so must be 	  */
/* run on the system that has the referenced file system. widelinks are   */
/* allowed if widelinks is true						  */
/**************************************************************************/
BOOL reduce_name(char *s, char *dir, BOOL widelinks)
{
   pstring dir2;
   pstring wd;
   pstring base_name;
   pstring newname;
   char *p = NULL;
   BOOL relative = (*s != '/');

   *dir2 = *wd = *base_name = *newname = 0;

   if (widelinks)
   {
      unix_clean_name(s);
      /* can't have a leading .. */
      if (strncmp(s, "..", 2) == 0 && (s[2] == 0 || s[2] == '/'))
      {
         return (False);
      }

      if (strlen(s) == 0)  pstrcpy(s, "./");

      return (True);
   }

   all_string_sub(s, "//", "/", 0);

   pstrcpy(base_name, s);
   p = strrchr(base_name, '/');

   if (!p) return (True);

   if (!(char *)getcwd(wd, sizeof (pstring)))
   {
      return (False);
   }

   if (chdir(dir) != 0)
   {
      return (False);
   }

   if (!(char *)getcwd(dir2, sizeof (pstring)))
   {
      chdir(wd);
      return (False);
   }

   if (p && (p != base_name))
   {
      *p = 0;
      if (strcmp(p + 1, ".") == 0)
      p[1] = 0;
      if (strcmp(p + 1, "..") == 0)  *p = '/';
   }

   if (chdir(base_name) != 0)
   {
      chdir(wd);
      return (False);
   }

   if (!((char *)getcwd(newname, sizeof (pstring))))
   {
      chdir(wd);
      return (False);
   }

   if (p && (p != base_name))
   {
      pstrcat(newname, "/");
      pstrcat(newname, p + 1);
   }

   size_t l = strlen(dir2);
   if (dir2[l - 1] == '/')
   l--;

   if (strncmp(newname, dir2, l) != 0)
   {
      chdir(wd);
      return (False);
   }

   if (relative)
   {
      if (newname[l] == '/')
         pstrcpy(s, newname + l + 1);
      else
         pstrcpy(s, newname + l);
   }
   else  pstrcpy(s, newname);

   chdir(wd);
   if (strlen(s) == 0) pstrcpy(s, "./");

   return (True);
}


/**************************************************************************/
/* Become a daemon, discarding the controlling terminal			  */
/**************************************************************************/
pid_t sys_fork(void)
{
   pid_t forkret = fork();

   if (forkret == (pid_t)0)  mypid = (pid_t) -1;

   return forkret;
}


/**************************************************************************/
/* expand a pointer to be a particular size				  */
/**************************************************************************/
char *Realloc(void *p, size_t size)
{
   char *ret = NULL;

   if (size == 0)
   {
      if (p) free(p);
      return NULL;
   }

   if (!p) ret = (char *)malloc(size);
   else    ret = (char *)realloc(p, size);

   if (!ret) return (ret);

   return (NULL);
}


/**************************************************************************/
/* Protected memcpy that deals with NULL parameters.			  */
/**************************************************************************/
BOOL memcpy_zero(void *to, const void *from, size_t size)
{
	if (to == NULL)
	{
	   return False;
	}
	if (from == NULL)
	{
	   memset(to, 0, size);
	   return False;
	}

	memcpy(to, from, size);
	return True;
}


/**************************************************************************/
/* Free memory, checks for NULL						  */
/**************************************************************************/
void safe_free(void *p)
{
   if (p != NULL)
   {
      free(p);
   }
}


/**************************************************************************/
/* Provide a checksum on a string					  */
/**************************************************************************/
int str_checksum(const char *s)
{
   int res = 0;
   int c;
   int i = 0;

   while (*s)
   {
      c = *s;
      res ^= (c << (i % 15)) ^ (c >> (15 - (i % 15)));
      s++;
      i++;
   }

   return (res);
}


/**************************************************************************/
/* Zero a memory area then free it. Used to catch bugs faster		  */
/**************************************************************************/
void zero_free(void *p, size_t size)
{
   memset(p, 0, size);
   free(p);
}


/**************************************************************************/
/* Like strdup but for memory						  */
/**************************************************************************/
void *memdup(const void *p, size_t size)
{
   void *p2;
   if (!p) return NULL;
   if (size == 0) return NULL;
   p2 = malloc(size);
   if (!p2) return NULL;
   memcpy(p2, p, size);

   return p2;
}


/**************************************************************************/
/* write hex dump of buf1 with length = len to log 			  */
/**************************************************************************/
void dump_data(int level, const char *buf1, int len)
{
   uchar const *buf = (uchar const *)buf1;
   int i = 0;

   if (DEBUGLEVEL < level) return;

   if (buf == NULL)
   {
      DEBUG(level, ("log_dump: NULL, len=%d\n", len));
      return;
   }

   if (len < 0)   return;
   if (len == 0)
   {
      DEBUG(level, ("\n"));
      return;
   }

   DEBUG(level, ("[%03X] ", i));
   for (i = 0; i < len;)
   {
      DEBUGADD(level, ("%02X ", (int)buf[i]));
      i++;
      if (i % 8 == 0)
      DEBUGADD(level, (" "));
      if (i % 16 == 0)
      {
         print_asc(level, &buf[i - 16], 8);
	 DEBUGADD(level, (" "));
	 print_asc(level, &buf[i - 8], 8);
	 DEBUGADD(level, ("\n"));
	 if (i < len)
	 DEBUGADD(level, ("[%03X] ", i));
      }
   }

   if (i % 16 != 0)	/* finish off a non-16-char-length row */
   {
      int n;

      n = 16 - (i % 16);
      DEBUGADD(level, (" "));

      if (n > 8)   DEBUGADD(level, (" "));
      while (n--)  DEBUGADD(level, ("   "));

      n = MIN(8, i % 16);
      print_asc(level, &buf[i - (i % 16)], n);
      DEBUGADD(level, (" "));
      n = (i % 16) - n;

      if (n > 0)  print_asc(level, &buf[i - n], n);

      DEBUGADD(level, ("\n"));
   }
}


/**************************************************************************/
/* Write asc part of dump to logfile 					  */
/**************************************************************************/
void print_asc(int level, uchar const *buf, int len)
{
   int i;

   for (i = 0; i < len; i++)
   {
      DEBUGADD(level, ("%c", isprint(buf[i]) ? buf[i] : '.'));
   }
}


/**************************************************************************/
/* sleep for a specified number of milliseconds				  */
/**************************************************************************/
void msleep(int t)
{
   int tdiff = 0;
   struct timeval tval, t1, t2;
   fd_set fds;

   GetTimeOfDay(&t1);
   GetTimeOfDay(&t2);

   while (tdiff < t)
   {
      tval.tv_sec = (t - tdiff) / 1000;
      tval.tv_usec = 1000 * ((t - tdiff) % 1000);

      FD_ZERO(&fds);
      errno = 0;
      sys_select(0, &fds, &tval);

      GetTimeOfDay(&t2);
      tdiff = TvalDiff(&t1, &t2);
   }
}


/**************************************************************************/
/* Check if /dev/urandom exists 					  */
/**************************************************************************/
void random_check()
{
   FILE *urand;

   if ((urand = fopen("/dev/urandom","r")) != NULL)
   {
      fclose(urand);
      LOG_SYS(0, ("Init: checking for random device... /dev/urandom\n"));
      drandom = True;
   }
   else
   {
      LOG_SYS(0, ("Init: checking for random device... No\n"));
      drandom = False;
   }
}


/**************************************************************************/
/* Random buffer fill							  */
/**************************************************************************/
void random_fill(char *buffer, int size)
{
   FILE *urand;
   int i, dsize;
   unsigned short *sbuff = (unsigned short *)buffer;

   if (drandom)
   {
      /* fill buffer from /dev/urandom   */
      if ((urand = fopen("/dev/urandom","r")) != NULL)
      {
         fread(buffer, size, 1, urand);
         fclose(urand);
	 return;
      }
      else
      {
         LOG_SYS(0, ("Warn: /dev/urandom disappeared after start...\n"));
	 drandom = False;
      }
   }

   /* fill buffer via rand() function */
   init_random();

   dsize = size / sizeof(unsigned short);

   for (i=0; i<dsize; i++)
   {
      sbuff[i] = (unsigned short)rand();
   }
}

