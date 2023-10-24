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
/*                                                                        */
/**************************************************************************/

#ifndef _PACKET_H
#define _PACKET_H

class Packet: public Buffer
{
public:
   Packet(void);
   Packet(Packet *packet);
   Packet(char *newBuff, unsigned long buffSize);
   ~Packet(void);
   char *print(void);
   
   void clearPacket(void);
   void reset(void) { asciiz = 1; net_order = 0; nextData = buff; };
   void append(void) { asciiz = 1; net_order = 0; nextData = buff + sizeVal; };

   Packet& operator << (char data);
   Packet& operator << (char *data);
   Packet& operator << (long data);
   Packet& operator << (unsigned short data);
   Packet& operator << (unsigned int data);
   Packet& operator << (unsigned long data);

   Packet& operator >> (char &in);
   Packet& operator >> (long &in);
   Packet& operator >> (unsigned short &in);
   Packet& operator >> (unsigned int &in);
   Packet& operator >> (unsigned long &in);
   Packet& operator >> (short &in);
   unsigned long size(void) { return(sizeVal);};
   
   char *nextData;
   long sock_hdl;
    
   unsigned short from_local;
   unsigned long  sock_rnd;
   unsigned long  seq_number;
   unsigned short sock_type;
   unsigned short flap_channel;
   unsigned short sock_evt;
   unsigned int maxSize;
   unsigned int from_port;
   struct in_addr from_ip;
   int sizeVal;
   char buff[MAX_PACKET_SIZE+1024];

};

#endif
