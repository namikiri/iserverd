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
/* This utility used to send server broadcast 				  */
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
  unsigned short online_only;
  char bc_full_message[1023];

  init_globals();
  process_command_line_opt(argc, argv);
  pstring configf;
  slprintf(configf, sizeof(configf)-1, lp_config_file());

  /* Global parameters initialization */
  setup_usrlogging( "", True );
  slprintf(debugf,  sizeof(debugf), "/dev/null");
  setup_alrlogging( "", True );
  setup_syslogging( "", True );

  /* Loading configuration file */
  if (!lp_load(configf,False,False,True))
  {
    printf("ERROR: Can't find config: \"%s\"\n", configf);
    exit(EXIT_ERROR_CONFIG_OPEN);
  }

  /* open listening socket */
  init_bindinterface();
  init_translate();
  server_addr = bind_interface;

  bind_interface.s_addr = INADDR_ANY;
  udpserver_start(0, 1000);
  setNonBlocking(msockfd);

  /* -[ Get message from user ]-------------------------------------- */
  char flag[16];
  char subject[64];
  char message[350];

  fgets(flag, sizeof(flag)-1, stdin);
  online_only = atoi(flag);
  fgets(subject, sizeof(subject)-1, stdin);
  fread(message, sizeof(message)-1, 1, stdin);

  snprintf(bc_full_message, sizeof(bc_full_message)-1, "%s%s%s",
           subject, CLUE_CHAR, message);

  /* ---------------------------------------------------------------- */
  /* prepare standart system header with password */
  reply_pack.clearPacket();
  reply_pack << (unsigned short)SYS_PROTO
             << (unsigned short)ICQ_SYSxRCV_SYSBCAST_REQ
             << (unsigned short)(strlen(lp_info_passwd())+1)
             << lp_info_passwd();

  /* now time to put bcast message to packet */
  reply_pack << (unsigned short)online_only
	     << (unsigned  long)0x00
	     << (unsigned short)0x14
	     << (unsigned short)(strlen(bc_full_message)+1)
	     << bc_full_message;

  /* And send packet to server */
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
             (pcomm == ICQ_SYSxSND_SYSBCAST_REP))
         {
	    printf("Broadcast message sent successfully...\n");
  	    exit(0);
         }
	 else
	 {
            if ((pvers == SYS_PROTO) &&
               (pcomm == ICQ_SYSxSND_PASSWD_ERR))
            {
	       printf("Can't send broadcast message\n");
	       printf("Server sent password error...\n");
	       exit(1);
	    }
	 }
      }

      msleep(200);
   }

   printf("Timeout. No response from server..\n");
   printf("Message not sent, check server and try again\n");

   /* save message to file to send again later */

   exit(2);
}

