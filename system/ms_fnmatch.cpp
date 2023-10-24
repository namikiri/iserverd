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
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* Microsoft-style pattern expander 					  */
/**************************************************************************/
int ms_fnmatch(char *pattern, char *string)
{
   char *p = pattern, *n = string;
   char c;

   while ((c = *p++))
   {
      switch (c)
      {
         case '?':
                   if (! *n) return -1;
                   n++;
                   break;
         case '>':
                   if (n[0] == '.')
                   {
                      if (! n[1] && ms_fnmatch(p, n+1) == 0) return 0;
                      if (ms_fnmatch(p, n) == 0) return 0;
                      return -1;
                   }

                   if (! *n) return ms_fnmatch(p, n);
                   n++;
                   break;
         case '*':
                   for (; *n; n++)
                   {
                      if (ms_fnmatch(p, n) == 0) return 0;
                   }

                   break;
         case '<':
                   for (; *n; n++)
                   {
                      if (ms_fnmatch(p, n) == 0) return 0;
                      if (*n == '.' && !strchr(n+1,'.'))
                      {
                         n++;
                         break;
                      }
                   }
                   break;
         case '"':
                   if (*n == 0 && ms_fnmatch(p, n) == 0) return 0;
                   if (*n != '.') return -1;
                   n++;
                   break;

         default:
                   if (c != *n) return -1;
                   n++;
      }
   }

   if (! *n) return 0;
	
   return -1;

}

