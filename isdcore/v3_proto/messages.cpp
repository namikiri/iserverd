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
/* This unit implements messages thru server for V3 transport		  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#include "v3_defines.h"

extern int v3_timeout; 
extern int v3_retries;


/**************************************************************************/
/* System message packet handler 				    	  */
/**************************************************************************/
void v3_process_sysmsg()
{
 unsigned short pvers, pcomm, seq1, seq2, msg_type, msg_len, i;
 unsigned long  uin_num, to_uin, temp_stamp;
 struct online_user user;
 struct msg_header msg_hdr;

 bzero((void *)&msg_hdr, sizeof(msg_hdr));
 
 int_pack.reset();
 int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num >> temp_stamp
          >> to_uin >> msg_type >> msg_len;
 
 if (db_online_lookup(uin_num, user) == 0)
 {
    if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) && 
       (user.udp_port == int_pack.from_port))
    {
       v3_send_ack(int_pack);
       int_pack.reset();
       int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num >> temp_stamp
                >> to_uin >> msg_type >> msg_len;
      
       if (!islen_valid(msg_len, lp_v3_max_msgsize()+1, "message", user))
       {
    	  move_user_offline(user.uin);
	  v3_send_not_connected(int_pack);
	  return;	 
       } 
       else 
       { 
          for(i=0; i<msg_len; i++) int_pack >> msg_buff[i]; 
       }
      
       /* fill message header structure */
       msg_hdr.mtype     = msg_type;
       msg_hdr.mkind	 = 1;
       msg_hdr.touin     = to_uin;
       msg_hdr.fromuin   = uin_num;
       msg_hdr.seq2      = seq2;
       msg_hdr.from_ver  = V3_PROTO;
       msg_hdr.mtime     = timeToLong(time(NULL));
       msg_hdr.fromindex = user.shm_index;
       msg_hdr.msglen    = msg_len;
      
       process_message(msg_hdr, msg_buff);
       
    }
    else
    {
       LOG_ALARM(0, ("Spoofed msg thru server command from %s:%d (%lu)\n", 
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
/* Broadcast message packet handler 				    	  */
/**************************************************************************/
void v3_process_broadcast()
{
   unsigned short pvers, pcomm, seq1, seq2, msg_type, msg_len, i;
   unsigned long  uin_num, to_uin, temp_stamp, department;
   struct online_user user;
   struct login_user_info user_info;
 
   log_alarm_packet(100, int_pack);

   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num >> temp_stamp
            >> to_uin >> msg_type >> msg_len;
 
   if (db_online_lookup(uin_num, user, seq1, seq2) == 0)
   {
      if (ipToIcq(user.ip) == ipToIcq(int_pack.from_ip))
      {
         if ((uin_num != ADMIN_UIN) && (db_users_lookup(uin_num, user_info) != 0)) return;
         if ((uin_num != ADMIN_UIN) && (user_info.can_broadcast != 1)) 
         {
	    LOG_ALARM(0, ("User %lu sent broadcast but haven't rights to do this...\n", 
	  	           uin_num));
	    return;
         }
	 
	 if ((uin_num == ADMIN_UIN) && (int_pack.from_local != 1)) return;
    
         v3_send_ack(int_pack);
      
         int_pack.reset();
         int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num >> temp_stamp
                  >> department >> msg_type >> msg_len;
      
         if (!islen_valid(msg_len, lp_v3_max_msgsize()+1, "message", user))
         {
  	    move_user_offline(user.uin);
	    v3_send_not_connected(int_pack);
	    return;
	 
         }
         else
         {
            for(i=0; i<msg_len; i++) int_pack >> msg_buff[i];
         }
          
         if (pcomm == ICQ_CMDxRCV_BROADCAST_MSG_ALL)
         {
            broadcast_message(False, department, msg_buff, 0x14, uin_num);
         } 
         else
         {
            broadcast_message(True, department, msg_buff, 0x14, uin_num);
         }
       
      }
      else
      {
         LOG_ALARM(0, ("Spoofed broadcast message command from %s:%d (%lu)\n", 
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
/* Send message to user via V3 proto  				    	  */
/**************************************************************************/
int v3_send_user_message(struct msg_header &msg_hdr, struct online_user &to_user,
                         char *message)
{
  unsigned short mtype;

  /* high byte is used as message flag, v3 clients */
  /* doesn't understand that so we should mask it  */
  msg_hdr.mtype = msg_hdr.mtype & 0x00ff;  
  mtype = v3_convert_message_type(msg_hdr.mtype);
  
  reply_pack.clearPacket();
  reply_pack << (unsigned short)V3_PROTO
             << (unsigned short)ICQ_CMDxSND_SYSxMSGxONLINE
             << (unsigned short)to_user.servseq
             << (unsigned short)msg_hdr.seq2
             << (unsigned  long)to_user.uin
             << (unsigned  long)0x0000
	     << (unsigned  long)msg_hdr.fromuin
	     << (unsigned short)mtype
	     << (unsigned short)(strlen(message)+1)
	     << message;
	     
   DEBUG(10, ("Info: Sending online system message to %lu\n", to_user.uin));

   reply_pack.from_ip   = to_user.ip;
   reply_pack.from_port = to_user.udp_port;
   
   /* send packets to client, we need confirm! */
   v3_send_indirect(reply_pack, to_user.uin, to_user.shm_index);
   
   return(0);
}


/**************************************************************************/
/* Process system message done ack  				    	  */
/**************************************************************************/
void v3_process_sysack()
{
   unsigned short pvers, pcomm, seq1, seq2;
   unsigned long  uin_num, temp_stamp;
   struct online_user user;

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
                  >> uin_num >> temp_stamp >> temp_stamp;

         db_del_messages(uin_num, temp_stamp);
      
      }
      else
      {
         LOG_ALARM(0, ("Spoofed offline_msg_delete request from %s:%d (%lu)\n", 
         inet_ntoa(int_pack.from_ip), int_pack.from_port, uin_num));
      }
   }
   else
   { 
      v3_send_not_connected(int_pack);
   }
}


/**************************************************************************/
/* Send offline message to user via V3 proto  			    	  */
/**************************************************************************/
int v3_send_user_sysmsg(struct msg_header &msg_hdr, struct online_user &to_user,
                        char *message)
{

   unsigned short mtype;
   
   /* high byte is used as message flag, v3 clients */
   /* doesn't understand that so we should mask it  */
   msg_hdr.mtype = msg_hdr.mtype & 0x00ff;
   
   mtype = v3_convert_message_type(msg_hdr.mtype);
   
   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_SYSxMSGxOFFLINE
              << (unsigned short)to_user.servseq
              << (unsigned short)msg_hdr.seq2
              << (unsigned  long)to_user.uin
              << (unsigned  long)0x0000
	      << (unsigned  long)msg_hdr.fromuin
	      << (unsigned  long)msg_hdr.mtime
	      << (unsigned short)msg_hdr.mtype
	      << (unsigned short)(strlen(message)+1)
	      << message;
	     
   DEBUG(10, ("Info: Sending offline system message to %lu\n", to_user.uin));

   reply_pack.from_ip   = to_user.ip;
   reply_pack.from_port = to_user.udp_port;
   
   /* send packet to client, we need confirm! */
   v3_send_indirect(reply_pack, to_user.uin, to_user.shm_index);
   
   return(0);
   
}


/**************************************************************************/
/* Process system and offline messages request		    		  */
/**************************************************************************/
void v3_process_sysmsg_req()
{
   unsigned short pvers, pcomm, seq1, seq2, mess_num;
   unsigned long  uin_num, dep_number, temp_stamp;
   struct online_user user;
   PGresult *res;
   cstring dbcomm_str; 
   struct msg_header msg_hdr;
   bzero((void *)&msg_hdr, sizeof(msg_hdr));
  
   int_pack.reset();
   int_pack >> pvers >> pcomm >> seq1 >> seq2 >> uin_num 
            >> temp_stamp >> temp_stamp >> dep_number;
 
   if (db_online_lookup(uin_num, user) == 0)
   {
      if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
         (user.udp_port == int_pack.from_port))
      {
         v3_send_ack(int_pack);
      
         PQclear(PQexec(users_dbconn, "BEGIN;"));
         slprintf(dbcomm_str, sizeof(dbcomm_str)-1, 
                 "DECLARE mportal CURSOR FOR SELECT * FROM Users_Messages WHERE to_uin=%d ORDER BY msg_date", 
		  uin_num);

         res = PQexec(users_dbconn, dbcomm_str);
         if (PQresultStatus(res) != PGRES_COMMAND_OK)
         {
            handle_database_error(res, "[DECLARE PORTAL (msgs)]");
            return;
	 }
	 else PQclear(res);
      
         for (mess_num=0;;mess_num++)
         {
            res = PQexec(users_dbconn, "FETCH IN mportal");

            if (PQresultStatus(res) != PGRES_TUPLES_OK)
            {
               handle_database_error(res, "[FETCH MESSAGE FROM PORTAL]");
               break;
            }
	 
	    if (PQntuples(res) == 0) {PQclear(res); break;}

            if (PQnfields(res) != MESS_TBL_FIELDS)
	    {
	       LOG_SYS(0, ("Corrypted usr_messages table structure in db: \"%s\"\n", 
	                    lp_db_users()));
			    
               exit(EXIT_ERROR_DB_STRUCTURE);
	    }
	    
	    strncpy(msg_buff, PQgetvalue(res, 0, 4), sizeof(msg_buff)-1);
	    ITrans.translateToClient(msg_buff);
	    
	    msg_hdr.touin     = atoul(PQgetvalue(res, 0, 0));
	    msg_hdr.fromuin   = atoul(PQgetvalue(res, 0, 1));
	    msg_hdr.mtime     = atoul(PQgetvalue(res, 0, 2));
	    msg_hdr.mtype     =  atol(PQgetvalue(res, 0, 3));

	    v3_send_user_sysmsg(msg_hdr, user, msg_buff);
	 
	    PQclear(res);
         }
      
         PQclear(PQexec(users_dbconn, "CLOSE mportal"));
	 PQclear(PQexec(users_dbconn, "END;"));

         v3_send_reply_ok(int_pack, user, ICQ_CMDxSND_SYSxMSGxDONE);
         return;
      }
      else
      {
         LOG_ALARM(0, ("Spoofed sys messages request from %s:%d (%lu)\n", 
         inet_ntoa(int_pack.from_ip), int_pack.from_port, uin_num));
      }   
   }
   else
   { 
      v3_send_not_connected(int_pack);
   }
}


/**************************************************************************/
/* V5<-->V3 message converter         				    	  */
/**************************************************************************/
unsigned short v3_convert_message_type(unsigned short type)
{
   switch (type)
   {
      case 0x13: return(0x01);
      default:   return(type);
   }
}


