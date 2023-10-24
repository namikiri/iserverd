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

#include "includes.h"

class Packet tlv_buff;

/* tlv_c section */
tlv_c::tlv_c(void)
{
  /* empty */
}

tlv_c::tlv_c(unsigned short type, unsigned short size)
{
   value = NULL;
   
   if (size > 0) value = (char *)calloc(1, size);
   if ((size > 0) && (errno == ENOMEM))
   {
      if (errno == ENOMEM) 
      {
         DEBUG(10, ("ERROR: Can't allocate memory for TLV, TLV skipped\n"));
      }
      
      this->type  = 0;
      this->size  = 0;
      
      value = NULL;
   }
   else
   {
      this->type  = type;
      this->size  = size;
      this->rsize = size + sizeof(size) + sizeof(type);
   }
   
   nextData = value;
   next  = NULL;
}


tlv_c::~tlv_c(void)
{
   free(value);
   type = 0;
   size = 0;
}


tlv_c& tlv_c::operator>>(char &in)
{
   if(nextData + SIZEOF_CHAR > (value + size)) 
      in = 0;
   else
   {
      in = *((char *)nextData);
      nextData += SIZEOF_CHAR;
   }
   return(*this);
}


tlv_c& tlv_c::operator>>(unsigned short &in)
{
   if(nextData + SIZEOF_UNSIGNED_SHORT > (value + size)) 
      in = 0;
   else
   {
      in = get_le_short(nextData);
      nextData += SIZEOF_UNSIGNED_SHORT;
   }
   return(*this);
}


tlv_c& tlv_c::operator>>(unsigned int &in)
{
   if(nextData + SIZEOF_UNSIGNED_INT > (value + size)) 
      in = 0;
   else
   {
      in = get_le_int(nextData);
      nextData += SIZEOF_UNSIGNED_INT;
   }
   return(*this);
}


tlv_c& tlv_c::operator>>(unsigned long &in)
{
   if(nextData + SIZEOF_UNSIGNED_LONG > (value + size)) 
      in = 0;
   else
   {
      in = get_le_long(nextData);
      nextData += SIZEOF_UNSIGNED_LONG;
   }
   return(*this);
}


tlv_c& tlv_c::operator>>(long &in)
{
   if(nextData + SIZEOF_LONG > (value + size)) 
      in = 0;
   else
   {
      in = get_le_long(nextData);
      nextData += SIZEOF_LONG;
   }
   return(*this);
}


tlv_c& tlv_c::operator>>(short &in)
{
   if(nextData + SIZEOF_SHORT > (value + size)) 
      in = 0;
   else
   {
      in = get_le_short(nextData);
      nextData += SIZEOF_SHORT;
   }
   return(*this);
}



/**************************************************************************/
/* tlv_chain_c section 							  */
/**************************************************************************/
tlv_chain_c::tlv_chain_c()
{
   this->tlv = NULL;
}

tlv_chain_c::~tlv_chain_c()
{
   class tlv_c *btlv = this->tlv;
   class tlv_c *otlv;
   
   while (btlv)
   {
      otlv = btlv;
      btlv = btlv->next;
      delete otlv;
   }
   
   this->tlv = NULL;
   
}


/* This func read all tlvs from packet and insert them into chain */
void tlv_chain_c::read(class Packet &pack)
{
   unsigned short type;
   unsigned short size;
   class tlv_c *ttlv, *otlv;
   
   while ((pack.nextData - pack.buff) < pack.sizeVal)
   {
      pack >> type
  	   >> size;

      DEBUG(350, ("PChain TLV: type=%04X, len=%04X\n", type, size));

      /* check if we can do this */	   
      if ((pack.nextData + size) > (pack.buff + pack.sizeVal)) return;
      
      ttlv = new class tlv_c(type, size);
      if (size > 0) memcpy((void *)ttlv->value,(const void *)pack.nextData, size);

      /* tlv ready, we should insert it into chain */
      if (this->tlv == NULL)
      {
         this->tlv  = ttlv;
	 ttlv->next = NULL;
      }
      else
      {
         otlv = this->tlv;
         this->tlv  = ttlv;
         ttlv->next = otlv;
      }
      
      pack.nextData += size;
      
      if (pack.nextData >= (pack.buff + pack.sizeVal)) 
      {
         pack.nextData = pack.buff + pack.sizeVal;
	 return;
      }
   }
}


/* return total size of the chain */
int tlv_chain_c::size()
{
   class tlv_c *btlv = this->tlv;
   int size = 0;
   
   while (btlv)
   {
      size += 4; /* type&size */
      size += btlv->size;
      btlv  = btlv->next;
   }
   
   return(size);
}


/* return number of tlvs in tlv chain */
int tlv_chain_c::num()
{
   class tlv_c *btlv = this->tlv;
   int tnum = 0;
   
   while (btlv)
   {
      tnum++;
      btlv = btlv->next;
   }
   
   return(tnum);
}


/* return number of tlvs in tlv chain */
int tlv_chain_c::num(unsigned short type)
{
   class tlv_c *btlv = this->tlv;
   int tnum = 0;
   
   while (btlv)
   {
      if (btlv->type == type) tnum++;
      btlv = btlv->next;
   }
   
   return(tnum);
}


/* setup network_order extraction on all tlvs in chain */
void tlv_chain_c::network_order(void)
{
   class tlv_c *btlv = this->tlv;
   
   while (btlv)
   {
      btlv->network_order();
      btlv = btlv->next;
   }
   
   return;
}


/* setup intel_order extraction on all tlvs in chain */
void tlv_chain_c::intel_order(void)
{
   class tlv_c *btlv = this->tlv;
   
   while (btlv)
   {
      btlv->intel_order();
      btlv = btlv->next;
   }
   
   return;
}


/* return tlv_c class pointer for specified tlv type */
class tlv_c *tlv_chain_c::get(unsigned short type)
{
   class tlv_c *btlv = this->tlv;
   
   while (btlv)
   {
      if (btlv->type == type) return(btlv);
      btlv = btlv->next;
   }
   
   return(NULL);
}

/* delete specified tlv from chain */
void tlv_chain_c::remove(unsigned short type)
{
   class tlv_c *btlv = this->tlv;
   class tlv_c *otlv = NULL;
   
   while (btlv)
   {
      if (btlv->type == type)
      {
         if (otlv == NULL)
	 {
	    this->tlv = btlv->next;
	    delete btlv;
	    return;
	 }
	 else
	 {
	    otlv->next = btlv->next;
	    delete btlv;
	    return;
	 }
      }
      else
      {
         otlv = btlv;
         btlv = btlv->next;
      }
   }
}

/* deallocate memory for all tlv chunks in chain */
void tlv_chain_c::free(void)
{
   class tlv_c *btlv = this->tlv;
   class tlv_c *otlv;
   
   while (btlv)
   {
      otlv = btlv;
      btlv = btlv->next;
      delete otlv;
   }
   
   this->tlv = NULL;
   
   return;
}


/* This func read all tlvs from tlv and insert them into chain */
/* icq2002a use tlvchain in tlv */
void tlv_chain_c::read(class tlv_c &pack)
{
   unsigned short type;
   unsigned short size, osize;
   class tlv_c *ttlv, *otlv;
   
   while ((pack.nextData - pack.value) < pack.size)
   {
      pack >> type
  	   >> size;

      /* check whether size and type is null*/
      if ((type == 0) && (size == 0))
      {
         DEBUG(150, ("TChain TLV Error: type=0000, len=0000! "));
	 return;
      }
       
      DEBUG(350, ("TChain TLV: type=%04X, len=%04X\n", type, size));

      osize = size;
      /* litle QIP's tlv change */
      if ((type == 0x0136) && (size != 0x0004))
      {
         DEBUG(10, ("Rewrite QIP Search_by_Uin request in TLV: old len=%04X, new len=0x0004\n",size));
	 size = 0x0004;
      }

      /* check if we can do this */	   
      if ((pack.nextData + size) > (pack.value + pack.size)) return;
      
      ttlv = new class tlv_c(type, size);
      if (size > 0) memcpy((void *)ttlv->value,(const void *)pack.nextData, size);

      /* tlv ready, we should insert it into chain */
      if (this->tlv == NULL)
      {
         this->tlv  = ttlv;
	 ttlv->next = NULL;
      }
      else
      {
         otlv = this->tlv;
         this->tlv  = ttlv;
         ttlv->next = otlv;
      }
      
      pack.nextData += osize;
      
      if (pack.nextData >= (pack.value + pack.size)) 
      {
         pack.nextData = pack.value + pack.size;
	 return;
      }
   }
}


/* This func read all tlvs from tlv and insert them into chain */
/* icq2002a use tlvchain in tlv (this ver stores TLV in reverse order) */
void tlv_chain_c::readRev(class tlv_c &pack)
{
   unsigned short type;
   unsigned short size;
   class tlv_c *ttlv, *otlv;
   
   while ((pack.nextData - pack.value) < pack.size)
   {
      pack >> type
  	   >> size;

      DEBUG(350, ("RTChain TLV: type=%04X, len=%04X\n", type, size));

      /* check if we can do this */	   
      if ((pack.nextData + size) > (pack.value + pack.size)) return;
      
      ttlv = new class tlv_c(type, size);
      if (size > 0) memcpy((void *)ttlv->value,(const void *)pack.nextData, size);

      /* tlv ready, we should insert it into chain */
      if (this->tlv == NULL)
      {
         /* tlv chain is empty */
         this->tlv  = ttlv;
	 ttlv->next = NULL;
      }
      else
      {
         /* insert new tlv at the end */
	 otlv = this->tlv;
	 while (otlv->next != NULL) { otlv = otlv->next; }
         otlv->next = ttlv;
         ttlv->next = NULL;
      }
      
      pack.nextData += size;
      
      if (pack.nextData >= (pack.value + pack.size)) 
      {
         pack.nextData = pack.value + pack.size;
	 return;
      }
   }
}


/* This func read all tlvs from tlv and insert them into chain */
/* This is also workaround for 2003b saveinfo bug */
void tlv_chain_c::readXXX(class tlv_c &pack)
{
   unsigned short type;
   unsigned short size;
   class tlv_c *ttlv, *otlv;
   
   while ((pack.nextData - pack.value) < pack.size)
   {
      pack >> type
  	   >> size;

      /* they made a bug in 2003b saveinfo code - min size of */
      /* email tlv is 4 ---> short_len + zero byte + flag */
      if ((type == INF_TLV_EMAIL) && (size==3)) 
      {
         DEBUG(10, ("Workaround: ICQ2003b empty primary email saveinfo bug detected!\n"));
         size++;
      }

      DEBUG(350, ("XTChain TLV: type=%04X, len=%04X\n", type, size));

      /* check if we can do this */	   
      if ((pack.nextData + size) > (pack.value + pack.size)) return;
      
      ttlv = new class tlv_c(type, size);
      if (size > 0) memcpy((void *)ttlv->value,(const void *)pack.nextData, size);

      /* tlv ready, we should insert it into chain */
      if (this->tlv == NULL)
      {
         this->tlv  = ttlv;
	 ttlv->next = NULL;
      }
      else
      {
         otlv = this->tlv;
         this->tlv  = ttlv;
         ttlv->next = otlv;
      }
      
      pack.nextData += size;
      
      if (pack.nextData >= (pack.value + pack.size)) 
      {
         pack.nextData = pack.value + pack.size;
	 return;
      }
   }
}


void tlv_chain_c::addToPacket(class Packet &pack)
{
   class tlv_c *btlv = this->tlv;
   
   while (btlv)
   {
      /* check if tlv is fit there */
      if ((unsigned int)abs(btlv->rsize+pack.sizeVal) > pack.maxSize) break;
      
      /* copy tlv to packet */
      pack << btlv->type
           << btlv->size;
	   
      memcpy(pack.nextData, btlv->value, btlv->size);
      pack.sizeVal  += btlv->size;
      pack.nextData += btlv->size;

      btlv = btlv->next;
   }
}   


/* code whole tlv chain into ascii string */
void tlv_chain_c::encode(char *coded_chain)
{
   tlv_buff.clearPacket();
   tlv_buff.setup_aim();
   
   this->addToPacket(tlv_buff);
   hexencode(coded_chain, tlv_buff.buff, this->size());
}


/* decode tlv chain from ascii string */
void tlv_chain_c::decode(char *coded_chain)
{
   tlv_buff.clearPacket();
   tlv_buff.setup_aim();
   
   hexdecode(tlv_buff.buff, coded_chain, sizeof(tlv_buff.buff)-1);
   tlv_buff.sizeVal = strlen(coded_chain) / 2;
   this->read(tlv_buff);
}

void tlv_chain_c::readUTF(class tlv_c &pack, unsigned short &req)
{
   unsigned short type, type2;
   unsigned short size;
   class tlv_c *ttlv, *otlv;
   unsigned short junk;
     
   pack >> size
	>> junk
	>> type2;

   DEBUG(350, ("UTFChain TLV: type=%04X, len=%04X\n", ntohs(type2), ntohs(size)));

   for(int i=0;i<6;i++)
         pack >> junk;

   if (type2 != 0x0fd2)  pack >> junk;
   
   pack >> type;
   
   req = type;
   
   DEBUG(350, ("UTFChain request type=%04X\n", type));

   if (type2 != 0x0fd2)  pack >> junk;

   pack >> type
	>> size;
						   
   while ((pack.nextData - pack.value) < pack.size)
   {

      pack >> type;

      pack >> size;

      /* check whether size and type is null*/
      if ((type == 0) && (size == 0))
      {
         DEBUG(150, ("UTFChain TLV Error: type=0000, len=0000! "));
	 return;
      }
       
      DEBUG(350, ("UTFChain TLV: type=%04X, len=%04X\n", type, size));

      /* check if we can do this */	   
      if ((pack.nextData + size) > (pack.value + pack.size)) return;

      ttlv = new class tlv_c(type, size);
      if (size > 0) memcpy((void *)ttlv->value,(const void *)pack.nextData, size);

      /* tlv ready, we should insert it into chain */
      if (this->tlv == NULL)
      {
         this->tlv  = ttlv;
	 ttlv->next = NULL;
      }
      else
      {
         otlv = this->tlv;
         this->tlv  = ttlv;
         ttlv->next = otlv;
      }
      
      pack.nextData += size;
      
      if (pack.nextData >= (pack.value + pack.size)) 
      {
         pack.nextData = pack.value + pack.size;
	 return;
      }
   }
}

void tlv_chain_c::readSub(class tlv_c &pack)
{
   unsigned short type;
   unsigned short size;
   class tlv_c *ttlv, *otlv;
      
   pack >> type
	>> size;

   DEBUG(350, ("UTFChain main sub TLV: type=%04X, len=%04X\n", type, size));
						   
   while ((pack.nextData - pack.value) < pack.size)
   {

      pack >> type;
      
      pack >> size;

      /* check whether size and type is null*/
      if ((type == 0) && (size == 0))
      {
         DEBUG(150, ("UTFChain sub TLV Error: type=0000, len=0000! "));
	 return;
      }
       
      DEBUG(350, ("UTFChain sub TLV: type=%04X, len=%04X\n", type, size));

      /* check if we can do this */	   
      if ((pack.nextData + size) > (pack.value + pack.size)) return;

      ttlv = new class tlv_c(type, size);
      if (size > 0) memcpy((void *)ttlv->value,(const void *)pack.nextData, size);

      /* tlv ready, we should insert it into chain */
      if (this->tlv == NULL)
      {
         this->tlv  = ttlv;
	 ttlv->next = NULL;
      }
      else
      {
         otlv = this->tlv;
         this->tlv  = ttlv;
         ttlv->next = otlv;
      }
      
      pack.nextData += size;
      
      if (pack.nextData >= (pack.value + pack.size)) 
      {
         pack.nextData = pack.value + pack.size;
	 return;
      }
   }
}

void tlv_chain_c::readSub2(class tlv_c &pack)
{
   unsigned short type, size, cnt;
   class tlv_c *ttlv, *otlv;
     
   pack.no_null_terminated();
   pack >> cnt;
   
   DEBUG(350, ("UTFChain main sub(cnt) TLV: cnt=%04X\n", cnt));
   
   for(unsigned short i=0;i<cnt;i++)
   {
	
	pack >> size;
	
	DEBUG(350, ("UTFChain sub(cnt) TLV: size=%04X\n", size));
	
	if ((pack.nextData - pack.value) < pack.size)
	{
	    pack >> type
		 >> size;

	    if ((type == 0) && (size == 0))
	    {
		DEBUG(150, ("UTFChain sub(cnt) TLV Error: type=0000, len=0000! "));
		return;
	    }
	    
	    DEBUG(350, ("UTFChain sub(cnt) TLV: type=%04X, len=%04X\n", type, size));

	    /* check if we can do this */
	    if ((pack.nextData + size) > (pack.value + pack.size)) return;
	    
	    ttlv = new class tlv_c(type, size);
	    if (size > 0) memcpy((void *)ttlv->value,(const void *)pack.nextData, size);
	    
	    /* tlv ready, we should insert it into chain */
	    if (this->tlv == NULL)
	    {
		this->tlv  = ttlv;
		ttlv->next = NULL;
	    }
	    else
	    {
		otlv = this->tlv;
		this->tlv  = ttlv;
		ttlv->next = otlv;
	    }
	    
	    this->tlv->type += (i*0x1000);
	    
	    pack.nextData += size;

    	    if (pack.nextData >= (pack.value + pack.size))
	    {
		pack.nextData = pack.value + pack.size;
		return;
	    }
	}

	if ((pack.nextData - pack.value) < pack.size)
	{
	    pack >> type
		 >> size;

	    if ((type == 0) && (size == 0))
	    {
		DEBUG(150, ("UTFChain sub(cnt) TLV Error: type=0000, len=0000! "));
		return;
	    }
	    
	    DEBUG(350, ("UTFChain sub(cnt) TLV: type=%04X, len=%04X\n", type, size));

	    /* check if we can do this */
	    if ((pack.nextData + size) > (pack.value + pack.size)) return;
	    
	    ttlv = new class tlv_c(type, size);
	    if (size > 0) memcpy((void *)ttlv->value,(const void *)pack.nextData, size);
	    
	    /* tlv ready, we should insert it into chain */
	    if (this->tlv == NULL)
	    {
		this->tlv  = ttlv;
		ttlv->next = NULL;
	    }
	    else
	    {
		otlv = this->tlv;
		this->tlv  = ttlv;
		ttlv->next = otlv;
	    }
	    
	    this->tlv->type += (i*0x1000);
	    
	    pack.nextData += size;

    	    if (pack.nextData >= (pack.value + pack.size))
	    {
		pack.nextData = pack.value + pack.size;
		return;
	    }
	}


   }
}
