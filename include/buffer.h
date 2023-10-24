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
/* Base packet class definition 					  */
/*									  */
/**************************************************************************/

#ifndef _BUFFER_H
#define _BUFFER_H

/* abstract class buffer - Packet & tlv classes parent */
class Buffer
{
 public:
   Buffer(void);
   virtual ~Buffer(void);
   
   void setup_aim(void) { network_order(); no_null_terminated(); };
   void network_order(void) { net_order = 1; };
   void intel_order(void) { net_order = 0; };
   void no_null_terminated(void) { asciiz = 0; };
   void null_terminated(void) { asciiz = 1; };

   /* clear virtual function */
   virtual void reset(void) = 0;

   unsigned short get_le_short(char *p);
   unsigned int get_le_int(char *p);
   unsigned long get_le_long(char *p);
   void put_le_short(unsigned short x, char *p);
   void put_le_int(unsigned int x, char *p);
   void put_le_long(unsigned long x, char *p);
   
   unsigned short asciiz;
   unsigned short net_order;

};

#endif /* _BUFFER_H */
