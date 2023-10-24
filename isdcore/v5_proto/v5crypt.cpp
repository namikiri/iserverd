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
/* Implements V5 packets crypting/descypting and checksum calculating	  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#include "v5crypt.h"

/**************************************************************************/
/* This will encrypt v5 version packet 					  */
/**************************************************************************/
void V5Encrypt(Packet &pack)
{

   unsigned long cc = calculate_checkcode(pack);
    
    /* Insert the checkcode at position 0x14 */
   long size = sizeof(cc);
   for(int i = 0, j = 0x14; i < size; i++)
		pack.buff[j++] = cc >> (i * 8);

   /* Calculate the encryption key (key) */
   unsigned long key = pack.sizeVal * 0x68656c6c + cc;

   /* Encrypt the byte array */
   for(int pos = 0xa, slab = SLAB(pack.sizeVal, pos); slab > 0; 
                      slab = SLAB(pack.sizeVal, pos))
   {
      unsigned long tmpUint = 0;
      for(int leftI = slab - 1; leftI >= 0; leftI--)
      {
         tmpUint <<= 8;
         tmpUint |= pack.buff[pos + leftI];
      }

      tmpUint ^= key + v5table[pos & 0xFF];

      for(int rightI = 0; rightI < slab; rightI++)
              pack.buff[pos++] = tmpUint >> (rightI * 8);
   }

   PutKey(pack, cc);
}


/**************************************************************************/
/* This will create checkcode for packets				  */
/**************************************************************************/
unsigned long calculate_checkcode(Packet &pack)
{
  /* number1 is used for calculating the checkcode (cc) */
   unsigned long cc;
   unsigned char B2, B4, B6, B8;
   unsigned char X1, X2, X3, X4;
  
   
   unsigned long number1 = 0;
   B2 = pack.buff[2];
   B4 = pack.buff[4];
   B6 = pack.buff[6];
   B8 = pack.buff[8];
   
   number1 += B8; number1 <<= 8; 
   number1 += B4; number1 <<= 8;
   number1 += B2; number1 <<= 8;
   number1 += B6;  
   
  /* r1 and r2 are used for calculating number2 */
   unsigned short r1 = rand() % 0x10;
   unsigned short r2 = rand() % 0xff;

  /* number2 is used for calculating the checkcode (cc) */
   unsigned long number2 = 0;
   
   X4 = r1;
   X3 = pack.buff[X4];
   X2 = r2;
   X1 = v5table[X2];
   
   number2 += X4; number2 <<= 8;
   number2 += X3; number2 <<= 8;
   number2 += X2; number2 <<= 8;
   number2 += X1;
   
   number2 ^= 0x00ff00ff;
   cc = number1 ^ number2;
   
   /* Here's the checkcode, cc */
   return (cc);

}


/**************************************************************************/
/* This will decrypt v5 version packet 					  */
/**************************************************************************/
void V5Decrypt(Packet &pack)
{
   int i;
   unsigned long k;
   unsigned long key;
   
   key = GetKey(pack);
   
   for (i=0x0a; i < pack.sizeVal+3; i+=4 )
   {
      k = key + v5table[i&0xff];
      
      if ( i != 0x16 )
      {
	 pack.buff[i]   ^= (unsigned char)( k & 0x000000ff);
	 pack.buff[i+1] ^= (unsigned char)((k & 0x0000ff00)>>8);
      }

      if ( i != 0x12 ) 
      {
	 pack.buff[i+2] ^= (unsigned char)((k & 0x00ff0000)>>16);
	 pack.buff[i+3] ^= (unsigned char)((k & 0xff000000)>>24);
      }
   }
}


/**************************************************************************/
/* Used to correct data because of byte_ordering 			  */
/**************************************************************************/
unsigned long ReverseLong(unsigned long l)
{
   if (rvs_order)
   {
       unsigned char z[4];

       z[3] = (unsigned char)((l)>>24) & 0x000000FF;
       z[2] = (unsigned char)((l)>>16) & 0x000000FF;
       z[1] = (unsigned char)((l)>>8)  & 0x000000FF;
       z[0] = (unsigned char)(l) & 0x000000FF;

       return *((unsigned long *)z);
    }
    else
    {
       return l;
    }
}


/**************************************************************************/
/* Used to correct data because of byte_ordering 			  */
/**************************************************************************/
unsigned short ReverseShort(unsigned short l)
{
   if (rvs_order)
   {
      unsigned char z[2];

      z[1] = (unsigned char)((l)>>8)  & 0x000000FF;
      z[0] = (unsigned char)(l) & 0x000000FF;

      return *((unsigned short *)z);
   }
   else
   {
      return l;
   }
}


/**************************************************************************/
/* Get and descramble check code from packet 				  */
/**************************************************************************/
unsigned long GetKey(Packet &pack)
{
    unsigned int A[6];
    unsigned int key;
    unsigned int check;

    check = ReverseLong(*(unsigned long *) (&(pack.buff[0x14])));

    A[1] = check & 0x0001F000;
    A[2] = check & 0x07C007C0;
    A[3] = check & 0x003E0001;
    A[4] = check & 0xF8000000;
    A[5] = check & 0x0000083E;

    A[1] = A[1] >> 0x0C;
    A[2] = A[2] >> 0x01;
    A[3] = A[3] << 0x0A;
    A[4] = A[4] >> 0x10;
    A[5] = A[5] << 0x0F;
    check = A[5] + A[1] + A[2] + A[3] + A[4];

    key  = pack.sizeVal * 0x68656C6C;
    key += check;
    return key;
}


/**************************************************************************/
/* Scramble check code and put it in packet  				  */
/**************************************************************************/
void PutKey(Packet &pack, unsigned long cc)
{
    unsigned long cc_size;
    unsigned long check;
    unsigned long i, j;

    /* server checkcode are not scrambled */
    check = ReverseLong(*(unsigned long *) &cc);

    /* Insert the scrambled checkcode at position 0x14 */
    cc_size = sizeof(check);
    for(i = 0, j = 0x11; i < cc_size; i++)
    	pack.buff[j++] = check >> (i * 8);

}


/**************************************************************************/
/* Put sseq in packet			 				  */
/**************************************************************************/
void PutSeq(Packet &pack, unsigned short cc)
{
    unsigned short cc_size;
    unsigned short check;
    unsigned short i, j;

    /* server checkcode are not scrambled */
    check = ReverseShort(*(unsigned short *) &cc);

    /* Insert the scrambled checkcode at position 0x14 */
    cc_size = sizeof(check);
    for(i = 0, j = 0x09; i < cc_size; i++)
    	pack.buff[j++] = check >> (i * 8);

}

