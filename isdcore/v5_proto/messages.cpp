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
/* This unit handle V5 messages thru server				  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#include "v5_defines.h"

extern int serverzone;

/**************************************************************************/
/* This packet mean that we should delete all messages from database	  */
/**************************************************************************/
void v5_process_sysmsg_delete()
{
    unsigned short pvers;
    unsigned long  uin_num, temp_stamp, session_id;
    struct online_user user;

    int_pack.reset();
    int_pack  >> pvers   >> temp_stamp
  	    >> uin_num >> session_id;

    if (db_online_lookup(uin_num, user) == 0)
    {
       if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
   	   (user.session_id == session_id) &&
	   (user.udp_port == int_pack.from_port))
       {
           v5_send_ack(int_pack);
           
           DEBUG(50, ("Processing sys msgs delete request from %lu\n", user.uin));
           db_del_messages(uin_num, 0xFFFFFFFF);

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
/* This func will create and send message to V5 client		   	  */
/**************************************************************************/
int v5_send_user_message(struct msg_header &msg_hdr, struct online_user &to_user,
                         char *message)
{
   int salloc = 1;
   unsigned short mtype = 0;
   unsigned short parts_num = 0;
   char *nmessage = NULL;
   
   char *msg_token = new char[lp_v5_max_msgsize()+10];
   unsigned short position = 0, last_part, curr_part_num = 0;
   BOOL last_message_part = False;
   
   nmessage = convert_message_text(msg_hdr.fromuin, msg_hdr.mtype, message);
   if (nmessage == NULL) { nmessage = message; salloc = 0; }

   parts_num = strlen(nmessage) / lp_v5_max_msgsize();
   if (strlen(nmessage) % lp_v5_max_msgsize() != 0) parts_num += 1;

   reply_pack.from_ip   = to_user.ip;
   reply_pack.from_port = to_user.udp_port;

   if (lp_v3_split_order() == ORDER_BACKWARD) position = strlen(nmessage);

   for (;;)
   {
      if (lp_v3_split_order() == ORDER_FORWARD)
      {
         /* first of all we should prepare message token    */
         if (abs(static_cast<int>(strlen(nmessage) - position)) <= lp_v5_max_msgsize())
         {
            /* this is the end of main message string */
	   last_part = strlen(nmessage) - position;
	   last_message_part = True;
	   memcpy(msg_token, nmessage+position, last_part);
	   msg_token[last_part] = 0;
	   position += last_part;
         }
         else
         {
	   memcpy(msg_token, nmessage+position, lp_v5_max_msgsize());
	   msg_token[lp_v5_max_msgsize()] = 0;
	   position += lp_v5_max_msgsize();
         }
      }
      else  /* we should split in backward mode */
      {
         /* first of all we should prepare message token */
         if (position <= lp_v5_max_msgsize())
         {
            /* this is the end of main message string */
	   last_message_part = True;
	   memcpy(msg_token, nmessage, position);
	   msg_token[position] = 0;
	   position = 0;
         }
         else
         {
	   memcpy(msg_token,nmessage+(position-lp_v5_max_msgsize()), lp_v5_max_msgsize());
	   msg_token[lp_v5_max_msgsize()] = 0;
	   position -= lp_v5_max_msgsize();
         }
      }
      
      curr_part_num += 1;
      
      /* here we should convert message type */
      
      mtype = convert_message_type(msg_hdr.mtype);
      
      /* now we can create & send to user message packet */
      reply_pack.clearPacket();
      reply_pack << (unsigned short)V5_PROTO
                 << (char)0x00
                 << (unsigned  long)to_user.session_id
	         << (unsigned short)ICQ_CMDxSND_SYSxMSGxONLINE
                 << (unsigned short)to_user.servseq
                 << (unsigned short)0x0000
                 << (unsigned  long)to_user.uin
	         << (unsigned  long)0x0000
	         << (unsigned  long)msg_hdr.fromuin
	         << (unsigned short)mtype
	         << (unsigned short)(strlen(msg_token)+1)
	         << msg_token;

      PutKey(reply_pack, calculate_checkcode(reply_pack));
      
      DEBUG(50, ("V5 ONmessage thru server (part %d of %d) from %lu to %lu\n", 
                  curr_part_num, parts_num, msg_hdr.fromuin, to_user.uin));

      /* sending packets to client, we need confirm! */
      v5_send_indirect(reply_pack, to_user.uin, to_user.shm_index);
      
      if (last_message_part) break;
   }
   
   delete [] msg_token;
   
   if (salloc==1) free(nmessage);
   return(parts_num);
}


/**************************************************************************/
/* Send offline message to user via V5 proto  			    	  */
/**************************************************************************/
int v5_send_user_sysmsg(struct msg_header &msg_hdr, struct online_user &to_user,
		        char *message)
{
   struct tm *sp_time;
   int salloc = 1;
   time_t rmess_time;
   unsigned short mtype = 0;
   unsigned short parts_num = 0;
   char *msg_token = new char[lp_v5_max_msgsize()+10];
   char *nmessage = NULL;
   unsigned short position = 0, last_part, curr_part_num = 0;
   BOOL last_message_part = False;
   
   nmessage = convert_message_text(msg_hdr.fromuin, msg_hdr.mtype, message);
   if (nmessage == NULL) { nmessage = message; salloc = 0; }

   parts_num = strlen(nmessage) / lp_v5_max_msgsize();
   if (strlen(nmessage) % lp_v5_max_msgsize() != 0) parts_num += 1;

   reply_pack.from_ip   = to_user.ip;
   reply_pack.from_port = to_user.udp_port;

   if (lp_v3_split_order() == ORDER_BACKWARD) position = strlen(nmessage);

   for (;;)
   {
      if (lp_v3_split_order() == ORDER_FORWARD)
      {
         /* first of all we should prepare message token    */
         if (abs(static_cast<int>(strlen(nmessage) - position)) <= lp_v5_max_msgsize())
         {
            /* this is the end of main message string */
	   last_part = strlen(nmessage) - position;
	   last_message_part = True;
	   memcpy(msg_token, nmessage+position, last_part);
	   msg_token[last_part] = 0;
	   position += last_part;
         }
         else
         {
	   memcpy(msg_token, nmessage+position, lp_v5_max_msgsize());
	   msg_token[lp_v5_max_msgsize()] = 0;
	   position += lp_v5_max_msgsize();
         }
      }
      else  /* we should split in backward mode */
      {
         /* first of all we should prepare message token */
         if (position <= lp_v5_max_msgsize())
         {
            /* this is the end of main message string */
	   last_message_part = True;
	   memcpy(msg_token, nmessage, position);
	   msg_token[position] = 0;
	   position = 0;
         }
         else
         {
	   memcpy(msg_token,nmessage+(position-lp_v5_max_msgsize()), lp_v5_max_msgsize());
	   msg_token[lp_v5_max_msgsize()] = 0;
	   position -= lp_v5_max_msgsize();
         }
      }
    
      curr_part_num += 1;
      
      /* ugh... :((( I don't know what to do with timezones... ICQ99a want   */
      /* time in UTC format, but unix gmtime doesn't work even after tszet() */
      /* call... */
      
      rmess_time = msg_hdr.mtime - serverzone;
      sp_time = gmtime(&rmess_time);

      /* here we should convert message type */ 
      mtype = convert_message_type(msg_hdr.mtype); 
               
      /* now we can create & send to user message packet */
      reply_pack.clearPacket();
      reply_pack << (unsigned short)V5_PROTO
                 << (char)0x00
                 << (unsigned  long)to_user.session_id
	         << (unsigned short)ICQ_CMDxSND_SYSxMSGxOFFLINE
                 << (unsigned short)to_user.servseq
                 << (unsigned short)0x0000
                 << (unsigned  long)to_user.uin
	         << (unsigned  long)0x0000
	         << (unsigned  long)msg_hdr.fromuin
                 << (unsigned short)((sp_time->tm_year)+1900)
                 << (char)((sp_time->tm_mon)+1)
                 << (char)(sp_time->tm_mday)
                 << (char)sp_time->tm_hour
                 << (char)sp_time->tm_min
	         << (unsigned short)mtype
	         << (unsigned short)(strlen(msg_token)+1)
	         << msg_token;

      PutKey(reply_pack, calculate_checkcode(reply_pack));
      
      DEBUG(50, ("V5 OFmessage thru server (part %d of %d) from %lu to %lu\n", 
                  curr_part_num, parts_num, msg_hdr.fromuin, to_user.uin));

      /* sending packets to client, we need confirm! */
      v5_send_indirect(reply_pack, to_user.uin, to_user.shm_index);
      
      if (last_message_part) break;
   }
   
   delete [] msg_token;
   
   if (salloc == 1) free(nmessage);
   return(parts_num);
	     
}


/**************************************************************************/
/* Send system offline messages to user from database     		  */
/**************************************************************************/
int v5_send_offline_messages(struct online_user &user)
{
   unsigned short mess_num;
   PGresult *res;
   fstring dbcomm_str; 
   struct msg_header msg_hdr;
   bzero((void *)&msg_hdr, sizeof(msg_hdr));
  
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, 
           "DECLARE mportal CURSOR FOR SELECT * FROM Users_Messages WHERE to_uin=%lu ORDER BY msg_date", 
	    user.uin);

   PQclear(PQexec(users_dbconn, "BEGIN;"));

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[V5 DECLARE PORTAL (msgs)]");
      return(0);
   }
   else
   {
      PQclear(res);
   }
        
   for (mess_num=0;;mess_num++)
   {
      res = PQexec(users_dbconn, "FETCH IN mportal");
      if (PQresultStatus(res) != PGRES_TUPLES_OK)
      {
         handle_database_error(res, "[V5 FETCH FROM PORTAL (msgs)]");
         break;
      }
  	 
      if (PQntuples(res) == 0)
      {
         PQclear(res);
         break;
      }

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
      msg_hdr.msglen    = strlen(msg_buff);

      v5_send_user_sysmsg(msg_hdr, user, msg_buff);

      PQclear(res);
   }
      
   PQclear(PQexec(users_dbconn, "CLOSE mportal"));
   PQclear(PQexec(users_dbconn, "END;"));
   v5_send_end_sysmsg(user);

   return (0);
}


/**************************************************************************/
/* System message packet handler 				    	  */
/**************************************************************************/
void v5_process_sysmsg()
{
  unsigned short pvers, pcomm, seq1, seq2, msg_type, msg_len, i;
  unsigned long  uin_num, to_uin, temp_stamp, session_id;
  struct online_user user; 
  struct msg_header msg_hdr;
  
  bzero((void *)&msg_hdr, sizeof(msg_hdr));
 
  int_pack.reset();
  int_pack >> pvers   >> temp_stamp
	   >> uin_num >> session_id;
	   
  if (db_online_lookup(uin_num, user) == 0)
  {
    if ((ipToIcq(user.ip) == ipToIcq(int_pack.from_ip)) &&
	(user.session_id == session_id) &&
	(user.udp_port == int_pack.from_port))
    {

      v5_send_ack(int_pack);
      
      int_pack.reset();
      int_pack >> pvers   >> temp_stamp
	       >> uin_num >> session_id
	       >> pcomm   >> seq1
               >> seq2    >> temp_stamp
	       >> to_uin  >> msg_type
	       >> msg_len;

      if (!islen_valid(msg_len, lp_v5_max_msgsize()+1, "message", user))
      {
  	 move_user_offline(user.uin);
	 v5_send_not_connected(int_pack);
	 return;
	 
      }
      else
      {
         for(i=0; i<msg_len; i++) int_pack >> msg_buff[i];
      }

      /* fill message header structure */
      msg_hdr.mtype     = msg_type;
      msg_hdr.touin     = to_uin;
      msg_hdr.fromuin   = uin_num;
      msg_hdr.seq2      = seq2;
      msg_hdr.from_ver  = V5_PROTO;
      msg_hdr.mtime     = timeToLong(time(NULL));
      msg_hdr.fromindex = user.shm_index;
      msg_hdr.msglen    = msg_len;

      process_message(msg_hdr, msg_buff);
            
    }
    else
    {
       LOG_ALARM(0, ("Spoofed msg thru server command from %s:%d (%lu)\n", 
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
/* V3<-->V5 message converter         				    	  */
/**************************************************************************/
unsigned short convert_message_type(unsigned short type)
{
      switch (type)
      {
         case 0x14: return(0x09);
         default:   return(type);
      
      }
}


/**************************************************************************/
/* V3<-->V5 message text converter    				    	  */
/**************************************************************************/
char *convert_message_text(unsigned long from_uin, unsigned short type, char *message)
{
   char *nmessage;
   switch (type)
   {
      case 0x14: 
      {
         if (from_uin > 1)
	 {
            nmessage = (char *)malloc(strlen(message)+128);
            snprintf(nmessage, 128, "ICQ User%s%s%snone%s3", CLUE_CHAR, CLUE_CHAR,
                     CLUE_CHAR, CLUE_CHAR);
            strncat(nmessage, message, strlen(message)+127);
	 
            return(nmessage);
	 }
      }
      break;
      default:   return(NULL);
   }
   
   return(NULL);
}


