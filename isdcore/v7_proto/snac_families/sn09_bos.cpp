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
/* This module contain snac creating/processing functions                 */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* BOS specific services SNAC family packets handler			  */
/**************************************************************************/
void process_snac_bos(struct snac_header &snac, Packet &pack)
{
   switch (snac.subfamily)
   {
      case SN_BOS_RIGHTSxREQUEST:   process_bos_get_rights(snac, pack);       break;
      case SN_BOS_ADDxVISIBLExLIST: process_bos_add_viscontact(snac, pack);   break;
      case SN_BOS_DELxVISIBLExLIST: process_bos_del_viscontact(snac, pack);   break;
      case SN_BOS_ADDxINVISxLIST:   process_bos_add_inviscontact(snac, pack); break;
      case SN_BOS_DELxINVISxLIST:   process_bos_del_inviscontact(snac, pack); break;
      
      default: DEBUG(10, ("Unknown bos SNAC(0x9, %04X)\n", snac.subfamily));
   }

}


/**************************************************************************/
/* Client is acking bos service limits					  */
/**************************************************************************/
void process_bos_get_rights(struct snac_header &snac, Packet &pack)
{
   struct online_user *user;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s:%d sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip), pack.from_port));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   pcopy(arpack, pack);
   arpack.clearPacket();
   arpack.setup_aim();

   arpack << (unsigned short)SN_TYP_BOS
          << (unsigned short)SN_BOS_RIGHTSxRESPONSE
	  << (unsigned short)0x0000
	  << (unsigned  long)snac.id

	  << (unsigned short)0x0002
	  << (unsigned short)sizeof(unsigned short)
	  << (unsigned short)lp_v7_max_invisible_size()

	  << (unsigned short)0x0001
	  << (unsigned short)sizeof(unsigned short)
	  << (unsigned short)lp_v7_max_visible_size();

   arpack.flap_channel = 0x02;
   arpack.sock_evt     = SOCK_DATA;
   tcp_writeback_packet(arpack);
}


/**************************************************************************/
/* Client wants to add new record to visible contact			  */
/**************************************************************************/
void process_bos_add_viscontact(struct snac_header &snac, Packet &pack)
{
   struct online_user *user;
   unsigned short contacts_num = 0;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip)));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   /* get the data from packet */
   while (pack.nextData < (pack.buff + pack.sizeVal))
   {
      contacts[contacts_num] = read_buin(pack);
      contacts_num++;
      
      /* check for single packet contacts limit */
      if (contacts_num > MAX_CONTACTS) break;
   }

   /* insert all extracted contacts into database */
   if (contacts_num > 0)
   {
      db_contact_insert(user->uin, contacts_num, contacts, VISIBLE_CONTACT, lrandom_num());
      DEBUG(10, ("Client have sent %d visible contacts...\n", contacts_num));      
   }
   else
   {
      /* client sent 0 visible contacts - we should delete all existing */
      DEBUG(10, ("Client have sent empty visible contact list...\n"));      
   }
   
   /* Client moved to invisible mode */
   db_contact_delete(user->uin, INVISIBLE_CONTACT);
}


/**************************************************************************/
/* Client wants to del visible from contact				  */
/**************************************************************************/
void process_bos_del_viscontact(struct snac_header &snac, Packet &pack)
{
   struct online_user *user;
   unsigned short contact_num = 0;
   unsigned long uin;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip)));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   /* get the data from packet */
   while (pack.nextData < (pack.buff + pack.sizeVal))
   {
      uin = read_buin(pack);
      db_contact_delete(user->uin, uin, VISIBLE_CONTACT);
      contact_num++;
   }
   
   DEBUG(10, ("Client want to delete %d visible contacts...\n", contact_num));
}



/**************************************************************************/
/* Client wants to add new record to invisible contact			  */
/**************************************************************************/
void process_bos_add_inviscontact(struct snac_header &snac, Packet &pack)
{
   struct online_user *user;
   unsigned short contacts_num = 0;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip)));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   /* get the data from packet */
   while (pack.nextData < (pack.buff + pack.sizeVal))
   {
      contacts[contacts_num] = read_buin(pack);
      contacts_num++;
      
      /* check for single packet contacts limit */
      if (contacts_num > MAX_CONTACTS) break;
   }

   /* insert all extracted contacts into database */
   if (contacts_num > 0)
   {
      db_contact_insert(user->uin, contacts_num, contacts, INVISIBLE_CONTACT, lrandom_num());
      DEBUG(10, ("Client have sent %d invisible contacts...\n", contacts_num));   
   }
   else
   {
      /* client sent 0 invisible contacts - we should delete all existing */
      DEBUG(10, ("Client have sent empty invisible contactlist...\n"));   
   }

   /* Client moved to visible mode */
   db_contact_delete(user->uin, VISIBLE_CONTACT);
}


/**************************************************************************/
/* Client wants to del invisible records from contact-list		  */
/**************************************************************************/
void process_bos_del_inviscontact(struct snac_header &snac, Packet &pack)
{
   struct online_user *user;
   unsigned short contact_num = 0;
   unsigned long uin;
   
   if ((user = shm_get_user(pack.sock_hdl, pack.sock_rnd)) == NULL)
   {
      LOG_SYS(0, ("Client from %s sent request but not authorized...\n", 
                  inet_ntoa(pack.from_ip)));
		  
      send_snac_error(snac.family, ERR_REQUEST_DENIED, snac.id, pack);
      return;
   }

   /* get the data from packet */
   while (pack.nextData < (pack.buff + pack.sizeVal))
   {
      uin = read_buin(pack);
      db_contact_delete(user->uin, uin, INVISIBLE_CONTACT);
      contact_num++;
   }
   
   DEBUG(10, ("Client want to delete %d invisible contacts...\n", contact_num));
}

