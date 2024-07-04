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

  TServinfo sinfo;
  Packet pack;
  struct in_addr server_addr;
  unsigned short pvers, pcomm;
  unsigned short pcount, prrole;
  unsigned long  prsize, prpid, users_num;
  char sprole[32];

  process_role = ROLE_STATUS;

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
  reply_pack.clearPacket();
  reply_pack << (unsigned short)SYS_PROTO
             << (unsigned short)ICQ_SYSxRCV_SYSINFO_REQ
             << (unsigned short)(strlen(lp_info_passwd())+1)
             << lp_info_passwd();

  reply_pack.from_ip   = server_addr;
  reply_pack.from_port = lp_udp_port();
  udp_send_direct_packet(reply_pack);
  msleep(50);

  /* Now time to get response - timeout 7 secs */
  for (int j=0; j<35; j++)
  {
     /* check for server status */
     if (udp_recv_pack(pack))
     {
         sinfo.lutime = time(NULL);
         pack >> (pvers)
              >> (pcomm);

         if ((pvers == SYS_PROTO) &&
            (pcomm == ICQ_SYSxSND_SYSINFO_REP))
         {
            pack >> (sinfo.bind_port)
                 >> (sinfo.bind_addr)
   	         >> (sinfo.server_started)
	         >> (sinfo.config_loaded)
	         >> (sinfo.pack_processed)
	         >> (sinfo.pack_in_queue)
	         >> (sinfo.queue_size)
	         >> (sinfo.used_memory)
	         >> (sinfo.max_queue_size)
	         >> (sinfo.max_queue_pack)
	         >> (sinfo.queue_send_errors)
	         >> (sinfo.sp_pid)
	         >> (sinfo.ep_pid)
	         >> (sinfo.pp_num)
	         >> (sinfo.bp_num)
	         >> (pcount);

            if (!lp_proclist())
	    {

               for (int i=0; i<pcount; i++)
	       {
	          pack >> prpid
	               >> prrole
	               >> prsize;
	       }

	       pack >> users_num;

               printf("Server status:     Online\n");
               printf("Bind address:      %s\n", inet_ntoa(icqToIp(sinfo.bind_addr)));
               printf("UDP bind port:     %lu\n", sinfo.bind_port);
               printf("Server started:    %s\n", time2str1(sinfo.server_started));
               printf("Config loaded:     %s\n", time2str1(sinfo.config_loaded));
	       printf("Users connected:   %lu\n", users_num);
               printf("-----------------------------------------\n");
               printf("Pack processed:    %lu\n", sinfo.pack_processed);
               printf("Pack in queue:     %lu\n", sinfo.pack_in_queue);
               printf("Queue size:        %lu bytes\n", sinfo.queue_size);
               printf("Max Queue size:    %lu bytes\n", sinfo.max_queue_size);
               printf("Max Queue pack:    %lu\n", sinfo.max_queue_pack);
               printf("Queue send error:  %lu\n", sinfo.queue_send_errors);
               /* printf("Used memory:       %lu\n", sinfo.used_memory); */
               printf("-----------------------------------------\n");
               printf("SP (main)  pid:    %lu\n", sinfo.sp_pid);
               printf("Number of PPS:     %lu\n", sinfo.pp_num);
            }
	    else
	    {
	       printf("Number of processes: %d\n", pcount);
	       printf("-----------------------------------\n");
               for (int i=0; i<pcount; i++)
	       {
	          pack >> prpid
	               >> prrole
	               >> prsize;

                  switch (prrole)
                  {
                     case ROLE_PACKET : strncpy(sprole, "[packet  ]", 31); break;
                     case ROLE_SOCKET : strncpy(sprole, "[socket  ]", 31); break;
                     case ROLE_EPACKET: strncpy(sprole, "[epacket ]", 31); break;
                     case ROLE_ETIMER : strncpy(sprole, "[etimer  ]", 31); break;
                     case ROLE_EUSER  : strncpy(sprole, "[euser   ]", 31); break;
                     case ROLE_DEFRAG : strncpy(sprole, "[defrag  ]", 31); break;
                     case ROLE_ACTIONS: strncpy(sprole, "[actions ]", 31); break;
                     case ROLE_BUSY   : strncpy(sprole, "[busy    ]", 31); break;
                     case ROLE_PAUSED : strncpy(sprole, "[paused  ]", 31); break;
                     case ROLE_EMPTY  : strncpy(sprole, "[empty   ]", 31); break;
                     default: strncpy(sprole,           "[unknown ]", 31); break;
                  }

		  printf(" ppid=%lu\tprole=%d\t%s\n", prpid, prrole, sprole);

               }

	       printf("\n");
            }

	    exit(0);
         }
	 else
	 {
            if ((pvers == SYS_PROTO) &&
               (pcomm == ICQ_SYSxSND_PASSWD_ERR))
            {
	       printf("Can't get server status.\n");
	       printf("Server sent password error...\n");
	       exit(1);
	    }
	 }
      } /* if (udp_receive) */

      msleep(200);

   }

   printf("Timeout. No response from server...\n");
   exit(2);
}

