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
/* Various functions related files processing				  */
/*									  */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* Return filesize of file with specified filename			  */
/**************************************************************************/
off_t get_file_size(char *file_name)
{
   struct stat buf;
   buf.st_size = 0;

   if (stat(file_name, &buf) != 0) return (off_t) - 1;

   return (buf.st_size);
}


/**************************************************************************/
/* Return filesize of file with specified opened file handler		  */
/**************************************************************************/
off_t get_file_size(int fd)
{
   struct stat buf;
   buf.st_size = 0;
   if (fstat(fd, &buf) != 0) return (off_t) - 1;

   return (buf.st_size);
}


/**************************************************************************/
/* Obtains file modified date.						  */
/**************************************************************************/
BOOL file_modified_date(const char *filename, time_t *lastmodified)
{
   struct stat st;

   if (stat(filename, &st) != 0) return False;

   *lastmodified = st.st_mtime;
   return True;
}


/**************************************************************************/
/* Checks if a file has changed since last read				  */
/**************************************************************************/
BOOL file_modified(const char *filename, time_t *lastmodified)
{
   time_t mtime;
   if (!file_modified_date(filename, &mtime)) return False;
   if (mtime <= *lastmodified) return False;

   *lastmodified = mtime;
   return True;
}


/**************************************************************************/
/* Load a file into memory						  */
/**************************************************************************/
char *file_load(char *fname, size_t *size)
{
   int fd;
   struct stat sbuf;
   char *p;

   if (!fname || !*fname) return NULL;
   fd = open(fname,O_RDONLY);
   if (fd == -1) return NULL;
   if (fstat(fd, &sbuf) != 0) return NULL;
   if (sbuf.st_size == 0) return NULL;

   p = (char *)malloc(sbuf.st_size+1);
   if (!p) return NULL;

   if (read(fd, p, sbuf.st_size) != sbuf.st_size)
   {
      free(p);
      return NULL;
   }

   p[sbuf.st_size] = 0;
   close(fd);

   if (size) *size = sbuf.st_size;

   return p;
}

