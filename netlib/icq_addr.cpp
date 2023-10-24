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
/* 									  */
/* Converting from ICQ unsigned long to in_addr and back		  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* return IP address in a format suitable to be transmitted to ICQ via	  */
/* packet class. ICQ uses big endian format, packet class return little	  */
/* endian longs, so we need a conversion from little to big endian.	  */
/* IP is represented in native format and converted to big-endian by	  */
/* htonl(); "aa.bb.cc.dd" must be passed to packet class as 0xddccbbaa	  */
/* on entry, l is in native format:					  */
/*    0xaabbccdd on Sparcs (big endian)					  */
/*    0xddccbbaa on Intels (little endian)				  */
/* after htonl(), l is in big-endian (net format) 0xaabbccdd to pass 	  */
/* to ICQ, l is converted to little endian) 				  */
/**************************************************************************/
unsigned long ipToIcq(struct in_addr ltemp)
{
  unsigned long l;
  
  l = (unsigned long)ltemp.s_addr;
  l = htonl(l);
  
  return (l << 24) | ((l & 0xff00) << 8) | ((l & 0xff0000) >> 8) | (l >> 24);
}

unsigned long ipToIcq2(struct in_addr ltemp)
{
  unsigned long l;
  
  l = (unsigned long)ltemp.s_addr;
  
  return (l << 24) | ((l & 0xff00) << 8) | ((l & 0xff0000) >> 8) | (l >> 24);
}

/**************************************************************************/
/* return IP address in a format suitable to be used by socket from a 	  */
/* long returned by packet class. ICQ uses big endian format, packet 	  */
/* class return little endian longs, so we need a conversion from little  */
/* to big endian. IP is returned in native format by ntohl();		  */
/* "aa.bb.cc.dd" is received from packet class as 0xddccbbaa.		  */
/* 0xddccbbaa is converted to net format (big endian) and than back to 	  */
/* host via ntohl(). 							  */
/**************************************************************************/
struct in_addr icqToIp(unsigned long l)
{ 
  struct in_addr atemp;
  
  atemp.s_addr = (u_int32_t)(ntohl((l << 24) | ((l & 0xff00) << 8) | 
			          ((l & 0xff0000) >> 8) | (l >> 24)));

  return atemp;  
}


struct in_addr icqToIp2(unsigned long l)
{ 
  unsigned long  ltemp;
  struct in_addr atemp;
  
  ltemp = ((l << 24) | ((l & 0xff00) << 8) | 
			    ((l & 0xff0000) >> 8) | 
			    (l >> 24));

  atemp.s_addr = (u_int32_t)ltemp;
  
  return atemp;
  
}

