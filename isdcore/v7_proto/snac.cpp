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
/* This module contain functions that create reply packets for AIM proto  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* Parse client SNAC packet (channel #2)				  */
/**************************************************************************/
void process_snac_packet(Packet &pack)
{
   struct snac_header snac;
      
   pack.reset();
   pack.setup_aim();
   pack >> snac.family >> snac.subfamily >> snac.flags >> snac.id;
   
   /* AOL server do this so we should too */
   if ((snac.id & 0x80000000) != 0) 
   {
      DEBUG(10, ("Snac.id underflow (%08X) closing connection\n", (unsigned int)snac.id));
      close_connection(pack);
      return;
   }

   DEBUG(300, ("[%s]: Incoming SNAC(%02X,%02X) with id=%08X & flags=%04X (%d bytes)\n", 
             timestring(False), snac.family, snac.subfamily, (unsigned int)snac.id, 
	     snac.flags, pack.sizeVal));

   /* Check if we have this family and it is enabled */
   if ((snac.family >= MAX_FAMILIES) || (!eservices[snac.family]))
   {
      DEBUG(50, ("ERR: snac fam=%02X, eserv[fam]=%d\n", snac.family, eservices[snac.family]));
      send_snac_error(snac.family, ERR_SERVICE_UNAVAILABLE, snac.id, pack);
      return;
   }

   /* select proper SNAC family handler */
   switch (snac.family)
   {      
      case SN_TYP_GENERIC:	  process_snac_generic(snac, pack); 	 break;
      case SN_TYP_LOCATION:	  process_snac_location(snac, pack); 	 break;
      case SN_TYP_BUDDYLIST:	  process_snac_buddylist(snac, pack); 	 break;
      case SN_TYP_MESSAGING:	  process_snac_messaging(snac, pack); 	 break;
      case SN_TYP_ADVERT:	  process_snac_advert(snac, pack); 	 break;
      case SN_TYP_INVITATION:	  process_snac_invitation(snac, pack); 	 break;
      case SN_TYP_ADMINISTRATIVE: process_snac_admin(snac, pack); 	 break;
      case SN_TYP_POPUPxNOTICES:  process_snac_popup(snac, pack); 	 break;
      case SN_TYP_BOS:		  process_snac_bos(snac, pack); 	 break;
      case SN_TYP_AIMxSEARCH:	  process_snac_search(snac, pack); 	 break;
      case SN_TYP_STATS:	  process_snac_stats(snac, pack); 	 break;
      case SN_TYP_TRANSLATE:	  process_snac_translate(snac, pack); 	 break;
      case SN_TYP_CHATxNAVIG:	  process_snac_chat_navig(snac, pack); 	 break;
      case SN_TYP_CHAT:		  process_snac_chat(snac, pack); 	 break;
      case SN_TYP_SSI:		  process_snac_ssi(snac, pack); 	 break;
      case SN_TYP_ICQxMESSxEXT:	  process_snac_ext_messages(snac, pack); break;
      case SN_TYP_REGISTRATION:	  process_snac_registration(snac, pack); break;
      
      default: DEBUG(10, ("Unknown SNAC family: %02X (typ=%02X,flg=%04X,id=%08X)\n", 
                          snac.family, snac.subfamily, snac.flags, 
			  (unsigned int)snac.id));
			  
               send_snac_error(snac.family, ERR_SERVICE_UNAVAILABLE, snac.id, pack);
               break;
   }   
}


/**************************************************************************/
/* Init SNAC common structures and data arrays				  */
/**************************************************************************/
void init_aim_services()
{
   struct aim_family *bam = aim_root.aim_families;

   /* Services perms structure initialization */
   for (int i=0; i<32; i++) eservices[i] = False;
	
   /* Now we should enable families from config */
   while (bam)
   {
      if (bam->number < MAX_FAMILIES)
      {
         eservices[bam->number] = True;
      }

      bam = bam->next;
   }
}

