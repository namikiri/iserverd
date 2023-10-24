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
/* IServerd basic unit: abstract buffer class (Packet & tlv parent)	  */
/*									  */
/**************************************************************************/

#include "includes.h"

/* buffer constructor */
Buffer::Buffer(void)
{

}


/* buffer destructor */
Buffer::~Buffer()
{

}


unsigned short Buffer::get_le_short(char *p)
{
   unsigned char *q = (unsigned char *)p;
   if (net_order == 0)
   {
      return q[0] + (q[1] << 8);
   }
   else
   {
      return q[1] + (q[0] << 8);
   }
}


unsigned int Buffer::get_le_int(char *p)
{
   unsigned char *q = (unsigned char *)p;
   if (net_order == 0)
   {
      return q[0] + (q[1] << 8) + (q[2] << 16) + (q[3] << 24);
   }
   else
   {
      return q[3] + (q[2] << 8) + (q[1] << 16) + (q[0] << 24);
   }
}


unsigned long Buffer::get_le_long(char *p)
{
   unsigned char *q = (unsigned char *)p;
   if (net_order == 0)
   {
      return q[0] + (q[1] << 8) + (q[2] << 16) + (q[3] << 24);
   }
   else
   {
      return q[3] + (q[2] << 8) + (q[1] << 16) + (q[0] << 24);
   }
}


void Buffer::put_le_short(unsigned short x, char *p)
{
   unsigned char *q = (unsigned char*)p;
   if (net_order == 0)
   {
      q[0] = x & 0xff;
      q[1] = (x >> 8) & 0xff;
   }
   else
   {
      q[1] = x & 0xff;
      q[0] = (x >> 8) & 0xff;   
   }
}


void Buffer::put_le_int(unsigned int x, char *p)
{
   unsigned char *q = (unsigned char*)p;
   if (net_order == 0)
   {
      q[0] = x & 0xff;
      q[1] = (x >> 8) & 0xff;
      q[2] = (x >> 16) & 0xff;
      q[3] = (x >> 24) & 0xff;
   }
   else     
   {
      q[3] = x & 0xff;
      q[2] = (x >> 8) & 0xff;
      q[1] = (x >> 16) & 0xff;
      q[0] = (x >> 24) & 0xff;
   }
}


void Buffer::put_le_long(unsigned long x, char *p)
{
   unsigned char *q = (unsigned char*)p;
   if (net_order == 0)
   {
      q[0] = x & 0xff;
      q[1] = (x >> 8) & 0xff;
      q[2] = (x >> 16) & 0xff;
      q[3] = (x >> 24) & 0xff;
   }
   else
   {
      q[3] = x & 0xff;
      q[2] = (x >> 8) & 0xff;
      q[1] = (x >> 16) & 0xff;
      q[0] = (x >> 24) & 0xff;
   }
}


