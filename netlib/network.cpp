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
/* Some network usefull functions 					  */
/*									  */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* interpret an internet addr or name into an IP address in 4 byte form   */
/**************************************************************************/
uint32 interpret_addr(char *str)
{
   struct hostent *hp;
   uint32 res;
   int i;
   BOOL pure_address = True;

   if (strcmp(str, "0.0.0.0") == 0) return (0);
   if (strcmp(str, "255.255.255.255") == 0) return (0xFFFFFFFF);

   for (i = 0; pure_address && str[i]; i++)
   if (!(isdigit((int)str[i]) || str[i] == '.')) pure_address = False;

   /* if it's in the form of an IP address then
      get the lib to interpret it */
   if (pure_address)
   {
     res = inet_addr(str);
   }
   else
   {
       /* otherwise assume it's a network name of some sort and use
          Get_Hostbyname */
      if ((hp = Get_Hostbyname(str)) == 0)
      {
         DEBUG(3, ("Get_Hostbyname: Unknown host. %s\n", str));
         return 0;
      }

      if (hp->h_addr == NULL)
      {
         DEBUG(3,
	      ("Get_Hostbyname: host address is invalid for host %s\n",
	       str));
	 return 0;
      }

      putip((char *)&res, (char *)hp->h_addr);
   }

   if (res == (uint32)-1)  return (0);
   return (res);
}


/**************************************************************************/
/* A convenient addition to interpret_addr()				  */
/**************************************************************************/
struct in_addr *interpret_addr2(char *str)
{
   static struct in_addr ret;
   uint32 a = interpret_addr(str);
   ret.s_addr = a;
   return (&ret);
}


/**************************************************************************/
/* check if an IP is the 0.0.0.0					  */
/**************************************************************************/
BOOL zero_ip(struct in_addr ip)
{
   uint32 a;
   putip((char *)&a, (char *)&ip);
   return (a == 0);
}


/**************************************************************************/
/* are two IPs on the same subnet?					  */
/**************************************************************************/
BOOL same_net(struct in_addr ip1, struct in_addr ip2, struct in_addr mask)
{
   uint32 net1, net2, nmask;
   nmask = ntohl(mask.s_addr);
   net1 = ntohl(ip1.s_addr);
   net2 = ntohl(ip2.s_addr);

   return ((net1 & nmask) == (net2 & nmask));
}


/**************************************************************************/
/* a wrapper for gethostbyname() that tries with all lower and 		  */
/* all upper case if the initial name fails				  */
/**************************************************************************/
struct hostent *Get_Hostbyname(const char *name)
{
   char *name2 = strdup(name);
   struct hostent *ret;

   if (!isalnum(*name2))
   {
      free(name2);
      return (NULL);
   }

   ret = gethostbyname(name2);
   if (ret != NULL)
   {
      free(name2);
      return (ret);
   }

   /* try with all lowercase */
   strlower(name2);
   ret = gethostbyname(name2);
   if (ret != NULL)
   {
      free(name2);
      return (ret);
   }

   /* try with all uppercase */
   strupper(name2);
   ret = gethostbyname(name2);
   if (ret != NULL)
   {
      free(name2);
      return (ret);
   }

   /* nothing works */
   free(name2);
   return (NULL);
}

