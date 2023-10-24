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
/* Handle V5 transport user packets					  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#include "v5_defines.h"

int v5_retries;
int v5_timeout;

/**************************************************************************/
/* Main v5 packet handlers selector 					  */
/* If packet can't be handled, selector write it to alarm log lev=(0)	  */
/**************************************************************************/
void handle_v5_proto(Packet &pack)
{
 unsigned short pvers, pcomm;
 unsigned long  uin_num, ljunk, session_id;
 
 int_pack = pack;
 int_pack.reset();
 
 /* we need some data from packet :) */
 int_pack  >> pvers   >> ljunk 
	   >> uin_num >> session_id 
	   >> pcomm;

 /* Well now we need to determine what we got... */
 switch (pcomm)
 {
    case ICQ_CMDxRCV_LOGON:  		v5_process_login(); 	     break;
    case ICQ_CMDxRCV_ACK:		v5_process_ack();            break;
    case ICQ_CMDxRCV_USERxLIST:		v5_process_contact();        break;
    case ICQ_CMDxRCV_LOGOFF:		v5_process_logoff();         break;
    case ICQ_CMDxRCV_PING:		v5_process_ping();           break;
    case ICQ_CMDxRCV_PING2:		v5_process_ping();           break;
    case ICQ_CMDxRCV_METAxUSER:		v5_process_user_meta();	     break;
    case ICQ_CMDxRCV_SETxSTATUS:	v5_process_status();         break;
    case ICQ_CMDxRCV_OLDxSEARCH:	v5_process_old_search();     break;
    case ICQ_CMDxRCV_OLDxSEARCHxUIN:    v5_process_old_srchuin();    break;
    case ICQ_CMDxRCV_USERxADD:		v5_process_useradd();	     break;
    case ICQ_CMDxRCV_FIRSTxLOGIN:	v5_process_firstlog();	     break;
    case ICQ_CMDxRCV_GETxDEPS:		v5_process_getdeps();	     break;
    case ICQ_CMDxRCV_VISIBLExLIST: 	v5_process_visible_list();   break;
    case ICQ_CMDxRCV_INVISIBLExLIST:    v5_process_invisible_list(); break;
    case ICQ_CMDxRCV_CHANGExVILISTS:	v5_process_change_vilists(); break;
    case ICQ_CMDxRCV_THRUxSERVER:	v5_process_sysmsg();	     break;
    case ICQ_CMDxRCV_THRUxSERVER2:	v5_process_sysmsg();	     break;
    case ICQ_CMDxRCV_SYSxMSGxDONExACK:  v5_process_sysmsg_delete();  break;
    case ICQ_CMDxRCV_USER_INFO_OLD:     v5_process_old_info();       break;
    case ICQ_CMDxRCV_USER_INFO_OLD_EXT: v5_process_old_info_ext();   break;
    case ICQ_CMDxSND_ACKxNEWxUIN:	v5_process_ack_new_uin();    break;
    case ICQ_CMDxRCV_SYSxMSGxREQUEST:   /* depricated, no action */  break;

    default:
	 DEBUG(10, ("We have unknown packet....\n"));
         log_alarm_packet(10, int_pack);  /* dump unknown packet */
	 v5_send_ack(int_pack); break;	 /* and ack it 	 */
 }
}


/**************************************************************************/
/* Disconnect user function				    		  */
/**************************************************************************/
void v5_disconnect_user(struct online_user *user)
{
   move_user_offline(user->uin);
}


/**************************************************************************/
/* Get old-style info request        				    	  */
/**************************************************************************/
void v5_process_old_info()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long  uin_num, temp_stamp, session_id, target_uin;
   struct online_user user;
   struct full_user_info tuser;

   int_pack.reset();
   int_pack >> pvers   >> temp_stamp
 	    >> uin_num >> session_id
 	    >> pcomm   >> seq1
            >> seq2    >> temp_stamp
	    >> target_uin;

   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
 	  (user.session_id == session_id))
      {
    
         v5_send_ack(int_pack);   
         db_online_sseq_open(user);
   
         if (db_users_lookup(target_uin, tuser) >= 0)
         {
            v5_send_old_style_info(user, tuser);
         }
         else
         {
            v5_send_invalid_uin(user, target_uin);
         }
       
         db_online_sseq_close(user,1);
      }
   }
}


/**************************************************************************/
/* Get old-style extended info request 				    	  */
/**************************************************************************/
void v5_process_old_info_ext()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long  uin_num, temp_stamp, session_id, target_uin;
   struct online_user user;
   struct full_user_info tuser;
   struct notes_user_info notes;

   int_pack.reset();
   int_pack >> pvers   >> temp_stamp
 	    >> uin_num >> session_id
 	    >> pcomm   >> seq1
            >> seq2    >> temp_stamp
	    >> target_uin;

   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
 	  (user.session_id == session_id))
      {
    
         v5_send_ack(int_pack);   
         db_online_sseq_open(user);
   
         if (db_users_lookup(target_uin, tuser) >= 0)
         {
	    db_users_notes(target_uin, notes);
            v5_send_old_style_info_ext(user, tuser, notes);
         }
         else
         {
            v5_send_invalid_uin(user, target_uin);
         }
       
         db_online_sseq_close(user,1);
      }
   }
}


/**************************************************************************/
/* Get depslist pseudo login packet				    	  */
/**************************************************************************/
void v5_process_getdeps()
{
  unsigned short pvers, pcomm, seq1, seq2, i, plen;
  unsigned long uin_num, clivers, temp_stamp, session_id;
  struct online_user nuser, check_user;
  struct login_user_info userinfo;
  char usrpass[20];
  
  int_pack.reset();
  int_pack  >> pvers   >> temp_stamp
	    >> uin_num >> session_id
	    >> pcomm   >> seq1
            >> seq2    >> temp_stamp
	    >> uin_num;
	   
  int_pack  >> plen; if (plen > 20) plen = 20; 
  for (i=0; i<plen; i++) int_pack >> usrpass[i];
  
  int_pack >> clivers;

  if (db_users_lookup(uin_num, userinfo) < 0)
  {
     LOG_USR(0, ("Can't find user %lu in database.\n", uin_num));
     v5_send_pass_err(int_pack);
     return;
  }
  
  if (!strcsequal(usrpass, userinfo.passwd))
  {
     LOG_USR(0, ("User %lu pseudo login with wrong password.\n", uin_num));
     v5_send_pass_err(int_pack);
     return;
  }

  if (db_online_lookup(uin_num, check_user) != 0)
  {
     nuser.uin      = uin_num;
     nuser.ip       = int_pack.from_ip;
     nuser.udp_port = int_pack.from_port;
     nuser.ttl	    = get_ping_time(pvers);
     nuser.ttlv	    = get_ping_time(pvers);
     nuser.protocol = pvers;
     nuser.servseq  = 1;
     
     v5_send_ack(int_pack);

     LOG_SYS(0, ("Pseudo login packet from %s:%ld (as %lu with vers: %lu)\n",
                inet_ntoa(nuser.ip), nuser.udp_port, nuser.uin, clivers));
		
     v5_send_depslist(seq2, nuser);

  }
  else
  {
     v5_send_busy(int_pack);
     LOG_USR(0, ("User %lu from %s:%d request login but already online.\n", 
                 uin_num, inet_ntoa(int_pack.from_ip), int_pack.from_port));
  } 
}


/**************************************************************************/
/* User add packet hanlder						  */
/**************************************************************************/
void v5_process_firstlog()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long uin_num, temp_stamp, session_id, session_id2;
  
   int_pack.reset();
   int_pack >> pvers   >> temp_stamp
	    >> uin_num >> session_id
	    >> pcomm   >> seq1
            >> seq2    >> temp_stamp
	    >> session_id2;
  

   LOG_USR(10, ("Got first login from [%s:%d]\n",  
               inet_ntoa(int_pack.from_ip), int_pack.from_port));

   /* may be i should do more here, but i don't know what */ 
   reply_pack.clearPacket();
   
   reply_pack  << (unsigned short)V5_PROTO
               << (char)0x00
               << (unsigned  long)session_id
  	       << (unsigned short)ICQ_CMDxSND_ACK
               << (unsigned short)seq1
               << (unsigned short)seq2
               << (unsigned  long)uin_num
  	       << (unsigned  long)0x0000
	       << (char)0x0a
	       << (unsigned  long)session_id2
	       << (unsigned short)0x0001;
  	    
   PutKey(reply_pack, calculate_checkcode(reply_pack));
    
   /* we do not need to encrypt packet */
   reply_pack.from_ip   = int_pack.from_ip;
   reply_pack.from_port = int_pack.from_port;

   /* send packets to client */
   udp_send_direct_packet(reply_pack);

}


/**************************************************************************/
/* User add packet hanlder						  */
/**************************************************************************/
void v5_process_useradd()
{
  unsigned short pvers, pcomm, seq1, seq2;
  unsigned long  uin_num, temp_stamp, session_id;
  unsigned long contact;
  struct online_user user, auser;

  int_pack.reset();
  int_pack  >> pvers   >> temp_stamp
	    >> uin_num >> session_id
	    >> pcomm   >> seq1
            >> seq2    >> temp_stamp
	    >> contact;

  if (db_online_lookup(uin_num, user) == 0)
  {
    if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
	(user.session_id == session_id))
    {
    
      v5_send_ack(int_pack);
      
      DEBUG(100, ("Process user add-to-contact command from %s:%d (%lu)\n", 
      inet_ntoa(int_pack.from_ip), int_pack.from_port, user.uin));

      if ((db_add_online_lookup(uin_num, contact, auser) == 0) &&
          (auser.active == 1))
      {
        v5_send_user_online(user, auser);
      }
       
      db_contact_insert(uin_num, 1, &contact, NORMAL_CONTACT, lrandom_num());

    }
    else
    {

       LOG_ALARM(0, ("Spoofed (%lu) add to contact command from %s:%d\n", uin_num,
		      inet_ntoa(int_pack.from_ip), int_pack.from_port));
       v5_send_not_connected(int_pack);
    }
  }
  else
  {
     v5_send_not_connected(int_pack);
  }
}


/**************************************************************************/
/* Invisible and visible user change packet 				  */
/**************************************************************************/
void v5_process_change_vilists()
{
  unsigned short pvers, pcomm, seq1, seq2;
  unsigned long  uin_num, temp_stamp, session_id, uin_change;
  struct online_user user, to_user;
  char ltype, action_type;

  int_pack.reset();
  int_pack  >> pvers   >> temp_stamp
	    >> uin_num >> session_id
	    >> pcomm   >> seq1
            >> seq2    >> temp_stamp
	    >> uin_change
	    >> ltype
	    >> action_type;

  if (db_online_lookup(uin_num, user) == 0)
  {
     if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
 	 (user.session_id == session_id))
    {
    
      v5_send_ack(int_pack);
      
      DEBUG(100, ("Process user visible/invisible contactlists change from %s:%d (%lu)\n", 
      inet_ntoa(int_pack.from_ip), int_pack.from_port, user.uin));
	
      if (ltype == 2)
      {
        if (action_type == 1)
	{
	   db_contact_insert(uin_num, 1, &uin_change, VISIBLE_CONTACT, lrandom_num());
	   
	   if (need_notification(uin_num, uin_change, to_user) == 0)
	   {
    	      send_user_online(to_user, user);
	   }
	}
	else
	{
	   db_contact_delete(uin_num, uin_change, VISIBLE_CONTACT);
	   
	   if (need_notification(uin_num, uin_change, to_user) == 0)
	   {
    	      send_user_offline(to_user, user.uin);
	   }
	}
      }

      if (ltype == 1)
      {
        if (action_type == 1)
	{
	   db_contact_insert(uin_num, 1, &uin_change, INVISIBLE_CONTACT, lrandom_num());
	   
	   if (need_notification(uin_num, uin_change, to_user) == 0)
	   {
    	      send_user_offline(to_user, user.uin);
	   }
	}
	else
	{
	   db_contact_delete(uin_num, uin_change, INVISIBLE_CONTACT);	
	   
	   if (need_notification(uin_num, uin_change, to_user) == 0)
	   {
    	      send_user_online(to_user, user);
	   }
	}
      }
    }
    else
    {
       LOG_ALARM(0, ("Spoofed (%lu) invisible/visible change command from %s:%d\n", uin_num,
		      inet_ntoa(int_pack.from_ip), int_pack.from_port));

       v5_send_not_connected(int_pack);
    }
  }
  else
  {
    v5_send_not_connected(int_pack);
  }
}


/**************************************************************************/
/* Status change hanlder						  */
/**************************************************************************/
void v5_process_status()
{
  unsigned short pvers, pcomm, seq1, seq2;
  unsigned long uin_num, temp_stamp, session_id;
  unsigned long new_estatus, new_status, old_status;
  struct online_user user;

  int_pack.reset();
  int_pack  >> pvers   >> temp_stamp
	    >> uin_num >> session_id
	    >> pcomm   >> seq1
            >> seq2    >> temp_stamp
	    >> new_status
	    >> new_estatus;

  if (db_online_lookup(uin_num, user) == 0)
  {
    if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
	(user.session_id == session_id))
    {
      v5_send_ack(int_pack);
      
      send_event2ap(papack, ACT_STATUS, user.uin, user.status, 
                    ipToIcq(user.ip), new_status, longToTime(time(NULL)), "");                     
      
      DEBUG(10, ("Process user status change command from %s:%d (%lu)\n", 
      inet_ntoa(int_pack.from_ip), int_pack.from_port, user.uin));

      old_status = user.status;
      user.status = new_status;
      user.estat = new_estatus;
      
      if (user.status != 0) user.uclass = user.uclass | CLASS_AWAY;
      if (user.status == 0) user.uclass = user.uclass & (~CLASS_AWAY);

      db_online_setstatus(user);
      broadcast_status( user , old_status);
      
    }
    else
    {

       LOG_ALARM(0, ("Spoofed (%lu) status change message from %s:%d\n", uin_num,
   		     inet_ntoa(int_pack.from_ip), int_pack.from_port));
       v5_send_not_connected(int_pack);
    }
  }
  else
  {

    v5_send_not_connected(int_pack);
    
  }
}


/**************************************************************************/
/* Ping packet hanlder							  */
/**************************************************************************/
void v5_process_ping()
{
  unsigned short pvers, pcomm, seq1, seq2;
  unsigned long  uin_num, temp_stamp, session_id;
  struct online_user user;

  int_pack.reset();
  int_pack >> pvers   >> temp_stamp
	   >> uin_num >> session_id
	   >> pcomm   >> seq1
           >> seq2    >> temp_stamp;

  if (db_online_lookup(uin_num, user) == 0)
  {
    if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
	(user.session_id == session_id))
    {
    
      v5_send_ack(int_pack);
      
      DEBUG(350, ("Process user ping command from %s:%d (%lu)\n", 
      inet_ntoa(int_pack.from_ip), int_pack.from_port, user.uin));

      db_online_touch( user );
      send_update2cache( user.uin, user.ttl );

    }
    else
    {

       LOG_ALARM(0, ("Spoofed (%lu) ping message from %s:%d\n", uin_num,
		      inet_ntoa(int_pack.from_ip), int_pack.from_port));

       v5_send_not_connected(int_pack);
    }
  }
  else
  {
  
    v5_send_not_connected(int_pack);

  }
}


/**************************************************************************/
/* Old search packet hanlder						  */
/**************************************************************************/
void v5_process_old_search()
{
  unsigned short pvers, pcomm, seq1, seq2;
  unsigned long  uin_num, temp_stamp, session_id;
  struct online_user user;

  int_pack.reset();
  int_pack  >> pvers   >> temp_stamp
	    >> uin_num >> session_id
	    >> pcomm   >> seq1
            >> seq2    >> temp_stamp;

  if (db_online_lookup(uin_num, user) == 0)
  {
    if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
	(user.session_id == session_id))
    {
    
      v5_send_ack(int_pack);
      
      DEBUG(100, ("Process user old search command from %s:%d (%lu)\n", 
      inet_ntoa(int_pack.from_ip), int_pack.from_port, user.uin));

      v5_old_search(int_pack, user, seq2);

    }
    else
    {

       LOG_ALARM(0, ("Spoofed (%lu) old search message from %s:%d\n", uin_num,
		      inet_ntoa(int_pack.from_ip), int_pack.from_port));
       v5_send_not_connected(int_pack);
    }
  }
  else
  {
  
    v5_send_not_connected(int_pack);

  }
}


/**************************************************************************/
/* Old search packet hanlder (search by uin)				  */
/**************************************************************************/
void v5_process_old_srchuin()
{
  unsigned short pvers, pcomm, seq1, seq2;
  unsigned long  uin_num, temp_stamp, session_id, tuin;
  struct online_user user;

  int_pack.reset();
  int_pack  >> pvers   >> temp_stamp
	    >> uin_num >> session_id
	    >> pcomm   >> seq1
            >> seq2    >> temp_stamp
	    >> tuin;

  if (db_online_lookup(uin_num, user) == 0)
  {
    if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
	(user.session_id == session_id))
    {
    
      v5_send_ack(int_pack);
      
      DEBUG(100, ("Process user old search (by uin) command from %s:%d (%lu)\n", 
      inet_ntoa(int_pack.from_ip), int_pack.from_port, user.uin));

      v5_old_search_uin(tuin, user, seq2);

    } else {

       LOG_ALARM(0, ("Spoofed (%lu) old search by uin message from %s:%d\n", uin_num,
		      inet_ntoa(int_pack.from_ip), int_pack.from_port));
       v5_send_not_connected(int_pack);
    }
  }
  else
  {
  
     v5_send_not_connected(int_pack);

  }
}


/**************************************************************************/
/* Login packet handler							  */
/**************************************************************************/
void v5_process_login()
{
   unsigned short vers, comm, seq1, seq2, i, tcpversion, plen;
   unsigned long uin_num, ljunk, tport, session_id;
   unsigned long futures, int_ip, web_port;
  
   unsigned short status, estatus;
   unsigned short usjunk;
   char dc_type;
   int lookup_res;
   struct online_user nuser, *user;
   struct login_user_info userinfo;
   char usrpass[20];
   fstring lock_text;
   v5_send_ack(int_pack);
  
   int_pack.reset();
   int_pack  >> vers    >> ljunk
 	     >> uin_num >> session_id
	     >> comm    >> seq1
             >> seq2    >> ljunk
	     >> ljunk   >> tport;
	   
   int_pack  >> plen; if (plen > 20) plen = 20; 
   for (i=0; i<plen; i++) int_pack >> usrpass[i];

   int_pack  >> ljunk   >> int_ip
             >> dc_type >> status 
	     >> estatus >> tcpversion
	     >> usjunk  >> ljunk
	     >> ljunk   >> web_port
	     >> futures;
	   
   if (!lp_v5_enabled()) 
   {
      LOG_USR(0, ("User %lu try login via V5 proto (V5 proto disabled)\n", uin_num));
      v5_send_login_err(int_pack, "V5 protocol disabled by administrator...\x0d\x0aTry to use another client version...");
      return;
   }
   
   lookup_res = db_users_lookup(uin_num, userinfo);	   

   if (lookup_res == -2)
   {
     LOG_ALARM(0, ("SQL error in database - we shouldn't accept login\n")); 
     return;
   }
  
   if (lookup_res == -1)
   {
     LOG_USR(0, ("Can't find user %lu in database.\n", uin_num));
     v5_send_pass_err(int_pack);
     return;
   }
  
   if (!strcsequal(usrpass, userinfo.passwd))
   {
     LOG_USR(0, ("User %lu trying login with wrong password.\n", uin_num));
     v5_send_pass_err(int_pack);
     return;
   }

   if ((userinfo.ip_addr != 0) &&
       (lp_restrict2luip()) &&
       (userinfo.ip_addr != ipToIcq(int_pack.from_ip)))
   {
      
      LOG_USR(0, ("User %lu (v5) trying login from wrong ip-addres.\n", uin_num));
      v5_send_login_err(int_pack, "Account locked...");
      return;
   }

   if (userinfo.disabled == 1)
   {
     LOG_USR(0, ("User %lu trying login but his account locked.\n", uin_num));
     if (db_users_lock_message(uin_num, lock_text))
     {  
        ITrans.translateToClient(lock_text);
        v5_send_login_err(int_pack, lock_text);
     } else v5_send_login_err(int_pack, "Account locked...");
     
     return;
   }    
  
   bzero((void *)&nuser, sizeof(nuser));

   /* well, now we should check if there is another user connected... */
   if ((user = shm_get_user(uin_num)) != NULL)
   {
      v5_send_busy(int_pack);
      LOG_USR(0, ("User %lu from %s:%d request login but already online.\n", 
                   uin_num, inet_ntoa(int_pack.from_ip), int_pack.from_port)); 
      return;
   }
  
   /* fill new record */
   nuser.uin         = uin_num;
   nuser.usid        = rand();
   nuser.ip          = int_pack.from_ip;
   nuser.tcp_port    = tport;
   nuser.udp_port    = int_pack.from_port;
   nuser.status      = status;
   nuser.estat       = estatus;
   nuser.uptime      = longToTime(time(NULL));
   nuser.lutime      = longToTime(time(NULL));
   nuser.ttl	     = get_ping_time(vers);
   nuser.ttlv	     = get_ping_time(vers);
   nuser.protocol    = vers;
   nuser.servseq     = 0;
   nuser.session_id  = session_id;	 /* udp connection cookie    */
   nuser.tcpver      = tcpversion;       /* version of tcp protocol  */
   nuser.active      = 1;		 /* user already online      */
   nuser.dc_type     = dc_type;	         /* direct connection type   */
   nuser.int_ip      = icqToIp(int_ip);  /* internal ip (if exist)   */
   nuser.uclass      = CLASS_FREE | CLASS_ICQ;
   nuser.web_port    = web_port;         /* user web front port      */
   nuser.cli_futures = futures;          /* client futures           */
   
   if (nuser.status != 0) nuser.uclass = nuser.uclass | CLASS_AWAY;
   
   nuser.mopt[MCH1].max_msglen   = lp_v5_max_msgsize();
   nuser.mopt[MCH1].max_sevil    = 999;
   nuser.mopt[MCH1].max_revil    = 999;
   nuser.mopt[MCH1].icbm_flags   = ICBM_FLG_BASE;

   nuser.mopt[MCH4].max_msglen   = lp_v5_max_msgsize()+12;
   nuser.mopt[MCH4].max_sevil    = 999;
   nuser.mopt[MCH4].max_revil    = 999;
   nuser.mopt[MCH4].icbm_flags   = ICBM_FLG_BASE;

   if (db_online_insert(nuser) != 0)
   {
      v5_send_login_err(int_pack, "Server users table full...Try again later");
      return;
   }
  
   v5_send_login_reply(int_pack, userinfo, nuser);

   move_user_online(nuser);
   LOG_USR(0, ("User %lu from %s:%ld moved online.\n", nuser.uin, 
                inet_ntoa(nuser.ip), nuser.udp_port));

}


/**************************************************************************/
/* Contact list hanlder							  */
/**************************************************************************/
void v5_process_contact()
{
 unsigned short pvers, pcomm, seq1, seq2, i;
 unsigned long  uin_num, temp_stamp, session_id, rid;
 int count; char ccount;
 struct online_user user, auser, *puser;
 PGresult *res;
 cstring dbcomm_str;
 int on_cnt1, on_cnt2, on_cnt3;

 int_pack.reset();
 int_pack  >> pvers   >> temp_stamp
	   >> uin_num >> session_id
	   >> pcomm   >> seq1
           >> seq2    >> temp_stamp
	   >> ccount;
 
 if (db_online_lookup(uin_num, user) == 0)
 {
    if (ipToIcq(user.ip) == ipToIcq(int_pack.from_ip))
    {
       if (ccount > 100)
       {
       	  move_user_offline(user.uin);
          v5_send_not_connected(int_pack);
	  return;
       }
       
       count = (int)ccount;
       rid   = lrandom_num();

       for (i=0;i<count;i++) int_pack >> contacts[i];

       v5_send_ack(int_pack);
       
       /* first of all we need ready contact-list in database */
       db_contact_insert(uin_num, count, contacts, NORMAL_CONTACT, rid);

#define BCAST_V5M795 \
"SELECT uin,ishm FROM online_users \
 JOIN \
 ( \
    SELECT tuin FROM online_contacts \
    WHERE (ouin=%lu) AND (type=%d) AND (rid=%lu) \
    EXCEPT \
    ( \
       SELECT ouin FROM online_contacts \
       WHERE tuin=%lu AND type=%d \
    ) \
    LIMIT %d \
 )\
 AS TMP on TMP.tuin=uin \
 WHERE (active=1) AND (stat<>%d)"

       /* now we'll try to get online users from our contact */
       slprintf(dbcomm_str, sizeof(dbcomm_str)-1, 
	        BCAST_V5M795, uin_num, NORMAL_CONTACT, rid, uin_num, 
	        INVISIBLE_CONTACT, lp_v7_max_contact_size(), ICQ_STATUS_PRIVATE);

       res = PQexec(users_dbconn, dbcomm_str);
       if (PQresultStatus(res) != PGRES_TUPLES_OK)
       {
          handle_database_error(res, "[V5 CONTACT LOOKUP]");
	  v5_send_not_connected(int_pack);
          return;
       }

       on_cnt1 = PQntuples(res);
       
       for (i=0;i<on_cnt1;i++) 
       {
          /* fill auser structure uin & ishm fields */
  	  auser.uin	  = atoul(PQgetvalue(res, i, 0));
  	  auser.shm_index = atoul(PQgetvalue(res, i, 1));
	
	  /* find online user in shm */
	  puser = shm_iget_user(auser.uin, auser.shm_index);
	  if (puser == NULL) continue;

          /* copy found user to auser structure */
	  memcpy((void *)&auser, (const void *)puser, 
	          sizeof(struct online_user));

  	  /* send packet to user */
          v5_send_user_online(user, auser);
       }
       
       PQclear(res);

/* Query define */
#define BCAST_V5M846 \
"SELECT uin,ishm FROM online_users \
 JOIN \
 ( \
    SELECT tuin FROM online_contacts \
    WHERE (ouin=%lu) AND (type=%d) AND (rid=%lu) \
    INTERSECT \
    ( \
       SELECT ouin FROM online_contacts \
       WHERE tuin=%lu AND type=%d \
    ) \
    LIMIT %d \
 ) \
 AS TMP on TMP.tuin=uin \
 WHERE (active=1) AND (stat=%d)"
    
       /* now we have to lookup for invisible users - run query */
       slprintf(dbcomm_str, sizeof(dbcomm_str)-1, 
                BCAST_V5M846, uin_num, NORMAL_CONTACT, rid, uin_num, 
                VISIBLE_CONTACT, lp_v7_max_contact_size(), ICQ_STATUS_PRIVATE);

       res = PQexec(users_dbconn, dbcomm_str);
       if (PQresultStatus(res) != PGRES_TUPLES_OK)
       {
          handle_database_error(res, "[V5 CONTACT LOOKUP (invisible)]");
	  v5_send_not_connected(int_pack);
          return;
       }

       on_cnt2 = PQntuples(res);
       
       for (i=0;i<on_cnt2;i++) 
       {
          /* fill auser structure uin & ishm fields */
  	  auser.uin	  = atoul(PQgetvalue(res, i, 0));
  	  auser.shm_index = atoul(PQgetvalue(res, i, 1));
	
	  /* find online user in shm */
	  puser = shm_iget_user(auser.uin, auser.shm_index);
	  if (puser == NULL) continue;

          /* copy found user to auser structure */
	  memcpy((void *)&auser, (const void *)puser, 
	          sizeof(struct online_user));

 	  /* send packet to user */
          v5_send_user_online(user, auser);
       }
       
       PQclear(res);
    
       /* This mean that client has more contacts    */
       /* if client has 100 contacts it sends 99 + 1 */
       if (count == 100 )
       {
          v5_send_end_contact(user);
       }
       else
       {
          v5_send_end_contact(user);
	  on_cnt3 = v5_send_offline_messages(user);
       }
       
    }
    else
    {
    
       LOG_ALARM(0, ("Spoofed contact list from %s:%d (%lu)\n", 
       inet_ntoa(int_pack.from_ip), int_pack.from_port, uin_num));
    }   
  }
  else
  {  
     v5_send_not_connected(int_pack);
  }
}


/**************************************************************************/
/* Visible contact list hanlder						  */
/**************************************************************************/
void v5_process_visible_list()
{
   unsigned short pvers, pcomm, seq1, seq2, i;
   unsigned long  uin_num, temp_stamp, session_id;
   int count; char ccount;
   struct online_user user;

   int_pack.reset();
   int_pack  >> pvers   >> temp_stamp
  	     >> uin_num >> session_id
  	     >> pcomm   >> seq1
             >> seq2    >> temp_stamp
  	     >> ccount;
 
   count = (int)ccount;
  
   for (i=0;i<count;i++) int_pack >> contacts[i];

   if (db_online_lookup(uin_num, user) == 0)
   {
      if (ipToIcq(user.ip) == ipToIcq(int_pack.from_ip))
      {
         v5_send_ack(int_pack);
         
         /* we should only add this list to database */
         db_contact_insert(uin_num, count, contacts, VISIBLE_CONTACT, lrandom_num());
               
      }
      else
      {
      
         LOG_ALARM(10, ("Spoofed contact list from %s:%d (%lu)\n", 
                   inet_ntoa(int_pack.from_ip), int_pack.from_port, uin_num));
		   
         v5_send_not_connected(int_pack);
      }   
   }
   else
   {
      v5_send_not_connected(int_pack);
   }
}


/**************************************************************************/
/* Invisible contact list hanlder					  */
/**************************************************************************/
void v5_process_invisible_list()
{
   unsigned short pvers, pcomm, seq1, seq2, i;
   unsigned long  uin_num, temp_stamp, session_id;
   int count; char ccount;
   struct online_user user;

   int_pack.reset();
   int_pack  >> pvers   >> temp_stamp
  	     >> uin_num >> session_id
  	     >> pcomm   >> seq1
             >> seq2    >> temp_stamp
  	     >> ccount;
   
   count = (int)ccount;
   
   for (i=0;i<count;i++) int_pack >> contacts[i];

   if (db_online_lookup(uin_num, user) == 0)
   {
      if (ipToIcq(user.ip) == ipToIcq(int_pack.from_ip))
      {
         v5_send_ack(int_pack);
         
         /* we should only add this list to database */
         db_contact_insert(uin_num, count, contacts, INVISIBLE_CONTACT, lrandom_num());
         
      }
      else
      {
         LOG_ALARM(0, ("Spoofed contact list from %s:%d (%lu)\n", 
                   inet_ntoa(int_pack.from_ip), int_pack.from_port, uin_num));
         v5_send_not_connected(int_pack);
      }   
   }
   else
   {
    
     v5_send_not_connected(int_pack);

   }
}


/**************************************************************************/
/* Used to send packets, that should be confirmed with ack 		  */
/**************************************************************************/
void v5_send_indirect(Packet &pack, unsigned long to_uin, unsigned long shm_index)
{
   /* now we send proto version instead of timestamp as last param */
   event_send_packet(pack, to_uin, shm_index, v5_retries, v5_timeout, V5_PROTO);
}


/**************************************************************************/
/* Used to init all proto variables and data structures 		  */
/**************************************************************************/
void v5_proto_init()
{

   v5_timeout = lp_v5_timeout();
   v5_retries = lp_v5_retries();
  
}


/**************************************************************************/
/* ack packet handler							  */
/**************************************************************************/
void v5_process_ack()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long uin_num, temp_stamp, session_id;

   int_pack.reset();
   int_pack  >> pvers   >> temp_stamp
 	     >> uin_num >> session_id
	     >> pcomm   >> seq1
             >> seq2    >> temp_stamp;
    
   temp_stamp = generate_ack_number(uin_num, seq1, seq2);
  
   DEBUG(150, ("GOT ACK: ver:%d, comm:%d, seq1:%d, seq2:%d, uin:%lu, stamp:%lu\n",
	 pvers, pcomm, seq1, seq2, uin_num, temp_stamp));
 
   process_ack_event(uin_num, temp_stamp);
}


/**************************************************************************/
/* "Text message" packet handler					  */
/**************************************************************************/
void v5_process_logoff()
{
  unsigned short pvers, pcomm, seq1, seq2, i, plen;
  unsigned long  uin_num, temp_stamp, session_id;
  char bmessage[20];

 int_pack.reset();
 int_pack  >> pvers   >> temp_stamp
	   >> uin_num >> session_id
	   >> pcomm   >> seq1
           >> seq2    >> temp_stamp;

 int_pack >> plen; if (plen > 20 ) plen = 20;
 for (i=0; i<plen; i++) int_pack >> bmessage[i];

 DEBUG(100, ("Bmessage: %s\n", bmessage));
 
 if (strequal(bmessage,"B_USER_DISCONNECTED"))
 {
    struct online_user temp_user;
    unsigned long uptime;
    
    if (db_online_lookup(uin_num, temp_user) == 0)
    {
    
      if ((ipToIcq(temp_user.ip) == ipToIcq(int_pack.from_ip)) &&
          (temp_user.session_id == session_id))
      { 
         move_user_offline(uin_num);
	 uptime = (unsigned long)(time(NULL) - longToTime(temp_user.uptime));
	 
	 LOG_USR(0, ("User %lu moved offline. Online time was: %lu seconds.....\n", uin_num, uptime));
      }
      else 
      { 
        /* All mirabilis udp clients have bad habbit to send this packet */
	/* on disconnect even they are not connected */
	
	/*  LOG_ALARM(0, ("Someone from %s:%d trying to disconnect user %d\n", 
                    inet_ntoa(int_pack.from_ip), int_pack.from_port, uin_num)); */

        v5_send_not_connected(int_pack);
      }
    }
    else 
    {
    
      v5_send_not_connected(int_pack);
    }
  }
}


/**************************************************************************/
/* This func will extract (carefully) text string from packet 		  */
/**************************************************************************/
BOOL v5_extract_string(char *dst_str, Packet &spack, int max_len, 
		       char *fname, struct online_user &user)
{
   unsigned short str_len, i;
   char description[25];
   
   spack >> str_len;
   snprintf(description, 24, "V5 %s string", fname);
   if (!islen_valid(str_len, max_len, description, user))
   {
      move_user_offline(user.uin);
      v5_send_not_connected(spack);
      return(False);
	 
   }
   else
   {
      for(i=0; i<str_len; i++) spack >> dst_str[i];
   }
   
   return(True);
}

