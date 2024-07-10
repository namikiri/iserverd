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
/* This utility get data from file, specified in command line 		  */
/* and send it via system protocol to iserverd daemon as wwp message.	  */
/* Message text should be formated with 0xD 0xA as string delimiter 	  */
/*									  */
/* Data file format:							  */
/*   1st str:  target uin-number  (6218895)				  */
/*   2nd str:  from Email	  (AVShutko@mail.khstu.ru)		  */
/*   3rd str:  from Name	  (Regressor)				  */
/*   4th str:  from ip address 	  (10.10.10.2)				  */
/*   5th+ str: message body	  (texttexttext...)			  */
/*									  */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* program entry point							  */
/**************************************************************************/
int main(int argc, char **argv)
{
  char fname[128];		/* data filename    */
  int dfhdl;			/* filename handler */
  int i, pos;

  unsigned long wwp_target_uin;	/* target uin (from data_file)     */
  char wwp_tuin[32];		/* target uin string from df	   */
  char wwp_email[128];		/* email string (from data file)   */
  char wwp_name[128];		/* sender name (from data file)    */
  char wwp_ip[32];		/* ip address from data file	   */
  unsigned long wwp_ip_addr;	/* sender ip address (from df)     */
  char wwp_message[450];	/* message - limited to 450 chars  */
  char file_buff[2048];		/* file can't be greater than 2048 */
  long read_len;		/* length of file */
  char **valid;

  /* check if we have called without parameters */
  if (argc < 2)
  {
    printf("Usage: %s datafilename\n", argv[0]);
    printf("Data file example:\n");
    printf("------------------------------------\n");
    printf("6218895\n");
    printf("Sender@myhost.org\n");
    printf("SenderName\n");
    printf("from.myhost.org\n");
    printf("This is the first string of message\n");
    printf("This is the second one\n");
    printf("------------------------------------\n");
    exit(EXIT_NORMAL);
  }

  /* open file for reading only */
  strncpy(fname, argv[1], sizeof(fname)-1);
  dfhdl = open(fname, O_RDONLY);

  if (dfhdl < 0)
  {
    LOG_SYS(10, ("Can't open wwp message data file for reading...\n"));
    exit(EXIT_ERROR_FATAL);
  }

  /* we don't want anybody to read this file, so we should delete
     it quickly */
  if (unlink(fname) < 0)
  {
     LOG_ALARM(10, ("Can't unlink wwp message data file...\n"));
  }

  /* now it is time to parse file and send data to iserverd */
  bzero(file_buff, 2048);
  read_len = read(dfhdl, file_buff, 2048);

  /* first of all we should interpret uin number at first string... */
  for (i=0; i<33; i++) if (file_buff[i] == '\n') break;
  if (i == 32) { LOG_ALARM(0, ("Uin field in datafile too big...\n")); exit(EXIT_ERROR_FATAL);}
  memcpy(wwp_tuin, file_buff, i); wwp_tuin[i] = 0;

  valid = NULL; pos = i+1;
  wwp_target_uin = strtoul(wwp_tuin, valid, 10);
  if (wwp_target_uin < 1001)
  {
     LOG_ALARM(0, ("Uin field in datafile contain incorrect symbols...\n"));
     exit(EXIT_ERROR_FATAL);
  }

  for (i=pos; i<pos+129; i++) if (file_buff[i] == '\n') break;
  if (i == pos+128) { LOG_ALARM(0, ("Email field in datafile too big...\n")); exit(EXIT_ERROR_FATAL);}
  memcpy(wwp_email, file_buff+pos, i-pos); wwp_email[i-pos] = 0;
  pos = i+1;

  for (i=pos; i<pos+65; i++) if (file_buff[i] == '\n') break;
  if (i == pos+64) { LOG_ALARM(0, ("Name field in datafile too big...\n")); exit(EXIT_ERROR_FATAL);}
  memcpy(wwp_name, file_buff+pos, i-pos); wwp_name[i-pos] = 0;
  pos = i+1;

  for (i=pos; i<pos+33; i++) if (file_buff[i] == '\n') break;
  if (i == pos+32) { LOG_ALARM(0, ("IP field in datafile too big...\n")); exit(EXIT_ERROR_FATAL);}
  memcpy(wwp_ip, file_buff+pos, i-pos); wwp_ip[i-pos] = 0;

  wwp_ip_addr = interpret_addr(wwp_ip); pos = i + 1;

  for (i=pos; i<pos+451; i++) if (file_buff[i] == '\0') break;
  if (i == pos+450) { LOG_ALARM(0, ("Text field in datafile too big...\n")); exit(EXIT_ERROR_FATAL);}
  memcpy(wwp_message, file_buff+pos, read_len - pos); wwp_message[read_len - pos] = 0;

  /* send data to special socket */
  reply_pack.clearPacket();
  reply_pack << (unsigned  long)0x01020709
             << (unsigned  long)wwp_ip_addr
	     << (unsigned  long)wwp_target_uin
	     << (unsigned short)(strlen(wwp_email)+1)
	     << wwp_email
	     << (unsigned short)(strlen(wwp_name)+1)
	     << wwp_name
	     << (unsigned short)(strlen(wwp_message)+1)
	     << wwp_message;

  if ((wwpsockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) exit(EXIT_ERROR_FATAL);
  bzero((char *)&wp_serv_addr, sizeof(wp_serv_addr));
  wp_serv_addr.sun_family = AF_UNIX;
  snprintf(wp_serv_addr.sun_path, sizeof(wp_serv_addr.sun_path)-1,
           "/tmp/wwp_sock");
  wp_saddrlen = sizeof(wp_serv_addr.sun_family) +
                       strlen(wp_serv_addr.sun_path) + 1;

  /* send packet to pipe socket */
  sendto(wwpsockfd, reply_pack.buff, reply_pack.sizeVal, 0,
          (struct sockaddr *)&wp_serv_addr, wp_saddrlen);

  shutdown(wwpsockfd, 2);
  close(wwpsockfd);

  printf("sent");
}

