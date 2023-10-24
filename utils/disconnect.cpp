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
/* This utility used to get server information and status 	 	  */
/*									  */
/**************************************************************************/

#include "includes.h"

extern pstring debugf; 
extern pstring systemf; 
extern pstring alarmf;
extern pstring usersf;
extern BOOL append_log;

/**************************************************************************/
/* UTILITY: Get status info from server					  */
/**************************************************************************/
int main(int argc, char **argv)
{

  Packet pack;
  struct in_addr server_addr;
  unsigned short pvers, pcomm;

  process_role = ROLE_DUSER;

  init_globals();
  process_command_line_opt(argc, argv);
  pstring configf;
  slprintf(configf, sizeof(configf)-1, lp_config_file());

  /* Global parameters initialization */
  setup_usrlogging( "", True);
  slprintf(debugf,  sizeof(debugf), "/dev/null");
  setup_alrlogging( "", True);
  setup_syslogging( "", True);
  
  /* Loading configuration file */
  if (!lp_load(configf,False,False,True))
  {
    printf("ERROR: Can't find config: \"%s\"\n", configf);
    exit(EXIT_ERROR_CONFIG_OPEN);
  }

  /* open listening socket */  
  init_bindinterface();
  server_addr = bind_interface;
 
  bind_interface.s_addr = INADDR_ANY;
  udpserver_start(0, 1000);
  setNonBlocking(msockfd);
  
  /* send system request to get data */
  if (deluser == 0) exit(0);
  
  reply_pack.clearPacket();
  reply_pack << (unsigned short)SYS_PROTO
             << (unsigned short)ICQ_SYSxRCV_DELUSER_REQ
             << (unsigned short)(strlen(lp_info_passwd())+1)
             << lp_info_passwd()
	     << (unsigned long)deluser;
 
  reply_pack.from_ip   = server_addr;
  reply_pack.from_port = lp_udp_port();
  udp_send_direct_packet(reply_pack);
  msleep(50);
  
  /* Now time to check response - timeout 7 secs */
  for (int j=0; j<35; j++)
  {
     /* check if message was sent */
     if (udp_recv_pack(pack))
     {
         pack >> (pvers)
              >> (pcomm);	      

         if ((pvers == SYS_PROTO) &&
             (pcomm == ICQ_SYSxSND_REQ_OK))
         {
	    printf("Your request was processed by server...\n");
  	    exit(0);
         }
	 else
	 {
            if ((pvers == SYS_PROTO) &&
               (pcomm == ICQ_SYSxSND_PASSWD_ERR))
            {
	       printf("Can't send disconnect request\n");
	       printf("Server sent password error...\n");
	       exit(1);
	    }
	 }
      }

      msleep(200);
   }

   printf("Timeout. No response from server..\n");
   printf("Request not sent, check server and try again\n");
   
   /* save message to file to send again later */
   
   exit(2);
}

