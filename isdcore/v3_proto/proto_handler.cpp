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
/* Main handler for V3 protocol				  		  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#include "v3_defines.h"

int v3_timeout; int v3_retries;

/**************************************************************************/
/* Main v3 packet handlers selector 					  */
/* If packet can't be handled, selector write it to alarm log lev=(200)	  */
/**************************************************************************/
void handle_v3_proto(Packet &pack)
{
   unsigned short pvers, pcomm;
   int_pack = pack;
  
   int_pack.reset(); int_pack >> pvers >> pcomm;

   /* Well now we need to determine what we got... */
   switch (pcomm)
   {
      case ICQ_CMDxRCV_LOGON:		  v3_process_login();       break;
      case ICQ_CMDxRCV_ACK:		  v3_process_ack();         break;
      case ICQ_CMDxRCV_PING:		  v3_process_ping();        break;
      case ICQ_CMDxRCV_PING2:		  v3_process_ping();	    break;
      case ICQ_CMDxRCV_LOGOFF:		  v3_process_logoff();      break;
      case ICQ_CMDxRCV_USERxADD:	  v3_process_useradd();     break;
      case ICQ_CMDxRCV_SETxSTATUS:	  v3_process_status();      break;
      case ICQ_CMDxRCV_USERxLIST:	  v3_process_contact();     break;
      case ICQ_CMDxRCV_GETxNOTES:	  v3_process_notes();       break;
      case ICQ_CMDxRCV_USERxGETINFO:	  v3_process_getinfo();     break;
      case ICQ_CMDxRCV_USERxGETINFO1:	  v3_process_getinfo1();    break;
      case ICQ_CMDxRCV_GETxDEPS:	  v3_process_getdeps();     break;
      case ICQ_CMDxRCV_GETxEXTERNALS:	  v3_process_getext();	    break;
      case ICQ_CMDxRCV_FIRSTxLOGIN:	  v3_process_firstlog();    break;
      case ICQ_CMDxRCV_GETxDEPS1: 	  v3_process_getdeps1();    break;
      case ICQ_CMDxRCV_RECONNECT: 	  v3_process_onlineinfo();  break;
      case ICQ_CMDxRCV_USAGESTATS:	  v3_process_usagestats();  break;    
      case ICQ_CMDxRCV_SETxNOTES:	  v3_process_setnotes();    break;
      case ICQ_CMDxRCV_SETxBASIC_INFO:	  v3_process_setbasic();    break;
      case ICQ_CMDxRCV_SETxHOME_INFO:	  v3_process_sethome();	    break;
      case ICQ_CMDxRCV_SETxHOME_PAGE:	  v3_process_setweb(HOME);  break;
      case ICQ_CMDxRCV_SETxWORK_INFO:	  v3_process_setwork();	    break;
      case ICQ_CMDxRCV_SETxWORK_PAGE:	  v3_process_setweb(WORK);  break;
      case ICQ_CMDxRCV_UKNOWN_DEP:	  v3_process_unknow_dep();  break;
      case ICQ_CMDxRCV_SYSxMSGxREQ:	  v3_process_sysmsg_req();  break;
      case ICQ_CMDxRCV_SET_PASSWORD:	  v3_process_setpass();	    break;
      case ICQ_CMDxRCV_SET_AUTH:	  v3_process_setauth();     break;
      case ICQ_CMDxRCV_SEARCHxSTART:	  v3_process_search();	    break;
      case ICQ_CMDxRCV_THRUxSERVER:	  v3_process_sysmsg();	    break;
      case ICQ_CMDxRCV_AUTHORIZE:	  v3_process_sysmsg();      break;
      case ICQ_CMDxRCV_SYSxMSGxDONExACK:  v3_process_sysack();	    break;
      case ICQ_CMDxRCV_SETxSTATE:	  v3_process_state();	    break;
      case ICQ_CMDxRCV_FRAGMENTED:	  v3_defrag_packet();	    break;
      case ICQ_CMDxRCV_REGxREQUEST_INFO:  v3_process_reginfo_req(); break;
      case ICQ_CMDxRCV_REGxNEWxUSERxINFO: v3_process_reg_newuser(); break;
      case ICQ_CMDxRCV_BROADCAST_MSG_ALL: v3_process_broadcast();   break;
      case ICQ_CMDxRCV_BROADCAST_MSG_ONL: v3_process_broadcast();   break;
      case ICQ_CMDxRCV_WWP_MSG:		  v3_process_wwp();	    break;
      case ICQ_CMDxRCV_DELUSER_REQ:       v3_process_deluser_req(); break;
    
      default: 
	 DEBUG(10, ("We have unknown packet...\n"));
         log_alarm_packet(10, int_pack); /* we should dump unknown packet */
         v3_send_ack(int_pack); break;	 /* and ack it */
 }
}


/**************************************************************************/
/* Disconnect user function				    		  */
/**************************************************************************/
void v3_disconnect_user(struct online_user *user)
{
   move_user_offline(user->uin);
}


/**************************************************************************/
/* Disconnect user request from admin (disconnect utility)    		  */
/**************************************************************************/
void v3_process_deluser_req()
{
   unsigned short pvers, pcomm;
   unsigned long  uin;
   struct online_user *user;

   int_pack.reset();
   int_pack >> pvers >> pcomm >> uin;

   if (int_pack.from_local)
   {
      DEBUG(10, ("Disconnect user (%lu) request now in PP...\n", uin));
      if ((user = shm_get_user(uin)) != NULL) disconnect_user(user);
   }
}


/**************************************************************************/
/* We should validate this mesg and then send it to recipient   	  */
/**************************************************************************/
void v3_process_wwp()
{
   unsigned short pvers, seq1, seq2, str_len, pcomm;
   unsigned  long junk, uin_num, tuin;
   int i;  
   struct online_user user;
  
   char name[32];
   char email[64];
   char wwp_header[128];
   char message[550];
   char full_message[1024];
   struct msg_header msg_hdr;
   
   bzero((void *)&msg_hdr, sizeof(msg_hdr));
  
   /* check if packet was spoofed */
   if (ipToIcq(int_pack.from_ip) != 0) return;
   if (int_pack.from_local != 1) return;

   /* hmm... look at this!!! it is wwp packet, now what we got? */
   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num >> junk >> tuin;
  
   int_pack >> str_len;
   if (str_len > 32)
   {
      DEBUG(10, ("WARNING: WWP sender name overflow (len=%d)...\n", str_len));
      return;
   }
   for(i=0; i<str_len; i++) int_pack >> name[i];

   int_pack >> str_len;
   if (str_len > 64)
   {
      DEBUG(10, ("WARNING: WWP sender email overflow (len=%d)...\n", str_len));
      return;
   }
  
   for(i=0; i<str_len; i++) int_pack >> email[i];

   int_pack >> str_len;
   if (str_len > 550)
   {
      DEBUG(10, ("WARNING: WWP message text overflow...\n"));
      return;
   }
  
   for(i=0; i<str_len; i++) int_pack >> message[i];
  
   /* packet parsed now we should create correct message text 
      with normal delimiters and send it to client (or in db) */
     
   snprintf(wwp_header, 128, "%s%s%s%s%s%s3%s", name, CLUE_CHAR, CLUE_CHAR, 
            CLUE_CHAR, email, CLUE_CHAR, CLUE_CHAR);

   strncpy(full_message, wwp_header, sizeof(full_message)-1);
   strncat(full_message, message, 550);

   user.uin	   = 1;
   user.ip	   = icqToIp(0); 
   user.protocol   = 3;
   user.servseq	   = 0;
   user.servseq2   = 0;
   user.session_id = 0;

   /* fill message header structure */
   msg_hdr.mtype    = WWP_TYPE;
   msg_hdr.touin    = tuin;
   msg_hdr.fromuin  = user.uin;
   msg_hdr.seq2     = seq2;
   msg_hdr.from_ver = V3_PROTO;
   msg_hdr.mtime    = timeToLong(time(NULL));
   msg_hdr.msglen   = strlen(full_message);
     
   process_message(msg_hdr, full_message);
}


/**************************************************************************/
/* We should ack and send register initial info		    		  */
/**************************************************************************/
void v3_process_reginfo_req()
{
   unsigned short seq1, seq2;
   unsigned long temp_stamp;

   int_pack.reset();
   int_pack >> temp_stamp >> seq1 >> seq2;
 
   v3_send_ack(int_pack);
   v3_send_registration_info(int_pack, seq2);
   
   DEBUG(50, ("User ask for registration info,,,\n"));
}


/**************************************************************************/
/* Set user's client state (minimized, maximized)	    		  */
/**************************************************************************/
void v3_process_state()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long  uin_num, temp_stamp;
   struct online_user user;
   char state;
 
   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num;
 
   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
          (user.udp_port == int_pack.from_port))
      {
         v3_send_ack(int_pack);
      
         int_pack.reset(); state = 0;
         int_pack >> pvers >> pcomm >> seq1 >> seq2 
                  >> uin_num >> temp_stamp;
         int_pack >> state;

         if (state > 1) state = -1;
         db_online_setstate(user, state);
      
      }
      else
      {
    
         LOG_ALARM(0, ("Spoofed set client state command from %s:%d (%lu)\n", 
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
/* Set user authorization mode				    		  */
/**************************************************************************/
void v3_process_setauth()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long  uin_num, temp_stamp;
   struct online_user user;
   char auth_mode;

   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num;
 
   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
          (user.udp_port == int_pack.from_port))
      {
         v3_send_ack(int_pack);
      
         int_pack.reset();
         int_pack >> pvers >> pcomm >> seq1 >> seq2 
                  >> uin_num >> temp_stamp;

         auth_mode = 0;
         int_pack >> auth_mode;

         v3_send_reply_ok(int_pack, user, ICQ_CMDxSND_USERxSET_AUTH_OK);
      
         db_users_setauthmode(user.uin, auth_mode);
      
      }
      else
      {
    
         LOG_ALARM(0, ("Spoofed set authmode command from %s:%d (%lu)\n", 
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
/* Process password changing				    		  */
/**************************************************************************/
void v3_process_setpass()
{
   unsigned short pvers, pcomm, seq1, seq2, str_len, i;
   unsigned long  uin_num, temp_stamp;
   struct online_user user;
   char password[32];

   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num;
 
   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
          (user.udp_port == int_pack.from_port))
      {  
         v3_send_ack(int_pack);
    
         int_pack.reset();
         int_pack >> pvers >> pcomm >> seq1 >> seq2 
                  >> uin_num >> temp_stamp >> str_len;

         /* extract new password from packet */
         if (!islen_valid(str_len, 32, "new_pass", user))
         {
  	    move_user_offline(user.uin);
	    v3_send_not_connected(int_pack);
	    return;
	 
         } else { for(i=0; i<str_len; i++) int_pack >> password[i]; }

         v3_send_reply_ok(int_pack, user, ICQ_CMDxSND_USERxSET_PASSWORD_OK);
         db_users_setpassword(user.uin, password);
      
      }
      else
      {
    
         LOG_ALARM(0, ("Spoofed set password command from %s:%d (%lu)\n", 
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
/* Process unknown packet				    		  */
/**************************************************************************/
void v3_process_unknow_dep()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long  uin_num, dep_number, temp_stamp;
   struct online_user user;

   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num 
            >> temp_stamp >> temp_stamp >> dep_number;
 
   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
          (user.udp_port == int_pack.from_port))
      {    
         v3_send_ack(int_pack);
      }
      else
      {
         LOG_ALARM(0, ("Spoofed unknown_dep request from %s:%d (%lu)\n", 
         inet_ntoa(int_pack.from_ip), int_pack.from_port, uin_num));
      }
   }
   else
   {
      v3_send_not_connected(int_pack);
   }
}


/**************************************************************************/
/* Process set-work-user-info packet			    		  */
/**************************************************************************/
void v3_process_setwork()
{
   unsigned short pvers, pcomm, seq1, seq2, str_len;
   unsigned long  uin_num, temp_stamp, i;
   struct online_user user;
   struct full_user_info userinfo;

   int_pack.reset();
 
   int_pack >> pvers >> pcomm >> seq1 >> seq2 
            >> uin_num >> temp_stamp >> str_len;
	  
   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
          (user.udp_port == int_pack.from_port))
      {
         v3_send_ack(int_pack);
      
         int_pack.reset();
         int_pack >> pvers >> pcomm >> seq1 >> seq2 
                  >> uin_num >> temp_stamp >> str_len;

         userinfo.wzip = 0; userinfo.wcountry = 0; userinfo.wdepart = 0;

         /* extract waddr from packet */
         if (!islen_valid(str_len, 64, "waddr", user))
         {
  	    move_user_offline(user.uin);
	    v3_send_not_connected(int_pack);
	    return;
	 
         } else { for(i=0; i<str_len; i++) int_pack >> userinfo.waddr[i]; }
      
         /* extract work city from packet */
         int_pack >> str_len;
         if (!islen_valid(str_len, 32, "wcity", user))
         {
  	    move_user_offline(user.uin);
	    v3_send_not_connected(int_pack);
	    return;
	 
         } else { for (i=0; i<str_len; i++) int_pack >> userinfo.wcity[i]; }

         /* extract work state name from packet */
         int_pack >> str_len;
         if (!islen_valid(str_len, 32, "wstate", user))
         {
  	    move_user_offline(user.uin);
	    v3_send_not_connected(int_pack);
	    return;
	 
         } else { for (i=0; i<str_len; i++) int_pack >> userinfo.wstate[i]; }
      
         int_pack >> userinfo.wcountry;

         /* extract work company number from packet */
         int_pack >> str_len;
         if (!islen_valid(str_len, 32, "wcompany", user))
         {
  	    move_user_offline(user.uin);
	    v3_send_not_connected(int_pack);
	    return;
 
         } else { for (i=0; i<str_len; i++) int_pack >> userinfo.wcompany[i]; }

         /* extract work title number from packet */
         int_pack >> str_len;
         if (!islen_valid(str_len, 32, "wtitle", user))
         {
  	    move_user_offline(user.uin);
	    v3_send_not_connected(int_pack);
	    return;
	 
         } else { for (i=0; i<str_len; i++) int_pack >> userinfo.wtitle[i]; }

         int_pack >> userinfo.wdepart;
      
         /* extract work phone number from packet */
         int_pack >> str_len;
         if (!islen_valid(str_len, 32, "wphone", user))
         {
  	    move_user_offline(user.uin);
	    v3_send_not_connected(int_pack);
	    return;
	 
         } else { for (i=0; i<str_len; i++) int_pack >> userinfo.wphone[i]; }

         /* extract work fax number from packet */
         int_pack >> str_len;
         if (!islen_valid(str_len, 32, "wfax", user))
         {
  	    move_user_offline(user.uin);
	    v3_send_not_connected(int_pack);
	    return;
	 
         } else { for (i=0; i<str_len; i++) int_pack >> userinfo.wfax[i]; }

         /* extract work pager number from packet */
         int_pack >> str_len;
         if (!islen_valid(str_len, 32, "wpager", user))
         {
  	    move_user_offline(user.uin);
	    v3_send_not_connected(int_pack);
	    return;
	 
         } else { for (i=0; i<str_len; i++) int_pack >> userinfo.wpager[i]; }

        int_pack >> userinfo.wzip;
      
        db_users_setwork_info(user.uin, userinfo);
        v3_send_reply_ok(int_pack, user, ICQ_CMDxSND_USERxSET_WORK_INFO_OK);
     }
     else
     {
        LOG_ALARM(0, ("Spoofed set work info request from %s:%d (%lu)\n", 
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
/* Process set-home-user-info packet			    		  */
/**************************************************************************/
void v3_process_sethome()
{
   unsigned short pvers, pcomm, seq1, seq2, str_len;
   unsigned long  uin_num, temp_stamp, i;
   struct online_user user;
   struct full_user_info userinfo;

   int_pack.reset();
 
   int_pack >> pvers >> pcomm >> seq1 >> seq2 
            >> uin_num >> temp_stamp >> str_len;
	  
    if (db_online_lookup(uin_num, user) == 0)
    {
       if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
           (user.udp_port == int_pack.from_port))
       {
          v3_send_ack(int_pack);
      
          int_pack.reset();
          int_pack >> pvers >> pcomm >> seq1 >> seq2 
                   >> uin_num >> temp_stamp >> str_len;

          bzero((void *)&userinfo, sizeof(struct full_user_info));

           /* extract home address from packet */
          if (!islen_valid(str_len, 64, "haddr", user))
          {
  	     move_user_offline(user.uin);
	     v3_send_not_connected(int_pack);
	     return;
	 
          } else { for(i=0; i<str_len; i++) int_pack >> userinfo.haddr[i]; }
      
          /* extract home city from packet */
          int_pack >> str_len;
          if (!islen_valid(str_len, 32, "hcity", user))
          {
  	      move_user_offline(user.uin);
	      v3_send_not_connected(int_pack);
	      return;
	 
          } else { for (i=0; i<str_len; i++) int_pack >> userinfo.hcity[i]; }

          /* extract home state name from packet */
          int_pack >> str_len;
          if (!islen_valid(str_len, 32, "hstate", user))
          {
  	      move_user_offline(user.uin);
	      v3_send_not_connected(int_pack);
	      return;
	      	 
          } else { for (i=0; i<str_len; i++) int_pack >> userinfo.hstate[i]; }
      
          int_pack >> userinfo.hcountry;
      
          /* extract home phone number from packet */
          int_pack >> str_len;
          if (!islen_valid(str_len, 32, "hphone", user))
          {
  	     move_user_offline(user.uin);
	     v3_send_not_connected(int_pack);
	     return;
	 
          } else { for (i=0; i<str_len; i++) int_pack >> userinfo.hphone[i]; }

          /* extract home fax number from packet */
          int_pack >> str_len;
          if (!islen_valid(str_len, 32, "hfax", user))
          {
  	     move_user_offline(user.uin);
	     v3_send_not_connected(int_pack);
	     return;
	 
          } else { for (i=0; i<str_len; i++) int_pack >> userinfo.hfax[i]; }

          /* extract home cell number from packet */
          int_pack >> str_len;
          if (!islen_valid(str_len, 32, "hcell", user))
          {
  	     move_user_offline(user.uin);
	     v3_send_not_connected(int_pack);
	     return;
	 
          } else { for (i=0; i<str_len; i++) int_pack >> userinfo.hcell[i]; }

         int_pack >> userinfo.hzip;
         int_pack >> userinfo.gender;
         int_pack >> userinfo.age;
         int_pack >> userinfo.bday;
         int_pack >> userinfo.bmonth;
         int_pack >> userinfo.byear;
      
         db_users_sethome_info(user.uin, userinfo);
         db_online_sseq_open(user);
         v3_send_reply_ok(int_pack, user, ICQ_CMDxSND_USERxSET_HOME_INFO_OK);
         db_online_sseq_close(user, 1);
      }
      else
      {
    
         LOG_ALARM(0, ("Spoofed set home info request from %s:%d (%lu)\n", 
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
/* Process user set homepage packet					  */
/**************************************************************************/
void v3_process_setweb(int htype)
{
   unsigned short pvers, pcomm, seq1, seq2, str_len;
   unsigned short ok_type = ICQ_CMDxSND_USERxSET_HOME_PAGE_OK;
   unsigned long  uin_num, temp_stamp, i;
   struct online_user user;
   fstring webpage;
  
   int_pack.reset();
  
   int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num;
	  
   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
          (user.udp_port == int_pack.from_port))
      {
         v3_send_ack(int_pack);
      
         int_pack.reset();
         int_pack >> pvers >> pcomm >> seq1 >> seq2 
                  >> uin_num >> temp_stamp >> str_len;

         /* extract nick from packet */
         if (!islen_valid(str_len, 128, "webpage", user))
         {
            move_user_offline(user.uin);
            v3_send_not_connected(int_pack);
            return;
   	 
         } else { for(i=0; i<str_len; i++) int_pack >> webpage[i]; }

         db_users_setwebpage_info(user.uin, webpage, htype);
      
         if (htype == HOME) ok_type = ICQ_CMDxSND_USERxSET_HOME_PAGE_OK;
         if (htype == WORK) ok_type = ICQ_CMDxSND_USERxSET_WORK_PAGE_OK;
         db_online_sseq_open(user);
         v3_send_reply_ok(int_pack, user, ok_type);
         db_online_sseq_close(user, 1);

      }
      else
      {
    
         LOG_ALARM(0, ("Spoofed set www page request from %s:%d (%lu)\n", 
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
/* Process set-basic-user-info packet			    		  */
/**************************************************************************/
void v3_process_setbasic()
{
   unsigned short pvers, pcomm, seq1, seq2, str_len;
   unsigned long  uin_num, temp_stamp, i;
   struct online_user user;
   struct full_user_info userinfo;

   int_pack.reset();
   
   int_pack >> pvers >> pcomm >> seq1 >> seq2 
            >> uin_num >> temp_stamp >> str_len;
  	  
   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
          (user.udp_port == int_pack.from_port))
      {
        v3_send_ack(int_pack);
      
        send_event2ap(papack, ACT_SAVEBASIC, user.uin, user.status, ipToIcq(user.ip), 
	              0, longToTime(time(NULL)), "");                     
      
        int_pack.reset();
        int_pack >> pvers >> pcomm >> seq1 >> seq2 
                 >> uin_num >> temp_stamp >> str_len;

        /* extract nick from packet */
        if (!islen_valid(str_len, 32, "nick", user))
        {
    	   move_user_offline(user.uin);
  	   v3_send_not_connected(int_pack);
  	   return;
  	 
        } else { for(i=0; i<str_len; i++) int_pack >> userinfo.nick[i]; }
      
        /* extract first name from packet */
        int_pack >> str_len;
        if (!islen_valid(str_len, 32, "first", user))
        {
    	   move_user_offline(user.uin);
  	   v3_send_not_connected(int_pack);
  	   return;
  	 
        } else { for (i=0; i<str_len; i++) int_pack >> userinfo.first[i]; }

        /* extract last name from packet */
        int_pack >> str_len;
        if (!islen_valid(str_len, 32, "last", user))
        {
    	   move_user_offline(user.uin);
  	   v3_send_not_connected(int_pack);
  	   return;
  	 
        } else { for (i=0; i<str_len; i++) int_pack >> userinfo.last[i]; }
      
        /* extract email name from packet */
        int_pack >> str_len;
        if (!islen_valid(str_len, 128, "email", user))
        {
    	   move_user_offline(user.uin);
  	   v3_send_not_connected(int_pack);
  	   return;
  	 
        } else { for (i=0; i<str_len; i++) int_pack >> userinfo.email1[i]; }
        
        (void)strncpy(userinfo.email2, userinfo.email1, sizeof(userinfo.email2));
        
        db_users_setbasic_info(user.uin, userinfo);
        db_online_sseq_open(user);
        v3_send_reply_ok(int_pack, user, ICQ_CMDxSND_USERxSET_BASIC_INFO_OK);
        db_online_sseq_close(user, 1);

     }
     else
     {
    
        LOG_ALARM(0, ("Spoofed set basic info request from %s:%d (%lu)\n", 
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
/* Process set notes packet 				    		  */
/**************************************************************************/
void v3_process_setnotes()
{
   unsigned short pvers, pcomm, seq1, seq2, notes_len;
   unsigned long  uin_num, temp_stamp;
   struct online_user user;
   struct notes_user_info notes;

   int_pack.reset(); notes_len = 0;
   
   int_pack >> pvers >> pcomm >> seq1 >> seq2 
            >> uin_num >> temp_stamp >> notes_len;
  	  
   for (int i=0; i<notes_len; i++) int_pack >> notes.notes[i];
    
   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) && 
          (user.udp_port == int_pack.from_port))
      {
         v3_send_ack(int_pack);
         if (notes_len > 256)
         {
            LOG_ALARM(0, ("Warning: User %lu from %s:%ld sent too big notes (%d)...\n",
            		   user.uin, inet_ntoa(user.ip), user.udp_port, notes_len));
	    notes_len = 256;
	 }

         DEBUG(50, ("Receive notes : %s\n", notes.notes));
      
         db_users_setnotes(user.uin, notes);
         v3_send_reply_ok(int_pack, user, ICQ_CMDxSND_USERxSET_NOTES_OK);

      }
      else
      {
       
        LOG_ALARM(0, ("Spoofed set notes request from %s:%d (%lu)\n", 
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
/* First login packet handler						  */
/**************************************************************************/
void v3_process_firstlog()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long uin_num;

   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num;
 
   DEBUG(100, ("First login from uin:%lu\n", uin_num));
   LOG_USR(10, ("Got first login from user %lu [%s:%d]\n", uin_num, 
                inet_ntoa(int_pack.from_ip), int_pack.from_port));

   /* may be i should do more here, but i don't know what */ 
   v3_send_ack(int_pack);
}


/**************************************************************************/
/* Get externals packet handler						  */
/**************************************************************************/
void v3_process_getext()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long uin_num, temp_stamp, ext_number;

   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 
            >> uin_num >> temp_stamp  >> ext_number;
  
   DEBUG(100, ("Externals request from %lu (requested number: %lu)\n", 
                uin_num, ext_number));
  
   DEBUG(100, ("TODO: Move Mirabilis ext list to db and make send_ext packet\n"));
  
   v3_send_ack(int_pack);
}


/**************************************************************************/
/* Usage statistic packet handler					  */
/**************************************************************************/
void v3_process_usagestats()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long uin_num, temp_stamp;
   struct online_user user;

   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 
            >> uin_num >> temp_stamp;
  
   DEBUG(100, ("User usage statistic from %lu\n", uin_num));

   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
          (user.udp_port == int_pack.from_port))
      {
         v3_send_ack(int_pack);

         LOG_USR(100, ("User %lu have sent usage statistic\n", user.uin));
         
      } else {
     
         LOG_ALARM(0, ("Spoofed usage statistic from %s:%d (%lu)\n", 
         inet_ntoa(int_pack.from_ip), int_pack.from_port, uin_num));
         v3_send_not_connected(int_pack);
      }   

  } else v3_send_not_connected(int_pack);
  
}


/**************************************************************************/
/* Set online_info packet handler					  */
/**************************************************************************/
void v3_process_onlineinfo()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long uin_num, temp_stamp;
   struct online_user user;

   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 
            >> uin_num >> temp_stamp;
  
   DEBUG(100, ("User set_online packet from %lu\n", uin_num));

   /* I need several dumps of this packet */
   log_alarm_packet(0, int_pack);

   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
          (user.udp_port == int_pack.from_port))
      {
         v3_send_ack(int_pack);
         LOG_USR(100, ("User %lu trying to set online_info\n", user.uin));		
        
      } else {
     
         LOG_ALARM(0, ("Spoofed set_online info from %s:%d (%lu)\n", 
         inet_ntoa(int_pack.from_ip), int_pack.from_port, uin_num));
         v3_send_not_connected(int_pack);
      }   

  } else v3_send_not_connected(int_pack);
  
}

/**************************************************************************/
/* Get depslist pseudo login packet				    	  */
/**************************************************************************/
void v3_process_getdeps()
{
   unsigned short vers, comm, seq1, seq2, i, plen;
   unsigned long uin_num, ljunk;
   struct online_user nuser, check_user;
   struct login_user_info userinfo;
   char usrpass[20];
   
   int_pack.reset();
   int_pack >> vers    >> comm 
 	    >> seq1    >> seq2 
            >> ljunk   >> ljunk
 	    >> uin_num;
 	   
   int_pack >> plen; if (plen > 20) plen = 20; 
   for (i=0; i<plen; i++) int_pack >> usrpass[i];

   if (db_users_lookup(uin_num, userinfo) == -1)
   {
      LOG_USR(0, ("Can't find user %lu in database.\n", uin_num));
      v3_send_pass_err(int_pack);
      return;
   }
   
   if (!strcsequal(usrpass, userinfo.passwd))
   {
      LOG_USR(0, ("User %lu pseudo login with wrong password.\n", uin_num));
      v3_send_pass_err(int_pack);
      return;
   }

   if (db_online_lookup(uin_num, check_user) != 0)
   {
      nuser.uin        = uin_num;
      nuser.ip         = int_pack.from_ip;
      nuser.udp_port   = int_pack.from_port;
      nuser.ttl	       = get_ping_time(vers);
      nuser.protocol   = vers;
      nuser.servseq    = 1;
      nuser.session_id = 0;
      
      v3_send_ack(int_pack);

      DEBUG(50, ("Pseudo login packet from %s:%ld (as %lu)\n",
                 inet_ntoa(nuser.ip), nuser.udp_port, nuser.uin));
 		
      v3_send_depslist(int_pack, nuser, 0);

   }
   else
   {
       v3_send_busy(uin_num, seq2, int_pack.from_ip, int_pack.from_port);
       LOG_USR(0, ("User %lu from %s:%d request login but already online.\n", 
                   uin_num, inet_ntoa(int_pack.from_ip), int_pack.from_port));
   } 
}


/**************************************************************************/
/* Getinfo packet handler						  */
/**************************************************************************/
void v3_process_getinfo()
{
  unsigned short pvers, pcomm, seq1, seq2;
  unsigned long  uin_num, to_uin, temp_stamp;
  struct online_user user;
  struct full_user_info tuser;

  int_pack.reset();
  int_pack >> pvers >> pcomm >> seq1 >> seq2 
           >> uin_num >> temp_stamp >> to_uin;
  
  if (db_online_lookup(uin_num, user) == 0)
  {
     if (ipToIcq(user.ip) == ipToIcq(int_pack.from_ip))
     {
       v3_send_ack(int_pack);
       
       if (db_users_lookup(to_uin, tuser) != -1)
       {
          /* we found user's notes */
          v3_send_basic_info(int_pack, user, tuser);
 	  v3_send_home_info(int_pack, user, tuser);
 	  v3_send_home_web(int_pack, user, tuser);
 	  v3_send_work_info(int_pack, user, tuser);
 	  v3_send_work_web(int_pack, user, tuser);
       }
       else
       {
       
          LOG_SYS(0, ("We can't find user info (No such user...)\n"));
          /* We can't find notes of the requested user */
          v3_send_invalid_user(int_pack, user);
       }
     }
     else
     {     
        LOG_ALARM(0, ("Spoofed notes request from %s:%d (%lu)\n", 
        inet_ntoa(int_pack.from_ip), int_pack.from_port, uin_num));
     }   
  }
  else
  {
     v3_send_not_connected(int_pack);
  }
} 


/**************************************************************************/
/* Getinfo1 packet handler (client want only basic info)		  */
/**************************************************************************/
void v3_process_getinfo1()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long  uin_num, to_uin, temp_stamp;
   struct online_user user;
   struct full_user_info tuser;

   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 
            >> uin_num >> temp_stamp >> to_uin;
   
   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
          (user.udp_port == int_pack.from_port))
      {
         v3_send_ack(int_pack);
        
         if (db_users_lookup(to_uin, tuser) != -1)
         {
            /* we found user's notes */
  	    db_online_sseq_open(user);
            v3_send_basic_info_single(int_pack, user, tuser);
  	    db_online_sseq_close(user, 1);
  	
         }
         else
         {
            /* We can't find notes of the requested user */
  	    db_online_sseq_open(user);
            v3_send_invalid_user(int_pack, user);
  	    db_online_sseq_close(user, 1);
         }

         db_online_update_servseq(user);     
      }
      else
      {
         LOG_ALARM(0, ("Spoofed getinfo1 request from %s:%d (%lu)\n", 
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
/* Login packet handler							  */
/**************************************************************************/
void v3_process_login()
{
   unsigned short vers, comm, seq1, seq2, i, plen;
   unsigned long uin_num, ljunk, tport, tcpversion, int_ip;
   unsigned short status, estatus;
   char dc_type;
   struct online_user nuser, *user;
   struct login_user_info userinfo;
   char usrpass[20];
   int lookup_res;
   fstring lock_text;

   v3_send_ack(int_pack);
  
   int_pack.reset();
   int_pack >> vers    >> comm 
	    >> seq1    >> seq2 
            >> uin_num >> ljunk 
	    >> tport;
	   
   int_pack >> plen; if (plen > 20) plen = 20; 
   for (i=0; i<plen; i++) int_pack >> usrpass[i];
 
   int_pack >> ljunk   >> int_ip
            >> dc_type >> status 
 	    >> estatus >> tcpversion;
	   
   if (!lp_v3_enabled()) 
   {
      v3_send_login_err(int_pack, "V3 protocol disabled by administrator...\x0d\x0aTry to use another client version...");
      return;
   }

   lookup_res = db_users_lookup(uin_num, userinfo);	   
   if (lookup_res == -1)
   {
      LOG_USR(0, ("Can't find user %lu in database.\n", uin_num));
      v3_send_pass_err(int_pack);
      return;
   }

   if (lookup_res == -2)
   {
      LOG_ALARM(0, ("SQL error in database - we shouldn't accept login\n"));
      v3_send_login_err(int_pack, "Database error. Try to reconnect later...");
      return;
   }
   
   if (!strcsequal(usrpass, userinfo.passwd))
   {
      LOG_USR(0, ("User %lu trying login with wrong password.\n", uin_num));
      v3_send_pass_err(int_pack);
      return;
   }

   if ((userinfo.ip_addr != 0) &&
       (lp_restrict2luip()) &&
       (userinfo.ip_addr != ipToIcq(int_pack.from_ip)))
   {
      
      LOG_USR(0, ("User %lu trying login from wrong ip-addres.\n", uin_num));
      v3_send_login_err(int_pack, "You can't login from this IP address");
      return;
   }

   if (userinfo.disabled == 1)
   {
     LOG_USR(0, ("User %lu trying login but his account locked.\n", uin_num));

     if (db_users_lock_message(uin_num, lock_text))
     {  
        ITrans.translateToClient(lock_text);
        v3_send_login_err(int_pack, lock_text);
     } else v3_send_login_err(int_pack, "Account locked...");
     
     return;
   }    

   bzero((void *)&nuser, sizeof(nuser));
   
   /* well, now we should check if there is another user connected... */
   if ((user = shm_get_user(uin_num)) != NULL)
   {
      snprintf(lock_text, 127, "User %lu already connected.\x0d\x0aTry again later.", user->uin);
      v3_send_login_err(int_pack, lock_text);
      LOG_USR(0, ("User %lu from %s:%d request login but already online.\n", 
                   uin_num, inet_ntoa(int_pack.from_ip), int_pack.from_port)); 
      return;
   }

   /* fill new record */   
   nuser.uin        = uin_num;
   nuser.usid       = rand();
   nuser.ip         = int_pack.from_ip;
   nuser.tcp_port   = tport;
   nuser.udp_port   = int_pack.from_port;
   nuser.status     = status;
   nuser.estat      = estatus;
   nuser.uptime     = longToTime(time(NULL));
   nuser.lutime     = longToTime(time(NULL));
   nuser.ttl	    = get_ping_time(vers);
   nuser.ttlv       = get_ping_time(vers);
   nuser.protocol   = vers;
   nuser.servseq    = 0;
   nuser.tcpver     = tcpversion;
   nuser.active     = 1;
   nuser.uclass     = CLASS_FREE | CLASS_ICQ;
   nuser.dc_type    = dc_type;
   nuser.int_ip     = icqToIp(int_ip);
   nuser.lutime     = time(NULL);

   if (nuser.status != 0) nuser.uclass = nuser.uclass | CLASS_AWAY;

   nuser.mopt[MCH1].max_msglen   = lp_v3_max_msgsize();
   nuser.mopt[MCH1].max_sevil    = 999;
   nuser.mopt[MCH1].max_revil    = 999;
   nuser.mopt[MCH1].icbm_flags   = ICBM_FLG_BASE;
   
   nuser.mopt[MCH4].max_msglen   = lp_v3_max_msgsize();
   nuser.mopt[MCH4].max_sevil    = 999;
   nuser.mopt[MCH4].max_revil    = 999;
   nuser.mopt[MCH4].icbm_flags   = ICBM_FLG_BASE;

   if (db_online_insert(nuser) != 0)
   {
      v3_send_login_err(int_pack, "Server users table full. Try again later");
      return;
   }

   results_delay(50); /* sleep 10 msecs */
   v3_send_login_reply(int_pack, userinfo, nuser);

   move_user_online(nuser);
   LOG_USR(0, ("User %lu from %s:%ld moved online.\n", nuser.uin, 
                inet_ntoa(nuser.ip), nuser.udp_port));

}


/**************************************************************************/
/* ack packet handler							  */
/**************************************************************************/
void v3_process_ack()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long uin_num, temp_stamp;

   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num;
   temp_stamp = generate_ack_number(uin_num, seq1, seq2);
  
   DEBUG(350, ("GOT ACK: ver:%d, comm:%d, seq1:%d, seq2:%d, uin:%lu, stamp:%lu\n", 
                pvers, pcomm, seq1, seq2, uin_num, temp_stamp));
  
   process_ack_event(uin_num, temp_stamp);
}


/**************************************************************************/
/* "Text message" packet handler					  */
/**************************************************************************/
void v3_process_logoff()
{
   unsigned short pvers, pcomm, seq1, seq2, i, plen;
   unsigned long  uin_num, temp_stamp;
   char bmessage[20];

   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num >> temp_stamp;
   
   int_pack >> plen; if (plen > 20 ) plen = 20;
   for (i=0; i<plen; i++) int_pack >> bmessage[i];

   if (strequal(bmessage,"B_MESSAGE_ACK")) v3_send_oob(uin_num);
   if (strequal(bmessage,"B_USER_DISCONNECTED"))
   {
      struct online_user temp_user;
      unsigned long uptime;
      
      if (db_online_lookup(uin_num, temp_user) == 0)
      {
        if ((ipToIcq(temp_user.ip) == ipToIcq(int_pack.from_ip)) &&
	    (temp_user.udp_port == int_pack.from_port))
        { 
           move_user_offline(uin_num);
  	   uptime = (unsigned long)(time(NULL) - longToTime(temp_user.uptime));
	 
	   LOG_USR(0, ("User %lu moved offline. Online time was: %lu seconds...\n", 
	                uin_num, uptime));
        }
        else 
        { 
           /* removed because stupid mirabilis clients send disconnect packet */
	   /* even they are not connected */
	 
           /* LOG_ALARM(0, ("Someone from %s:%d trying to disconnect user %d\n", 
              inet_ntoa(int_pack.from_ip), int_pack.from_port, uin_num)); */

           v3_send_not_connected(int_pack);

      }
    }
    else 
    {
      v3_send_not_connected(int_pack);
    }
  }
}


/**************************************************************************/
/* Contact list hanlder							  */
/**************************************************************************/
void v3_process_contact()
{
   unsigned short pvers, pcomm, seq1, seq2, i;
   unsigned long  uin_num, temp_stamp, rid;
   int count = 0; char ccount;
   struct online_user user, auser, *pauser;
   PGresult *res;
   fstring dbcomm_str;
   int on_cnt;

   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 
            >> uin_num >> temp_stamp >> ccount;

   count = (int)ccount;
   rid   = lrandom_num();

   /* there is no need to check count value because we use */
   /* char variable for this data (max = 256) */
   
   for (i=0;i<count;i++) int_pack >> contacts[i];

   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
          (user.udp_port == int_pack.from_port))
      {
         v3_send_ack(int_pack);
         /* first of all we need ready contact-list in database */
         db_contact_insert(uin_num, count, contacts, NORMAL_CONTACT, rid);

#define BCST_V3M1341 \
"SELECT uin,ishm FROM online_users \
 JOIN \
 ( \
    SELECT tuin FROM online_contacts \
    WHERE (ouin=%lu) AND (type=%d) AND (rid=%lu) \
    GROUP BY tuin LIMIT %d \
 ) \
 AS TMP on uin=TMP.tuin \
 WHERE active=1"
	 
         /* exec select command on sql server */
         slprintf(dbcomm_str, sizeof(dbcomm_str)-1, 
	          BCST_V3M1341, uin_num, NORMAL_CONTACT, 
		  rid, lp_v7_max_contact_size());
	 
         res = PQexec(users_dbconn, dbcomm_str);
         if (PQresultStatus(res) != PGRES_TUPLES_OK)
         {
            handle_database_error(res, "[V3 PROCESS CONTACT]");
            v3_send_not_connected(int_pack);
            return;
         }

         on_cnt = PQntuples(res);
       
         for (i=0;i<on_cnt;i++) 
         {	 
            /* fill auser structure uin & ishm fields */
  	    auser.uin	    = atoul(PQgetvalue(res, i, 0));
  	    auser.shm_index = atoul(PQgetvalue(res, i, 1));
	
	    /* find online user in shm */
	    pauser = shm_iget_user(auser.uin, auser.shm_index);
            if (pauser == NULL) continue;

            /* copy found user to auser structure */
	    memcpy((void *)&auser, (const void *)pauser, 
	            sizeof(struct online_user));
	    
  	    /* send packet to user */
            v3_send_user_online(user, auser);
	    
         }
       
         PQclear(res);
         v3_send_end_contact(user);
      }
      else
      {
    
         LOG_ALARM(0, ("Spoofed contact list from %s:%d (%lu)\n", 
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
/* User add packet hanlder						  */
/**************************************************************************/
void v3_process_useradd()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long  uin_num, temp_stamp, contact;
   struct online_user user, auser;

   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num >> temp_stamp;
   int_pack >> contact;

   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
          (user.udp_port == int_pack.from_port))
      {
         v3_send_ack(int_pack);
         
         if ((db_online_lookup(contact, auser) == 0) &&
	     (auser.active == 1))
         {  
            v3_send_user_online(user, auser);
         }
         
         db_contact_insert(uin_num, 1, &contact, NORMAL_CONTACT, lrandom_num());
         
      }
      else
      {
    
         LOG_ALARM(0, ("Spoofed add user cmd from %s:%d (%lu)\n", 
         inet_ntoa(int_pack.from_ip), int_pack.from_port, uin_num));
      }
   }
   else
   {
      v3_send_not_connected(int_pack);
   }
}


/**************************************************************************/
/* Notes request hanlder						  */
/**************************************************************************/
void v3_process_notes()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long  uin_num, to_uin, temp_stamp;
   struct online_user user;
   struct notes_user_info notes;

   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 
            >> uin_num >> temp_stamp >> to_uin;
   
   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
          (user.udp_port == int_pack.from_port))
      {
        v3_send_ack(int_pack);
       
        if (db_users_notes(to_uin, notes) != -1)
        {
           /* we found user's notesv */
           v3_send_notes(int_pack, user, notes);
        }
        else
        {
           /* We can't find notes of the requested user */
           v3_send_invalid_user(int_pack, user);
        }
      }
      else
      {
      
         LOG_ALARM(0, ("Spoofed notes request from %s:%d (%lu)\n", 
         inet_ntoa(int_pack.from_ip), int_pack.from_port, uin_num));
      }   
   }
   else
   {
      v3_send_not_connected(int_pack);
   }
}


/**************************************************************************/
/* Ping packet hanlder							  */
/**************************************************************************/
void v3_process_ping()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long  uin_num, temp_stamp;
   struct online_user user;

   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num >> temp_stamp;
  
   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
          (user.udp_port == int_pack.from_port))
      {
    
         v3_send_ack(int_pack);
         
         DEBUG(350, ("Process user ping command from %s:%d (%lu)\n", 
         inet_ntoa(int_pack.from_ip), int_pack.from_port, user.uin));
   	
         db_online_touch( user );
         send_update2cache( user.uin, user.ttl );

      }
      else
      {

         LOG_ALARM(0, ("Spoofed (%lu) ping message from %s:%d\n", uin_num,
	                inet_ntoa(int_pack.from_ip), int_pack.from_port));
      }
   }
   else
   {
  
      v3_send_not_connected(int_pack);
   }
}


/**************************************************************************/
/* Get deplist #1 packet hanlder					  */
/**************************************************************************/
void v3_process_getdeps1()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long  uin_num, temp_stamp;
   struct online_user user;

   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num >> temp_stamp;
   
   if (db_online_lookup(uin_num, user) == 0)
   {
      if (ipToIcq(user.ip) == ipToIcq(int_pack.from_ip))
      {
      
         v3_send_ack(int_pack);
         
         DEBUG(10, ("Process user get depslist request from %s:%d (%lu)\n", 
         inet_ntoa(int_pack.from_ip), int_pack.from_port, user.uin));
         
         v3_send_depslist1(int_pack, user, user.servseq); 

      }
      else
      {

         LOG_ALARM(0, ("Spoofed (%lu) get depslist request from %s:%d\n", uin_num,
     		        inet_ntoa(int_pack.from_ip), int_pack.from_port));
         v3_send_not_connected(int_pack);
      }
   }
   else
   {
     v3_send_not_connected(int_pack);
   }
}


/**************************************************************************/
/* Status change hanlder						  */
/**************************************************************************/
void v3_process_status()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long  uin_num, temp_stamp;
   unsigned long  new_status, old_status, new_estatus;
   struct online_user user;

   int_pack.reset();
   int_pack >> pvers      >> pcomm      
            >> seq1       >> seq2 
            >> uin_num    >> temp_stamp 
 	    >> new_status >> new_estatus;
  
   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
          (user.udp_port == int_pack.from_port))
      {
      
         v3_send_ack(int_pack);
         
	 send_event2ap(papack, ACT_STATUS, user.uin, user.status,                
                       ipToIcq(user.ip), new_status, longToTime(time(NULL)), "");
         
         DEBUG(100, ("Process user status change command from %s:%d (%lu)\n", 
         inet_ntoa(int_pack.from_ip), int_pack.from_port, user.uin));
	
         old_status  = user.status;
         user.status = new_status;
         user.estat  = new_estatus;
	 
	 if (user.status != 0) user.uclass = user.uclass | CLASS_AWAY;
         if (user.status == 0) user.uclass = user.uclass & (~CLASS_AWAY);
	 
         db_online_setstatus(user);
         broadcast_status( user, old_status);
        
      }
      else
      {

         LOG_ALARM(0, ("Spoofed (%lu) status change message from %s:%d\n", uin_num,
  		        inet_ntoa(int_pack.from_ip), int_pack.from_port));
      }
   }
   else
   {
      v3_send_not_connected(int_pack);
   }
}


/**************************************************************************/
/* Used to send packets, that should be confirmed with ack 		  */
/**************************************************************************/
int v3_send_indirect(Packet &pack, unsigned long to_uin, unsigned long shm_index)
{

   if (pack.sizeVal <= lp_v3_packet_mtu())
   {
      event_send_packet(pack, to_uin, shm_index, v3_retries, v3_timeout, V3_PROTO);
      return(1);
   }
   else
   {
      return(v3_send_packet_in_fragments(pack, to_uin, shm_index));
   }
}


/**************************************************************************/
/* Used to init all proto variables and data structures 		  */
/**************************************************************************/
void v3_proto_init()
{
  v3_timeout = lp_v3_timeout();
  v3_retries = lp_v3_retries();  
}


/**************************************************************************/
/* Put sseq in packet			 				  */
/**************************************************************************/
void PutSeq3(Packet &pack, unsigned short cc)
{
    unsigned short cc_size;
    unsigned short check;
    unsigned short i, j;

    check = ReverseShort(*(unsigned short *) &cc);

    cc_size = sizeof(check);
    for(i = 0, j = 0x04; i < cc_size; i++)
    	pack.buff[j++] = check >> (i * 8);

}

