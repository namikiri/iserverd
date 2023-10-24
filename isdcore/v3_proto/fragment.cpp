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
/* This file implement functions that gather fragmented packets and 	  */
/* split big packet into small fragments				  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#include "v3_defines.h"

extern int v3_timeout; extern int v3_retries;

/**************************************************************************/
/* This used to merge packet fragments				   	  */
/* What is fragmented packets ? It is usuall big packet splited by 	  */
/* client into several small fragments. Server should merge all    	  */
/* fragments and then inject result packet into processing pipe	   	  */
/**************************************************************************/
void v3_defrag_packet()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long  uin_num, temp_stamp;
   struct online_user user;
   char cpart_num, cpart_cnt;
   int part_num, part_cnt, len;
   struct defrag_pack dpack;
 
   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num;
 
   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
          (user.udp_port == int_pack.from_port))
      {
         v3_send_ack(int_pack);
      
         part_num = 0; part_cnt = 0;
      
         int_pack.reset(); 
	 part_num = 0; 
	 part_cnt = 0;
	 
         int_pack >> pvers 
	          >> pcomm 
		  >> seq1 
		  >> seq2 
                  >> uin_num 
		  >> temp_stamp
                  >> cpart_num 
		  >> cpart_cnt;
      
         part_num = cpart_num; 
	 part_cnt = cpart_cnt;
      
         len = int_pack.sizeVal - 18;  /* packet len - header len */
      
         /* now we should notify DP to check fragments */
         db_defrag_addpart(uin_num, seq2, part_num, part_cnt, 
                           len, int_pack.nextData);
	
         dpack.mtype   = MESS_DEFRAG;
         dpack.uin_num = uin_num;
         dpack.seq     = seq2;
      
         defrag_send_pipe(dpack);
      
      }
      else
      {
    
         LOG_ALARM(0, ("Spoofed packet fragment from %s:%d (%lu)\n", 
         inet_ntoa(int_pack.from_ip), int_pack.from_port, uin_num));
         v3_send_not_connected(int_pack);
      }   
   }
   else
   {
      v3_send_not_connected(int_pack);
   }
}


/**************************************************************************/  
/* This used to split packet into fragments			   	  */
/* Sometimes when we need to send BIG packet (for example it can   	  */
/* be a user offline messsage) we should split it into small       	  */
/* fragments because v3 client don't expect packets greater than   	  */
/* 500 bytes, but sometime it accept such packets :) 		   	  */
/**************************************************************************/
int v3_send_packet_in_fragments(Packet &pack, unsigned long to_uin, 
                                unsigned long shm_index)
{ 
   int part_cnt=0, last_len=0, i, part_cnt_r;
   unsigned long tstamp;
   unsigned short seq2;
  
   DEBUG(50, ("We should sent big packet in fragments, so lets go!\n"));

   /* now we should split packet into fragments and send them all */
   part_cnt = pack.sizeVal / (lp_v3_packet_mtu()-18);
   last_len = pack.sizeVal % (lp_v3_packet_mtu()-18);
   if (last_len !=0) 
   {
      part_cnt_r = part_cnt + 1;
   }
   else
   {
      part_cnt_r = part_cnt;
   }
  
   /* also we need seq2 from splited packet */
   pack.reset(); pack >> tstamp >> seq2 >> seq2; pack.reset();
  
   /* send all fragments with mtu-len with the same seq2 and diff seq1 */
   for (i=0;i<part_cnt;i++)
   {
      tstamp = generate_ack_number(to_uin, (unsigned long)seq2, 0);
      v3_send_fragment(pack, to_uin, seq2, seq2, i, part_cnt_r, tstamp, 
 		       pack.nextData, lp_v3_packet_mtu()-18, shm_index);
      pack.nextData +=( lp_v3_packet_mtu()-18); 
      seq2++;
   }
  
   /* send last fragment if necessary */
   if (last_len != 0)
   {
      tstamp = generate_ack_number(to_uin, (unsigned long)seq2, 0);
      v3_send_fragment(pack, to_uin, seq2, seq2, part_cnt, part_cnt_r, 
                       tstamp, pack.nextData, last_len, shm_index);
   }

   /* we should return number of sent packets to caller */   
   if (last_len !=0) 
   {
      return(part_cnt+1);
   }
   else
   {
      return(part_cnt);
   }  
}


/**************************************************************************/
/* This used to add to data fragment header and send result packet 	  */
/**************************************************************************/
void v3_send_fragment(Packet &pack, unsigned long to_uin, unsigned short seq1, 
		      unsigned short seq2, int part_num, int part_cnt, 
		      unsigned long temp_stamp, char *buffer, int len, 
		      unsigned long shm_index)
{
   Packet fragment_pack;
   /* some paranoid sanity checks :) */
   if ((len + 18) > MAX_PACKET_SIZE) return;
   
   /* add header to fragment */
   fragment_pack.clearPacket();
   fragment_pack << (unsigned short)V3_PROTO
                 << (unsigned short)ICQ_CMDxSND_SENDxFRAGMENT
                 << (unsigned short)seq1
                 << (unsigned short)seq2
                 << (unsigned  long)to_uin
                 << (unsigned  long)0x0000
	         << (char)part_num
	         << (char)part_cnt;
   
   /* copy packet fragment to new packet */
   memcpy(fragment_pack.nextData, buffer, len);
   fragment_pack.sizeVal += len;

   DEBUG(100, ("Info: Sending packet fragment to user %lu\n", to_uin));

   /* take ip from source packet */
   fragment_pack.from_ip   = pack.from_ip;
   fragment_pack.from_port = pack.from_port;
   
   /* sending packets to client, we need confirm! */
   event_send_packet(fragment_pack, to_uin, shm_index, v3_retries, v3_timeout, V3_PROTO);
}

