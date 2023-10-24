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
/* tlv class, tlv_chain class						  */
/*                                                                        */
/**************************************************************************/

#ifndef _TLV_H
#define _TLV_H

/* tlv_c class to keep tlv chunk and manage its data */
class tlv_c: public Buffer
{
public:
   tlv_c(void);
   tlv_c(unsigned short type, unsigned short size);
   ~tlv_c(void);

   void reset(void) { net_order = 0; nextData = value; };

   tlv_c& operator >> (char &in);
   tlv_c& operator >> (long &in);
   tlv_c& operator >> (unsigned short &in);
   tlv_c& operator >> (unsigned int &in);
   tlv_c& operator >> (unsigned long &in);
   tlv_c& operator >> (short &in);
   
   unsigned short type;
   unsigned short size;
   unsigned short rsize;

   char *nextData;
   char *value;
   
   class tlv_c *next;

};


/* tlv_chain class to manage tlv chains */
class tlv_chain_c
{
public:
   tlv_chain_c(void);
   ~tlv_chain_c(void);
      
   void read(class Packet &pack);
   void read(class tlv_c &pack);
   void readRev(class tlv_c &pack);
   void readXXX(class tlv_c &pack);
   void readUTF(class tlv_c &pack, unsigned short &req_type);
   void readSub(class tlv_c &pack);
   void readSub2(class tlv_c &pack);
   void addToPacket(class Packet &pack);
   void free(void);
   void intel_order(void);
   void network_order(void);
   class tlv_c *get(unsigned short type);
   void remove(unsigned short type);
   void encode(char *coded_chain);
   void decode(char *coded_chain);
   int  num();
   int  num(unsigned short type);
   int  size();
   
   class tlv_c *tlv;

};

#endif /* _TLV_H */
