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
/* One of IServerd basic units: Packet class				  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

Packet::Packet(void)
{
   maxSize = MAX_PACKET_SIZE;
   nextData = buff;
   sizeVal = 0;
}


/* packet class constructor */
Packet::Packet(Packet *packet)
{
   maxSize = packet->maxSize;
   nextData = packet->nextData;
   sizeVal = packet->sizeVal;   
   for (long i = 0; i < sizeVal; i++) buff[i] = packet->buff[i];
}



Packet::Packet(char *newBuff, unsigned long buffSize)
{
   maxSize = MAX_PACKET_SIZE;
   if (buffSize > maxSize) buffSize = maxSize;
   memcpy(buff, newBuff, buffSize);
   nextData = buff;
   sizeVal = buffSize;
}


/* packet destructor */
Packet::~Packet()
{

}


char *Packet::print(void)
{
    static char p[2048];
    char *pPos = p;

    slprintf(pPos, sizeof(pPos)-1, "  ");
    pPos += 2;
    for(long i = 0; i < sizeVal; i++)
    {
       slprintf(pPos, sizeof(pPos)-1, "%02X ", (unsigned char)buff[i]);
       pPos += 3;
       if((i + 1) % 24 == 0) { slprintf(pPos, sizeof(pPos)-1, "\n  "); pPos += 3; }
    }
    slprintf(pPos, sizeof(pPos)-1, "\n%c", '\0');
    return(p);
}


void Packet::clearPacket(void)
{
   net_order = 0;
   nextData = buff;
   sizeVal = 0;
   asciiz = 1;
}


Packet& Packet::operator<<(long data)
{
   put_le_long(data, nextData);
   sizeVal += SIZEOF_LONG;
   nextData += SIZEOF_LONG;
   return(*this);
}

Packet& Packet::operator<<(unsigned int data)
{
   put_le_int(data, nextData);
   sizeVal += SIZEOF_UNSIGNED_INT;
   nextData += SIZEOF_UNSIGNED_INT;
   return(*this);    
}


Packet& Packet::operator<<(unsigned short data)
{
   put_le_short(data, nextData);
   sizeVal += SIZEOF_UNSIGNED_SHORT;
   nextData += SIZEOF_UNSIGNED_SHORT;
   return(*this);    
}


Packet& Packet::operator<<(unsigned long data)
{
   put_le_long(data, nextData);
   sizeVal += SIZEOF_UNSIGNED_LONG;
   nextData += SIZEOF_UNSIGNED_LONG;
   return(*this);    
}


Packet& Packet::operator<<(char data)
{
   int s = sizeof(unsigned char);
   memcpy(nextData, &data, s);
   sizeVal += s;
   nextData += s;
   return(*this);    
}


Packet& Packet::operator<<(char *data)
{
   int s;
   
   if (asciiz == 1)
   {
      s = strlen((char *)data) + 1;
   }
   else
   {
      s = strlen((char *)data);
   }
   memcpy(nextData, data, s);
   sizeVal += s;
   nextData += s;
   return(*this);
}


Packet& Packet::operator>>(char &in)
{
   if(nextData + SIZEOF_CHAR > (buff + sizeVal)) 
      in = 0;
   else
   {
      in = *((char *)nextData);
      nextData += SIZEOF_CHAR;
   }
   return(*this);
}


Packet& Packet::operator>>(unsigned short &in)
{
   if(nextData + SIZEOF_UNSIGNED_SHORT > (buff + sizeVal)) 
      in = 0;
   else
   {
      in = get_le_short(nextData);
      nextData += SIZEOF_UNSIGNED_SHORT;
   }
   return(*this);
}


Packet& Packet::operator>>(unsigned int &in)
{
   if(nextData + SIZEOF_UNSIGNED_INT > (buff + sizeVal)) 
      in = 0;
   else
   {
      in = get_le_int(nextData);
      nextData += SIZEOF_UNSIGNED_INT;
   }
   return(*this);
}


Packet& Packet::operator>>(unsigned long &in)
{
   if(nextData + SIZEOF_UNSIGNED_LONG > (buff + sizeVal)) 
      in = 0;
   else
   {
      in = get_le_long(nextData);
      nextData += SIZEOF_UNSIGNED_LONG;
   }
   return(*this);
}


Packet& Packet::operator>>(long &in)
{
   if(nextData + SIZEOF_LONG > (buff + sizeVal)) 
      in = 0;
   else
   {
      in = get_le_long(nextData);
      nextData += SIZEOF_LONG;
   }
   return(*this);
}


Packet& Packet::operator>>(short &in)
{
   if(nextData + SIZEOF_SHORT > (buff + sizeVal)) 
      in = 0;
   else
   {
      in = get_le_short(nextData);
      nextData += SIZEOF_SHORT;
   }
   return(*this);
}


