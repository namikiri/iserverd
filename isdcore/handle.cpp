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
/* Incoming packets handling 						  */
/*                                                                        */
/**************************************************************************/


#include "includes.h"


/**************************************************************************/
/* This func called by packet_processors. It make protocol switching	  */
/* between specific protocol handlers, if protocol is not supported it 	  */
/* dump to log notice message with level 10 				  */
/**************************************************************************/
void handle_icq_packet(Packet &pack)
{
   short unsigned int prot_vers;
   short unsigned int command_num;

   /* first check if we have OSCAR packet */ 
   if (pack.sock_type == SAIM) 
   {
      handle_aim_proto(pack);
      return;
   }

   /* is this a msn packet ? [not handled yet] */
   if (pack.sock_type == SMSN) 
   {
      return;
   }
   
   /* now we should try to determine packet proto version. When you 
   adding new proto - you should add case string bellow with proper 
   proto handler. */
   
   /* get some data from packet and reset it to begin  */
   pack >> prot_vers;		/* packet version	     */
   pack >> command_num;		/* command code		     */
   pack.reset();

   switch (prot_vers)
   {
      case V2_PROTO:  NO_PROTO_STUB(V2_PROTO); break;
      case V3_PROTO:  handle_v3_proto(pack);   break;
      case V4_PROTO:  NO_PROTO_STUB(V4_PROTO); break;
      case V5_PROTO:  handle_v5_proto(pack);   break;
      case SYS_PROTO: break;
   
      default: 	      NO_PROTO_STUB(prot_vers);

   }
}


/**************************************************************************/
/* This func make initialization of all available protocols. If you are	  */
/* developing new proto - insert your init funct here.			  */
/**************************************************************************/
void init_all_protocols()
{
   v3_proto_init();   /* Initialization of V3 proto variables */
   v5_proto_init();   /* Initialization of V5 proto variables */  
   v7_proto_init();   /* Initialization of V7 proto variables */
}


/**************************************************************************/
/* This function select proper function to send user_online packet 	  */
/**************************************************************************/
void send_user_online(struct online_user &to_user, struct online_user &user)
{
   switch (to_user.protocol)
   {
      case V3_PROTO: v3_send_user_online( to_user, user ); break;
      case V4_PROTO: return;
      case V5_PROTO: v5_send_user_online( to_user, user ); break;
      case V7_PROTO: v7_send_user_online( to_user, user ); break;
      
      default: return;
   }
}


/**************************************************************************/
/* This function used to disconnect specific user			  */
/**************************************************************************/
void disconnect_user(struct online_user *user)
{
   if (user != NULL)
   {
      DEBUG(150, ("Disconnect user %lu called\n", user->uin));
    
      switch (user->protocol)
      {
         case V3_PROTO: v3_disconnect_user(user); break;
         case V4_PROTO: return;
         case V5_PROTO: v5_disconnect_user(user); break;
         case V7_PROTO: v7_disconnect_user(user); break;
         default: return;
      }
   }
}


/**************************************************************************/
/* This function select proper function to send user_ch_status packet	  */
/**************************************************************************/
void send_user_status(struct online_user &to_user, struct online_user &user)
{
   switch (to_user.protocol)
   {
      case V3_PROTO: v3_send_user_status(to_user, user); break;
      case V4_PROTO: return;
      case V5_PROTO: v5_send_user_status(to_user, user); break;
      case V7_PROTO: v7_send_user_status(to_user, user); break;
      default: return;
   }
}


/************************************************************************/
/* This function select proper function to send user_offline packet 	*/
/************************************************************************/
void send_user_offline(struct online_user &to_user, unsigned long uin)
{
   switch (to_user.protocol)
   {
      case V3_PROTO: v3_send_user_offline(to_user, uin); break;
      case V4_PROTO: return;
      case V5_PROTO: v5_send_user_offline(to_user, uin); break;
      case V7_PROTO: v7_send_user_offline(to_user, uin); break;
      default: return;
   }
}


/**************************************************************************/
/* This func send protocol independent offline message to user	      	  */
/**************************************************************************/
void send_online_message(struct msg_header &msg_hdr, struct online_user &to_user, 
                         char *message)
{
   struct full_user_info userinfo;

   DEBUG(100, ("Sending system message (type: %d) to user %lu from %lu\n", 
                msg_hdr.mtype, to_user.uin, msg_hdr.fromuin));

   DEBUG(50, ("Msg: '%s'\n", message));

   /* check for empty "you were added" - miranda lasy programmers bug */
   if ((strlen(message) < 1) &&
       (msg_hdr.mtype == 12))
   {
      DEBUG(100, ("Empty you were added message from %lu\n", msg_hdr.fromuin));

      bzero((void *)&userinfo, sizeof(userinfo));
      if (db_users_lookup_short(msg_hdr.fromuin, userinfo) == 0)
      {
         snprintf(message, 128, "%s%s%s%s%s%s%s%s1", userinfo.nick, CLUE_CHAR,
	          userinfo.first, CLUE_CHAR, userinfo.last, CLUE_CHAR, 
		  userinfo.email1, CLUE_CHAR);
      }
      else
      {
         snprintf(message, 128, "-%s-%s-%s-%s1", 
                  CLUE_CHAR, CLUE_CHAR, CLUE_CHAR, CLUE_CHAR);
      }
   }

   switch (to_user.protocol)
   {
      case V3_PROTO: v3_send_user_message(msg_hdr, to_user, message); break;
      case V4_PROTO: return;
      case V5_PROTO: v5_send_user_message(msg_hdr, to_user, message); break;
      case V7_PROTO: v7_send_user_message(msg_hdr, to_user, message); break;
      default: return;
   }    
}


/**************************************************************************/
/* To send disconnect packet to client with disconnect reason string   	  */
/**************************************************************************/
void send_user_disconnect(struct online_user &user, unsigned short err_code, 
		          char *err_string)
{
   switch (user.protocol)
   {
      case V2_PROTO: return;      
      case V3_PROTO: v3_send_srv_disconnect(user, err_string); return;
      case V4_PROTO: return;
      case V5_PROTO: v5_send_srv_disconnect(user); return;
      case V7_PROTO: v7_send_srv_disconnect(user, err_code, err_string); return;
      default: return;
   }    
}


/**************************************************************************/
/* This func return max user timeout for specified protocol version	  */
/**************************************************************************/
int get_ping_time(int protocol)
{
   switch (protocol)
   {
      case V3_PROTO: return(lp_v3_pingtime());
      case V4_PROTO: return(lp_default_ping());
      case V5_PROTO: return(lp_v5_pingtime());
      case V7_PROTO: return(0); /* not applicable */
      
      default: return(lp_default_ping()); break;
   }
}


/**************************************************************************/
/* Send message to user or adds it to database			      	  */
/**************************************************************************/
void process_message(struct msg_header &msg_hdr, char *message)
{
   struct online_user to_user;

   if ((db_online_lookup(msg_hdr.touin, to_user) == 0) &&
       (to_user.active == 1))
   {
      DEBUG(150, ("ONMessage (%d) from user %lu to user %lu (%d bytes)\n", msg_hdr.mtype, 
                  msg_hdr.fromuin, msg_hdr.touin, strlen(message)));
 
      send_online_message(msg_hdr, to_user, message);
   } 
   else 
   {
      DEBUG(150, ("OFMessage (%d) from user %lu to user %lu (%d bytes)\n", msg_hdr.mtype, 
                  msg_hdr.fromuin, msg_hdr.touin, strlen(message)));

      db_add_message(msg_hdr, message);
   }
}


