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
#define TALLOC_ALIGN 32
#define TALLOC_CHUNK_SIZE (0x2000)

TALLOC_CTX *talloc_init(void)
{
   TALLOC_CTX *t;

   t = (TALLOC_CTX *)malloc(sizeof(*t));
   if (!t) return NULL;

   t->list = NULL;

   return t;
}


void *talloc(TALLOC_CTX *t, size_t size)
{
   void *p;

   size = (size + (TALLOC_ALIGN-1)) & ~(TALLOC_ALIGN-1);
   if (!t->list || (t->list->total_size - t->list->alloc_size) < size)
   {
      struct talloc_chunk *c;
      size_t asize = (size + (TALLOC_CHUNK_SIZE-1)) & ~(TALLOC_CHUNK_SIZE-1);

      c = (struct talloc_chunk *)malloc(sizeof(*c));
      if (!c) return NULL;
      c->next = t->list;
      c->ptr = (void *)malloc(asize);
      if (!c->ptr)
      {
         free(c);
         return NULL;
      }

      c->alloc_size = 0;
      c->total_size = asize;
      t->list = c;
   }

   p = ((char *)t->list->ptr) + t->list->alloc_size;
   t->list->alloc_size += size;
   return p;
}


void talloc_destroy(TALLOC_CTX *t)
{
   struct talloc_chunk *c;
	
   while (t->list)
   {
      c = t->list->next;
      free(t->list->ptr);
      free(t->list);
      t->list = c;
   }

   free(t);

}

