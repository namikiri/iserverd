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
/* Interfaces detecting and probing					  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

#define MAX_INTERFACES 128

static struct iface_struct *probed_ifaces;
static int total_probed;

struct in_addr ipzero;
struct in_addr allones_ip;
struct in_addr loopback_ip;

static struct interface *local_interfaces  = NULL;

#define ALLONES  ((uint32)0xFFFFFFFF)
#define MKBCADDR(_IP, _NM) ((_IP & _NM) | (_NM ^ ALLONES))
#define MKNETADDR(_IP, _NM) (_IP & _NM)

/**************************************************************************/
/* Try and find an interface that matches an ip. If we can't, return NULL */
/**************************************************************************/
static struct interface *iface_find(struct in_addr ip)
{
   struct interface *i;
   if (zero_ip(ip)) return local_interfaces;

   for (i=local_interfaces;i;i=i->next)
        if (same_net(i->ip,ip,i->nmask)) return i;

   return NULL;
}


/**************************************************************************/
/* Add an interface to the linked list of interfaces			  */
/**************************************************************************/
static void add_interface(struct in_addr ip, struct in_addr nmask)
{
   struct interface *iface;
   if (iface_find(ip))
   {
      DEBUG(3,("Not adding duplicate interface %s\n",inet_ntoa(ip)));
      return;
   }

   if (ip_equal(nmask, allones_ip))
   {
      DEBUG(3,("Not adding non-broadcast interface %s\n",inet_ntoa(ip)));
      return;
   }

   iface = (struct interface *)malloc(sizeof(*iface));
   if (!iface) return;
	
   ZERO_STRUCTPN(iface);

   iface->ip = ip;
   iface->nmask = nmask;
   iface->bcast.s_addr = MKBCADDR(iface->ip.s_addr, iface->nmask.s_addr);

   DLIST_ADD(local_interfaces, iface);

}


/**************************************************************************/
/* Interpret a single element from a interfaces= config line 		  */
/**************************************************************************/
static void interpret_interface(char *token)
{
   struct in_addr ip, nmask;
   char *p;
   int i, added=0;

   ip = ipzero;
   nmask = ipzero;
	
   for (i=0;i<total_probed;i++)
   {
      if (ms_fnmatch(token, probed_ifaces[i].name) == 0)
      {
         add_interface(probed_ifaces[i].ip,
	               probed_ifaces[i].netmask);
         added = 1;
      }
   }

   if (added) return;

   /* maybe it is a DNS name */
   p = strchr(token,'/');
   if (!p)
   {
      ip = *interpret_addr2(token);
      for (i=0;i<total_probed;i++)
      {
         if (ip.s_addr == probed_ifaces[i].ip.s_addr &&
            !ip_equal(allones_ip, probed_ifaces[i].netmask))
         {
            add_interface(probed_ifaces[i].ip, probed_ifaces[i].netmask);
            return;
         }
      }

      return;
   }

   /* parse it into an IP address/netmasklength pair */
   *p++ = 0;

   ip = *interpret_addr2(token);

   if (strlen(p) > 2)
   {
      nmask = *interpret_addr2(p);
   }
   else
   {
      nmask.s_addr = htonl(((ALLONES >> atoi(p)) ^ ALLONES));
   }

   /* maybe the first component was a broadcast address */
   if (ip.s_addr == MKBCADDR(ip.s_addr, nmask.s_addr) ||
       ip.s_addr == MKNETADDR(ip.s_addr, nmask.s_addr))
   {
      for (i=0;i<total_probed;i++)
      {
         if (same_net(ip, probed_ifaces[i].ip, nmask))
         {
            add_interface(probed_ifaces[i].ip, nmask);
            return;
         }
      }

      DEBUG(2,("Can't determine ip for broadcast address %s\n", token));
      return;
   }

   add_interface(ip, nmask);
}


/**************************************************************************/
/* load the list of network interfaces					  */
/**************************************************************************/
void load_interfaces(void)
{
   char *ptr;
   int i;
   struct iface_struct ifaces[MAX_INTERFACES];

   ptr = lp_interface();

   ipzero      = *interpret_addr2("0.0.0.0");
   allones_ip  = *interpret_addr2("255.255.255.255");
   loopback_ip = *interpret_addr2("127.0.0.1");

   if (probed_ifaces)
   {
      free(probed_ifaces);
      probed_ifaces = NULL;
   }

   /* dump the current interfaces if any */
   while (local_interfaces)
   {
      struct interface *iface = local_interfaces;
      DLIST_REMOVE(local_interfaces, local_interfaces);
      ZERO_STRUCTPN(iface);
      free(iface);
   }

   /* probe the kernel for interfaces */
   total_probed = get_interfaces(ifaces, MAX_INTERFACES);

   if (total_probed > 0)
   {
      probed_ifaces = (iface_struct *)memdup(ifaces, sizeof(ifaces[0])*total_probed);
   }

   /* if we don't have a interfaces line then use all broadcast capable 
      interfaces except loopback */
   if (!ptr || !*ptr)
   {
      if (total_probed <= 0)
      {
         LOG_SYS(0,("ERROR: Could not determine network interfaces, you must use a interfaces config line\n"));
         exit(1);
      }

      for (i=0;i<total_probed;i++)
      {
         if (probed_ifaces[i].netmask.s_addr != allones_ip.s_addr &&
	     probed_ifaces[i].ip.s_addr != loopback_ip.s_addr)
	 {
	     add_interface(probed_ifaces[i].ip, 
	                   probed_ifaces[i].netmask);
         }
      }

      return;
   }

   interpret_interface(ptr);

   if (!local_interfaces)
   {
      LOG_SYS(0,("WARNING: no network interfaces found\n"));
   }
}


/**************************************************************************/
/* Return True if the list of probed interfaces has changed		  */
/**************************************************************************/
BOOL interfaces_changed(void)
{
   int n;
   struct iface_struct ifaces[MAX_INTERFACES];
   n = get_interfaces(ifaces, MAX_INTERFACES);

   if (n != total_probed ||
       memcmp(ifaces, probed_ifaces, sizeof(ifaces[0])*n))
   {
      return True;
   }
	
   return False;
}


/**************************************************************************/
/*  check if an IP is one of mine					  */
/**************************************************************************/
BOOL ismyip(struct in_addr ip)
{
   struct interface *i;
   for (i=local_interfaces;i;i=i->next)
       if (ip_equal(i->ip,ip)) return True;

   return False;
}


/**************************************************************************/
/*  check if a packet is from a local (known) net			  */
/**************************************************************************/
BOOL is_local_net(struct in_addr from)
{
   struct interface *i;
   for (i=local_interfaces;i;i=i->next)
   {
      if((from.s_addr & i->nmask.s_addr) == 
         (i->ip.s_addr & i->nmask.s_addr))
         return True;
   }

   return False;
}


/**************************************************************************/
/*  how many interfaces do we have					  */
/**************************************************************************/
int iface_count(void)
{
   int ret = 0;
   struct interface *i;

   for (i=local_interfaces;i;i=i->next)	ret++;

   return ret;
}


/**************************************************************************/
/* True if we have two or more interfaces.				  */
/**************************************************************************/
BOOL we_are_multihomed(void)
{
   static int multi = -1;

   if(multi == -1)
   multi = (iface_count() > 1 ? True : False);
	
   return multi;
}


/**************************************************************************/
/* Return the Nth interface						  */
/**************************************************************************/
struct interface *get_interface(int n)
{ 
   struct interface *i;  
   for (i=local_interfaces;i && n;i=i->next) n--;

   if (i) return i;

   return NULL;
}


/**************************************************************************/
/* Return IP of the Nth interface					  */
/**************************************************************************/
struct in_addr *iface_n_ip(int n)
{
   struct interface *i;
  
   for (i=local_interfaces;i && n;i=i->next)  n--;

   if (i) return &i->ip;

   return NULL;
}


/**************************************************************************/
/* Return bcast of the Nth interface					  */
/**************************************************************************/
struct in_addr *iface_n_bcast(int n)
{
   struct interface *i;
  
   for (i=local_interfaces;i && n;i=i->next) n--;

   if (i) return &i->bcast;

   return NULL;
}


/**************************************************************************/
/* this function provides a simple hash of the configured interfaces. It  */
/* is used to detect a change in interfaces to tell us whether to discard */
/* the current wins.dat file.						  */
/* Note that the result is independent of the order of the interfaces	  */
/**************************************************************************/
unsigned iface_hash(void)
{
   unsigned ret = 0;
   struct interface *i;

   for (i=local_interfaces;i;i=i->next)
   {
      unsigned x1 = (unsigned)str_checksum(inet_ntoa(i->ip));
      unsigned x2 = (unsigned)str_checksum(inet_ntoa(i->nmask));
      ret ^= (x1 ^ x2);
   }

   return ret;
}


/**************************************************************************/
/* these 3 functions return the ip/bcast/nmask for the interface	  */
/* most appropriate for the given ip address. If they can't find	  */
/* an appropriate interface they return the requested field of the	  */
/* first known interface. 						  */
/**************************************************************************/
struct in_addr *iface_bcast(struct in_addr ip)
{
   struct interface *i = iface_find(ip);
   return(i ? &i->bcast : &local_interfaces->bcast);
}


struct in_addr *iface_ip(struct in_addr ip)
{
   struct interface *i = iface_find(ip);
   return(i ? &i->ip : &local_interfaces->ip);
}


int get_interfaces(struct iface_struct *ifaces, int max_interfaces)
{  
   struct ifconf ifc;
   char buff[8192];
   int fd, i, n;
   struct ifreq *ifr=NULL;
   int total = 0;
   struct in_addr ipaddr;
   struct in_addr nmask;
   char *iname;

   if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
   {
      return -1;
   }
  
   ifc.ifc_len = sizeof(buff);
   ifc.ifc_buf = buff;

   if (ioctl(fd, SIOCGIFCONF, &ifc) != 0)
   {
      close(fd);
      return -1;
   } 

   ifr = ifc.ifc_req;
  
   n = ifc.ifc_len / sizeof(struct ifreq);

   /* Loop through interfaces, looking for given IP address */
   for (i=n-1;i>=0 && total < max_interfaces;i--)
   {
      if (ioctl(fd, SIOCGIFADDR, &ifr[i]) != 0)
      {
          continue;
      }

      iname = ifr[i].ifr_name;
      ipaddr = (*(struct sockaddr_in *)&ifr[i].ifr_addr).sin_addr;

      if (ioctl(fd, SIOCGIFFLAGS, &ifr[i]) != 0)
      {
         continue;
      }  

      if (!(ifr[i].ifr_flags & IFF_UP))
      {
         continue;
      }

      if (ioctl(fd, SIOCGIFNETMASK, &ifr[i]) != 0)
      {
         continue;
      }  

      nmask = ((struct sockaddr_in *)&ifr[i].ifr_addr)->sin_addr;

      strncpy(ifaces[total].name, iname, sizeof(ifaces[total].name)-1);
      ifaces[total].name[sizeof(ifaces[total].name)-1] = 0;
      ifaces[total].ip = ipaddr;
      ifaces[total].netmask = nmask;
      total++;
   }

   close(fd);

   return total;
}  


/**************************************************************************/
/* This func initialize main bind interface variable for udp server	  */
/**************************************************************************/
void init_bindinterface()
{
  load_interfaces();
  if (iface_count() > 0) 
  {
     if (lp_bind_all_ifaces())
     {
        bind_interface.s_addr = INADDR_ANY;
	LOG_SYS(0, ("Init: Warning. Binding on all interfaces\n"));
     }
     else
     {
        bind_interface = *iface_n_ip(0);
     }

  }
  else
  {
     LOG_SYS(0, ("ERROR: Can't initialize bind interface, exiting...\n"));
     exit(EXIT_ERROR_INTERFACES);
  }
}

