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
/* Various functions to work with online users shared memory		  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* This func add online user record to shm array			  */
/**************************************************************************/
BOOL shm_add(struct online_user &user)
{
   lock_ushm();
   
   /* Check if we have space for new user record */
   if (ipc_vars->online_usr_num < (unsigned long)abs(max_user_cnt))
   {
      ipc_vars->online_usr_num++;
      
      /* let's find free space */      
      for (int i=0; i<max_user_cnt; i++)
      {
         if (usr_shm[i].uin == 0)
	 {	 
            memcpy((void *)&(usr_shm[i]), (const void *)&user, 
                    sizeof(struct online_user));
		    
	    usr_shm[i].shm_index = i;
	    
	    /* for db_online_insert() */
	    user.shm_index = i;
	 
	    unlock_ushm();
	    return(True);
	 }
      }
      
      LOG_SYS(0, ("WARN: usr_num < max_user_num, but i can't find empty record, internal err ?\n"));
      
      unlock_ushm();
      return(False);    
   }
   else
   {
      LOG_SYS(10, ("Warning: shm segment full... Can't accept new user...\n"));
   }
  
   unlock_ushm();
   return(False);
}


/**************************************************************************/
/* This func search online user record in shm array			  */
/**************************************************************************/
BOOL shm_lookup(unsigned long to_uin, struct online_user &temp_user)
{
   lock_ushm();
   
   for (int i=0; i<max_user_cnt; i++)
   {
      if (usr_shm[i].uin == to_uin)
      {
         memcpy((void *)&temp_user, (const void *)&(usr_shm[i]), 
	         sizeof(struct online_user));

         unlock_ushm();
         return(False);
      }
   }
   
   unlock_ushm();
  
   return(True);
}


/**************************************************************************/
/* This func remove online user record from shm array			  */
/**************************************************************************/
BOOL shm_delete(unsigned long to_uin)
{
   lock_ushm();
   
   for (int i=0; i<max_user_cnt; i++)
   {
      if (usr_shm[i].uin == to_uin)
      {
	 ipc_vars->online_usr_num--;
	 bzero((void *)&usr_shm[i], sizeof(struct online_user));

         unlock_ushm();
         return(True);
      }
   }
   
   unlock_ushm();
  
   return(False);
}



/**************************************************************************/
/* This func set specified user status					  */
/**************************************************************************/
BOOL shm_setstatus(struct online_user &temp_user)
{
   lock_ushm();

   for (int i=0; i<max_user_cnt; i++)
   {
      if (usr_shm[i].uin == temp_user.uin)
      {
         usr_shm[i].status  = temp_user.status;
	 usr_shm[i].estat   = temp_user.estat;

         unlock_ushm();
         return(False);
      }
   }
   
   unlock_ushm();
  
   return(True);

}


/**************************************************************************/
/* This func set specified user state					  */
/**************************************************************************/
BOOL shm_setstate(struct online_user &temp_user, int state)
{

   for (int i=0; i<max_user_cnt; i++)
   {
      if (usr_shm[i].uin == temp_user.uin)
      {
         usr_shm[i].state  = state;

         return(False);
      }
   }
   
   return(True);

}


/**************************************************************************/
/* This func set specified user state					  */
/**************************************************************************/
BOOL shm_touch(struct online_user &temp_user)
{
   
   for (int i=0; i<max_user_cnt; i++)
   {
      if (usr_shm[i].uin == temp_user.uin)
      {
         usr_shm[i].lutime  = timeToLong(time(NULL));

         return(False);
      }
   }
   
   return(True);

}


/**************************************************************************/
/* This func return addr of record for specified user			  */
/**************************************************************************/
struct online_user *shm_iget_user(unsigned long uin, unsigned long shm_index)
{
   /* first try index search (check index value first) */
   if ((shm_index < (unsigned long)abs(max_user_cnt)) && 
       (usr_shm[shm_index].uin == uin ))
   {
      return(&(usr_shm[shm_index]));
   }

   DEBUG(50, ("info: shm iget index lookup miss...\n"));

   /* if index search failed */
   for (int i=0; i<max_user_cnt; i++)
   {
      if (usr_shm[i].uin == uin)
      {

         return(&(usr_shm[i]));
      }
   }

   return(NULL);
}


/**************************************************************************/
/* This func return addr of record for specified user			  */
/**************************************************************************/
struct online_user *shm_get_user(unsigned long uin)
{
   for (int i=0; i<max_user_cnt; i++)
   {
      if (usr_shm[i].uin == uin)
      {
         return(&(usr_shm[i]));
      }
   }

   return(NULL);
}


/**************************************************************************/
/* This func return addr of record for user eith same socket and sock_rnd */
/**************************************************************************/
struct online_user *shm_get_user(unsigned long sock_hdl, unsigned long sock_rnd)
{
   for (int i=0; i<max_user_cnt; i++)
   {
      if ((usr_shm[i].sock_rnd == sock_rnd) &&
          (usr_shm[i].sock_hdl == sock_hdl))
      {
         return(&(usr_shm[i]));
      }
   }

   return(NULL);
}


/**************************************************************************/
/* This func return ip addr for specified user				  */
/**************************************************************************/
unsigned long shm_get_ip(unsigned long uin)
{
   for (int i=0; i<max_user_cnt; i++)
   {
      if (usr_shm[i].uin == uin)
      {
         return(ipToIcq(usr_shm[i].ip));
      }
   }

   return(0);
}


/**************************************************************************/
/* This func updates user ttl field in shm record			  */
/**************************************************************************/
void shm_user_update(unsigned long uin, unsigned short ttl)
{
   for (int i=0; i<max_user_cnt; i++)
   {
      if (usr_shm[i].uin == uin)
      {
         usr_shm[i].ttlv = usr_shm[i].ttl;
	 usr_shm[i].lutime = timeToLong(time(NULL));
      }
   }
}


/**************************************************************************/
/* This func check if user with specified uin exists in shm table	  */
/**************************************************************************/
BOOL shm_user_exist(unsigned long uin)
{

   for (int i=0; i<max_user_cnt; i++)
   {
      if (usr_shm[i].uin == uin)
      {
         return(True);
      }
   }

   return(False);
}


/**************************************************************************/
/* This func activate user record 					  */
/**************************************************************************/
BOOL shm_activate_user(unsigned long uin)
{
   for (int i=0; i<max_user_cnt; i++)
   {
      if (usr_shm[i].uin == uin)
      {
         usr_shm[i].active = 1;
         return(True);
      }
   }

   return(False);
}


