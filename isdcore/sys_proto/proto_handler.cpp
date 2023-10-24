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
/* Handler for IServerd internal transport protocol			  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#include "vsys_defines.h"
#include "isdcore/v3_proto/v3_defines.h"

Packet sys_int_pack; Packet sys_reply_pack;
extern unsigned long increm_num;

/**************************************************************************/
/* Main system packet handlers selector 				  */
/* If packet can't be handled, selector write it to alarm log lev=(200)	  */
/**************************************************************************/
void handle_sys_proto(Packet &pack)
{
   unsigned short pvers, pcomm;
   sys_int_pack = pack;
  
   sys_int_pack.reset(); sys_int_pack >> pvers >> pcomm;
  
   /* We should process packet ONLY in socket-processor calls */
   if (process_role != ROLE_SOCKET) return;

   /* Well now we need to determine what we got... */
   switch (pcomm)
   {
      case ICQ_SYSxRCV_SYSINFO_REQ:  sys_process_info_req();  break;
      case ICQ_SYSxRCV_WWPMESS:	     sys_process_wwp_mess();  break;
      case ICQ_SYSxRCV_SYSBCAST_REQ: sys_process_broadcast(); break;
      case ICQ_SYSxRCV_DELUSER_REQ:  sys_process_deluser();   break;
   }
}


/**************************************************************************/
/* We should check password and set system information     		  */
/**************************************************************************/
void sys_process_broadcast()
{
   unsigned short pvers, pcomm, pass_len, mess_type;
   unsigned short mess_len, online_only;

   unsigned long bcast_uin = ADMIN_UIN, dep_num = 0, i = 0;
   char sys_pass[20];
   char bcmessage[2048];
   char bcmessage1[2048];
  
   sys_int_pack.reset();
   sys_int_pack >> pvers >> pcomm;
   sys_int_pack >> pass_len;
  
   /* We don't want overflow, isn't it? */
   if (pass_len > 20) return;
   for(i=0; i<pass_len; i++) sys_int_pack >> sys_pass[i];
  
   /* We can't accept default password */
   if (strcsequal(sys_pass, "DEFAULT")) { sys_wrong_passwd() ;return; }
   if (!strcsequal(sys_pass, lp_info_passwd())) { sys_wrong_passwd() ;return;}
   
   sys_int_pack >> online_only
                >> dep_num
                >> mess_type
	        >> mess_len;
	        
   if (mess_len > (sizeof(bcmessage)-1)) return;
   for (i=0; i<mess_len; i++) sys_int_pack >> bcmessage[i];
  
   snprintf(bcmessage1, 128, "ICQ Admin%s%s%s%s%s3%s", CLUE_CHAR, CLUE_CHAR, 
            CLUE_CHAR, lp_admin_email(), CLUE_CHAR, CLUE_CHAR);
  
   strncat(bcmessage1, bcmessage, 1500);
  
   /* now i'm going to create v3 broadcast packet with magic1&2 */
   sys_reply_pack.clearPacket();

   if (online_only)
   {
      sys_reply_pack << (unsigned short)V3_PROTO
		     << (unsigned short)ICQ_CMDxRCV_BROADCAST_MSG_ONL;
   }
   else
   {
      sys_reply_pack << (unsigned short)V3_PROTO
		     << (unsigned short)ICQ_CMDxRCV_BROADCAST_MSG_ALL;
   }
  
   sys_reply_pack   << ipc_vars->magic_num1
                    << ipc_vars->magic_num2
		    << (unsigned long)bcast_uin
		    << (unsigned long)0x00
		    << (unsigned long)dep_num
		    << mess_type
		    << (unsigned short)(strlen(bcmessage1)+1)
		    << bcmessage1;
                    
   sys_reply_pack.from_ip.s_addr = INADDR_LOOPBACK;
   sys_reply_pack.from_port = 4000;
   
   increm_num++;
   sys_reply_pack.from_local = 1;
   pipe_send_packet(sys_reply_pack);
    
   sys_reply_pack.clearPacket();
   sys_reply_pack << (unsigned short)SYS_PROTO
                  << (unsigned short)ICQ_SYSxSND_SYSBCAST_REP;

   sys_reply_pack.from_ip   = sys_int_pack.from_ip;
   sys_reply_pack.from_port = sys_int_pack.from_port;
   
   udp_send_direct_packet(sys_reply_pack);

   DEBUG(100, ("Admin has sent broadcast message...\n"));
}


/**************************************************************************/
/* We should check password and set system information     		  */
/**************************************************************************/
void sys_process_info_req()
{
   unsigned short pvers, pcomm, pass_len;
   char sys_pass[20];
  
   sys_int_pack.reset();
   sys_int_pack >> pvers >> pcomm;
   sys_int_pack >> pass_len;
  
   /* We don't want overflow */
   if (pass_len > 20) return;
   for(int i=0; i<pass_len; i++) sys_int_pack >> sys_pass[i];
  
   /* We can't accept default password */
   if (strcsequal(sys_pass, "DEFAULT")) { sys_wrong_passwd() ;return; }
   if (!strcsequal(sys_pass, lp_info_passwd())) { sys_wrong_passwd() ;return;}

   sys_reply_pack.clearPacket();
   sys_reply_pack 
       << (unsigned short)SYS_PROTO		   /* protocol id */
       << (unsigned short)ICQ_SYSxSND_SYSINFO_REP  /* command id  */
       << (unsigned  long)lp_udp_port()		   /* bind port   */
       << (unsigned  long)ipToIcq(bind_interface)  /* bind addr   */
       << (unsigned  long)server_started	   /* start time  */
       << (unsigned  long)config_loaded		   /* config time */
       << (unsigned  long)pack_processed	   /* processed   */
       << (unsigned  long)ipc_vars->pack_in_queue  /* in queue    */
       << (unsigned  long)ipc_vars->byte_in_queue  /* queue size  */
       << (unsigned  long)0x0			   /* used memory */
       << (unsigned  long)ipc_vars->max_queue_size
       << (unsigned  long)ipc_vars->max_queue_pack
       << (unsigned  long)ipc_vars->queue_send_errors
       << (unsigned  long)getpid()		   /* daemon pid  */
       << (unsigned  long)geteppid()		   /* ep pid      */
       << (unsigned  long)childs_num(ROLE_PACKET)  /* pp number   */
       << (unsigned  long)childs_num(ROLE_BUSY)	   /* bp number   */

       /* Now we should pass process list to server_status */
      
       << (unsigned short)(childs_num(-1)+1)       /* childs num +1 */
       << (unsigned  long)getpid()		   /* daemon pid    */
       << (unsigned short)ROLE_SOCKET		   /* it's role	    */
       << (unsigned  long)0x000000;		   /* it's size     */

       cl_list = cl_begin;
  
       while(cl_list)
       {      
          sys_reply_pack 
	    << (unsigned  long)cl_list->child.child_pid
	    << (unsigned short)cl_list->child.child_role
	    << (unsigned  long)0x000000;
	 
          cl_list = cl_list->next;
       }

       sys_reply_pack << ipc_vars->online_usr_num;

   sys_reply_pack.from_ip   = sys_int_pack.from_ip;
   sys_reply_pack.from_port = sys_int_pack.from_port;

   udp_send_direct_packet(sys_reply_pack);

   DEBUG(150, ("Admin ask for system info...\n"));
}


/**************************************************************************/
/* We should check password and send wwp message to pp			  */
/**************************************************************************/
void sys_process_wwp_mess()
{
   unsigned short pvers, pcomm, pass_len;
   char sys_pass[20];

   sys_int_pack.reset();
   sys_int_pack >> pvers >> pcomm;
   sys_int_pack >> pass_len;
  
   /* We don't want overflow, isn't it? */
   if (pass_len > 20) return;
   for(int i=0; i<pass_len; i++) sys_int_pack >> sys_pass[i];
   
   /* We can't accept default password */
   if (strcsequal(sys_pass, "DEFAULT")) { sys_wrong_passwd() ;return;}
   if (!strcsequal(sys_pass, lp_info_passwd())) { sys_wrong_passwd() ;return;}
}


/**************************************************************************/
/* We should check password and send disconnect user request		  */
/**************************************************************************/
void sys_process_deluser()
{
   unsigned short pvers, pcomm, pass_len;
   char sys_pass[20];

   sys_int_pack.reset();
   sys_int_pack >> pvers >> pcomm;
   sys_int_pack >> pass_len;
  
   /* We don't want overflow, isn't it? */
   if (pass_len > 20) return;
   for(int i=0; i<pass_len; i++) sys_int_pack >> sys_pass[i];
   
   /* We can't accept default password */
   if (strcsequal(sys_pass, "DEFAULT")) { sys_wrong_passwd() ;return;}
   if (!strcsequal(sys_pass, lp_info_passwd())) { sys_wrong_passwd() ;return;}
   sys_int_pack >> deluser;
   
   if (deluser != 0)
   {
      /* now time to disconnect requested user */
      sys_reply_pack.clearPacket();
      sys_reply_pack << (unsigned short)V3_PROTO
 		     << (unsigned short)ICQ_CMDxRCV_DELUSER_REQ
 		     << (unsigned long)deluser;
                    
      sys_reply_pack.from_ip.s_addr = INADDR_LOOPBACK;
      sys_reply_pack.from_port = 4000;
   
      increm_num++;
      sys_reply_pack.from_local = 1;
      pipe_send_packet(sys_reply_pack);
   }

   sys_reply_pack.clearPacket();
   sys_reply_pack << (unsigned short)SYS_PROTO
                  << (unsigned short)ICQ_SYSxSND_REQ_OK;

   sys_reply_pack.from_ip   = sys_int_pack.from_ip;
   sys_reply_pack.from_port = sys_int_pack.from_port;
   udp_send_direct_packet(sys_reply_pack);

   LOG_SYS(1, ("Admin sent disconnect user (%lu) request...\n", deluser));
  
}


/****************************************************************/
/* User sent incorrect password so we should alert him     	*/
/****************************************************************/
void sys_wrong_passwd()
{
   sys_reply_pack.clearPacket();
   sys_reply_pack 
       << (unsigned short)SYS_PROTO
       << (unsigned short)ICQ_SYSxSND_PASSWD_ERR;

   sys_reply_pack.from_ip   = sys_int_pack.from_ip;
   sys_reply_pack.from_port = sys_int_pack.from_port;
   
   udp_send_direct_packet(sys_reply_pack);

   LOG_ALARM(150, ("Admin sent wrong password...\n"));
}

