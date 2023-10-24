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
/* File locking functions 						  */
/*									  */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* Lock file using best available mechanizm				  */
/**************************************************************************/
int Lock(FILE *fp)
{
   int                rc = -1;
#ifdef HAVE_FCNTL
   struct flock       lock;
#endif

   if (fp == NULL)
   {
      return(-1);
   }

#ifdef HAVE_FLOCK
   rc = flock(fileno(fp), LOCK_EX|LOCK_NB);
#elif HAVE_FCNTL
   lock.l_type = F_WRLCK;
   lock.l_start = 0L;
   lock.l_whence = SEEK_SET;
   lock.l_len = 0L;
   lock.l_pid = getpid();
   rc = fcntl(fileno(fp), F_SETLK, &lock);
#elif HAVE_LOCKF
#warn "lockf() file locking not supported!"
#elif HAVE_FLOCKFILE
   if (ftrylockfile(fp) == 0)
   {
      flockfile(fp);
      rc = 0;
   }
#endif /* HAVE_FLOCK */

   return(rc);

}


/**************************************************************************/
/* Unlock file using best available mechanizm				  */
/**************************************************************************/
int Unlock(FILE *fp)
{
   int                rc = -1;
#ifdef HAVE_FCNTL
   struct flock       lock;
#endif

   if (fp == NULL)
   {
      return(-1);
   }

#ifdef HAVE_FLOCK
   rc = flock(fileno(fp), LOCK_UN|LOCK_NB);
#elif HAVE_FCNTL
   lock.l_type = F_UNLCK;
   lock.l_start = 0L;
   lock.l_whence = SEEK_SET;
   lock.l_len = 0L;
   lock.l_pid = getpid();
   rc = fcntl(fileno(fp), F_SETLK, &lock);
    
#elif HAVE_LOCKF
#warn "lockf() file unlocking not supported!"
#elif HAVE_FLOCKFILE
   funlockfile(fp);
   rc = 0;
#endif /* HAVE_FLOCK */

   return(rc);
}

