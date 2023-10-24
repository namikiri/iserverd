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
/* This file implements V3 packets assembling				  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#include "v3_defines.h"


/**************************************************************************/
/* This used to inform client that search finished 		   	  */
/**************************************************************************/
void v3_send_registration_ok(Packet &pack, unsigned short seq1, 
					   unsigned short seq2,
					   struct full_user_info &new_user)
{
   char  sub_str[512];
  
   reply_pack.clearPacket();
   
   strncpy(sub_str, lp_v3_post_registration(), sizeof(sub_str)-1);
   subst_post_register(sub_str, new_user.uin, new_user.passwd);
   
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_REGISTRATIONxOK
	      << (unsigned short)seq1
	      << (unsigned short)seq2
	      << (unsigned  long)0x0000
	      << (unsigned  long)0x0000
	      << (unsigned short)(strlen(sub_str)+1)
 	      << sub_str;

   reply_pack.from_ip   = pack.from_ip;
   reply_pack.from_port = pack.from_port;
   
   udp_send_direct_packet(reply_pack);

}


/**************************************************************************/
/* This used to inform client that search finished 		   	  */
/**************************************************************************/
void v3_send_registration_info(Packet &int_pack, unsigned short seq2)
{
   cstring admin_notes;
   snprintf(admin_notes, 15, "Notes...");
  
   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_SENDxFRAGMENT
	      << (unsigned short)0x0000
	      << (unsigned short)seq2
	      << (unsigned  long)0x0000
	      << (unsigned  long)0x0000
	      << (char)0x00
	      << (char)0x03
  
              << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_REGISTER_INFO
              << (unsigned short)0x0000
              << (unsigned short)seq2	
              << (unsigned  long)0x0000
              << (unsigned  long)0x0000
  	      << (unsigned short)(strlen(lp_v3_admin_notes())+1)
	      << lp_v3_admin_notes()
	      << (char)lp_v3_registration_enabled()
	      << (unsigned short)0x0002
	      << (unsigned short)0x002A;

   udp_send_direct_packet(reply_pack);
   
   PGresult *res;
   int deps_num;

   fstring dbcomm_str;
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, 
	    "SELECT depcode, depmin FROM Users_Deps");

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[V3 SELECT DEPS]");
      return;
   }

   deps_num = PQntuples(res); 
   
   if (deps_num > DEPS_MAX_NUM) deps_num = DEPS_MAX_NUM;
   
   if (PQnfields(res) != 2) 
   {
      LOG_SYS(0, ("Corrypted deps table structure in db: \"%s\"\n", 
                  lp_db_users()));
      exit(EXIT_ERROR_DB_STRUCTURE);
   } 
   
   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_SENDxFRAGMENT
	      << (unsigned short)0x0001
	      << (unsigned short)seq2
	      << (unsigned  long)0x0000
	      << (unsigned  long)0x0000
	      << (char)0x01
	      << (char)0x03

	      << (unsigned  long)lp_deplist_vers()
	      << (unsigned  long)deps_num;
	      
   for (int i=0; i<deps_num; i++)
   {
      char depmin[10];
      snprintf(depmin, sizeof(depmin)-1, PQgetvalue(res, i, 1));
      ITrans.translateToClient(depmin);
      reply_pack << (unsigned  long)atoul(PQgetvalue(res, i, 0));
      reply_pack << (unsigned short)(strlen(depmin)+1);
      reply_pack << depmin;
   }

   udp_send_direct_packet(reply_pack);
   
   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_SENDxFRAGMENT
	      << (unsigned short)0x0002
	      << (unsigned short)seq2
	      << (unsigned  long)0x0000
	      << (unsigned  long)0x0000
	      << (char)0x02
	      << (char)0x03
	      
	      << (unsigned short)(1)
	      << (char)0x00
	      << (unsigned short)(1)
	      << (char)0x00
	      << (unsigned short)(1)
	      << (char)0x00
	      << (unsigned short)(1)
	      << (char)0x00
	      << (unsigned long)0xFFFFFFFF
	      << (unsigned short)7
	      << (unsigned short)(1)
	      << (char)0x00;
	      
   udp_send_direct_packet(reply_pack);
}


/**************************************************************************/
/* This used to inform client that search finished 		   	  */
/**************************************************************************/
void v3_send_search_finished(Packet &pack, struct online_user &user, int more)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num;
   reply_pack.clearPacket();
 
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_SEARCHxDONE
              << (unsigned short)user.servseq
              << (unsigned short)seq2	
              << (unsigned  long)uin_num	
              << (unsigned  long)0x0000
	      << (char)more;
	    
   reply_pack.from_ip   = pack.from_ip;
   reply_pack.from_port = pack.from_port;
 
   /* sending packets to client */
   v3_send_indirect(reply_pack, user.uin, user.shm_index);
}


/**************************************************************************/
/* This funct send packet with found user information	   	   	  */
/**************************************************************************/
void v3_send_found_info(Packet &pack, struct online_user &user, 
			    struct found_info &tuser)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num;

   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_SEARCHxFOUND
              << (unsigned short)user.servseq
              << (unsigned short)seq2
              << (unsigned  long)user.uin
              << (unsigned  long)0x0000
	      << (unsigned  long)tuser.uin
	      << (unsigned short)(strlen(tuser.nick)+1)
	      << tuser.nick
	      << (unsigned short)(strlen(tuser.first)+1)
	      << tuser.first
	      << (unsigned short)(strlen(tuser.last)+1)
	      << tuser.last
	      << (unsigned short)(strlen(tuser.email2)+1)
	      << tuser.email2
              << (char)tuser.auth;

   DEBUG(100, ("Info: Sending found user (%lu) basic info to user %lu\n", tuser.uin, user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v3_send_indirect(reply_pack, user.uin, user.shm_index);

}   


/**************************************************************************/
/* This used to reply on info changing 				   	  */
/**************************************************************************/
void v3_send_reply_ok(Packet &pack, struct online_user &user, 
				    unsigned short command)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num;
   reply_pack.clearPacket();
 
   reply_pack << (unsigned short)V3_PROTO	/* proto number 	  */
              << (unsigned short)command	/* command number 	  */
              << (unsigned short)user.servseq	/* sequence 1 num 	  */
              << (unsigned short)seq2		/* sequence 2 num 	  */
              << (unsigned  long)uin_num	/* client uin number 	  */
              << (unsigned  long)0x0000;	/* reserved number	  */
	    
   reply_pack.from_ip   = pack.from_ip;
   reply_pack.from_port = pack.from_port;
 
   /* send packet to client */
   v3_send_indirect(reply_pack, user.uin, user.shm_index);
}


/**************************************************************************/
/* This function will send deplist to authentificated user 	    	  */
/**************************************************************************/
void v3_send_depslist1(Packet &pack, struct online_user &tuser, 
				    unsigned short servseq)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num;

   /* first of all we need to connect to database and receive     */
   /* departments list, then we'll make proper packet ans send it */

   PGresult *res; /* database result variable */
   int deps_num;

   fstring dbcomm_str;
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, 
	    "SELECT depcode, depmin FROM Users_Deps");

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[V3 SELECT DEPS #2]");
      return;
   }

   deps_num = PQntuples(res); 
   
   if (deps_num > DEPS_MAX_NUM) deps_num = DEPS_MAX_NUM;   
   if (PQnfields(res) != 2) 
   {
      LOG_SYS(0, ("Corrypted deps table structure in db: \"%s\"\n", 
                  lp_db_users()));
      exit(EXIT_ERROR_DB_STRUCTURE);
   }

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num;

   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_USERxDEPS_LIST1
              << (unsigned short)servseq
              << (unsigned short)seq2
              << (unsigned  long)tuser.uin
              << (unsigned  long)0x0000
	      << (unsigned short)0x003C
	      << (unsigned  long)lp_deplist_vers()
	      << (unsigned  long)deps_num;

   for (int i=0; i<deps_num; i++)
   {
      char depmin[10];
      snprintf(depmin, sizeof(depmin)-1, PQgetvalue(res, i, 1));
      ITrans.translateToClient(depmin);
      reply_pack << (unsigned  long)atoul(PQgetvalue(res, i, 0));
      reply_pack << (unsigned short)(strlen(depmin)+1);
      reply_pack << depmin;
   }
   
   reply_pack << (unsigned short)0x0002;
   reply_pack << (unsigned short)0x002a;

   DEBUG(10, ("Deplist: Sending department list to user %lu\n", tuser.uin));

   reply_pack.from_ip   = tuser.ip;
   reply_pack.from_port = tuser.udp_port;
   
   if (servseq != 0) { v3_send_indirect(reply_pack, tuser.uin, tuser.shm_index); }
   else udp_send_direct_packet(reply_pack);
}



/**************************************************************************/
/* This function will send deplist to authentificated user 	    	  */
/**************************************************************************/
void v3_send_depslist(Packet &pack, struct online_user &tuser, 
				    unsigned short servseq)
{

   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num;

   /* first of all we need to connect to database and receive     */
   /* departments list, then we'll make proper packet ans send it */

   PGresult *res; /* database result variable */
   int deps_num;

   fstring dbcomm_str;
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, 
	    "SELECT depcode, depmin FROM Users_Deps");

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[SEND DEPS LIST]");
      return;
   }

   deps_num = PQntuples(res); 
   
   if (deps_num > DEPS_MAX_NUM) deps_num = DEPS_MAX_NUM;
   
   if (PQnfields(res) != 2) 
   {
      LOG_SYS(0, ("Corrypted deps table structure in db: \"%s\"\n", lp_db_users()));
      exit(EXIT_ERROR_DB_STRUCTURE);
   }

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num;

   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_USERxDEPS_LIST
              << (unsigned short)servseq
              << (unsigned short)seq2
              << (unsigned  long)tuser.uin
              << (unsigned  long)0x0000
	      << (unsigned  long)lp_deplist_vers()
	      << (unsigned  long)deps_num;

   for (int i=0; i<deps_num; i++)
   {
      char depmin[10];
      snprintf(depmin, sizeof(depmin)-1, PQgetvalue(res, i, 1));
      ITrans.translateToClient(depmin);
      reply_pack << (unsigned  long)atoul(PQgetvalue(res, i, 0));
      reply_pack << (unsigned short)(strlen(depmin)+1);
      reply_pack << depmin;
   }
   
   reply_pack << (unsigned short)0x0002;
   reply_pack << (unsigned short)0x002a;

   DEBUG(10, ("Deplist: Sending department list to user %lu\n", tuser.uin));

   reply_pack.from_ip   = tuser.ip;
   reply_pack.from_port = tuser.udp_port;
   
   if (servseq != 0) { v3_send_indirect(reply_pack, tuser.uin, tuser.shm_index); }
   else udp_send_direct_packet(reply_pack);   
}


/**************************************************************************/
/* This funct send requested user work info	 		   	  */
/**************************************************************************/
void v3_send_work_info(Packet &pack, struct online_user &user, 
			    struct full_user_info &tuser)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num;

   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_USERxINFO_WORK
              << (unsigned short)user.servseq
              << (unsigned short)seq2
              << (unsigned  long)user.uin
              << (unsigned  long)0x0000
	      << (unsigned short)(strlen(tuser.waddr)+1)
	      << tuser.waddr
	      << (unsigned short)(strlen(tuser.wcity)+1)
	      << tuser.wcity
	      << (unsigned short)(strlen(tuser.wstate)+1)
	      << tuser.wstate
	      << (unsigned short)tuser.wcountry
	      << (unsigned short)(strlen(tuser.wcompany)+1)
	      << tuser.wcompany
	      << (unsigned short)(strlen(tuser.wtitle)+1)
	      << tuser.wtitle
	      << (unsigned long)tuser.wdepart
	      << (unsigned short)(strlen(tuser.wphone)+1)
	      << tuser.wphone
	      << (unsigned short)(strlen(tuser.wfax)+1)
	      << tuser.wfax
	      << (unsigned short)(strlen(tuser.wpager)+1)
	      << tuser.wpager
	      << (unsigned long)tuser.wzip;

   DEBUG(100, ("Info: Sending user's %lu work info to user %lu\n", 
               tuser.uin, user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v3_send_indirect(reply_pack, user.uin, user.shm_index);
   
}

/**************************************************************************/
/* This funct send requested user home info 		 	   	  */
/**************************************************************************/
void v3_send_home_info(Packet &pack, struct online_user &user, 
			    struct full_user_info &tuser)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num;
   char temp_year;

   /* we need seq1, seq2 numbers and uin */
   pack.reset();
   pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num;

   if (tuser.byear < 1900) {temp_year = tuser.byear; } 
                      else {temp_year = tuser.byear - 1900; };

   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_USERxINFO_HOME
              << (unsigned short)user.servseq
              << (unsigned short)seq2
              << (unsigned  long)user.uin
              << (unsigned  long)0x0000
	      << (unsigned short)(strlen(tuser.haddr)+1)
	      << tuser.haddr
	      << (unsigned short)(strlen(tuser.hcity)+1)
	      << tuser.hcity
	      << (unsigned short)(strlen(tuser.hstate)+1)
	      << tuser.hstate
	      << (unsigned short)tuser.hcountry
	      << (unsigned short)(strlen(tuser.hphone)+1)
	      << tuser.hphone
	      << (unsigned short)(strlen(tuser.hfax)+1)
	      << tuser.hfax
	      << (unsigned short)(strlen(tuser.hcell)+1)
	      << tuser.hcell
	      << (unsigned long)tuser.hzip
	      << (char)tuser.gender
	      << (unsigned short)tuser.age
	      << (char)tuser.bday
	      << (char)tuser.bmonth
	      << (char)temp_year
	      << (char)0x00;

   DEBUG(100, ("Info: Sending user's %lu home info to user %lu\n", 
               tuser.uin, user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v3_send_indirect(reply_pack, user.uin, user.shm_index);
   
}


/**************************************************************************/
/* This funct send requested user work www page info 		   	  */
/**************************************************************************/
void v3_send_work_web(Packet &pack, struct online_user &user, 
			    struct full_user_info &tuser)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num;

   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_USERxINFO_WWEB
              << (unsigned short)user.servseq
              << (unsigned short)seq2
              << (unsigned  long)user.uin
              << (unsigned  long)0x0000
	      << (unsigned short)(strlen(tuser.wpage)+1)
	      << tuser.wpage;

   DEBUG(100, ("Info: Sending user's %lu work page info to user %lu\n", 
               tuser.uin, user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* sending packets to client, we need confirm! */
   v3_send_indirect(reply_pack, user.uin, user.shm_index);
   
}


/**************************************************************************/
/* This funct send requested user home page info 		   	  */
/**************************************************************************/
void v3_send_home_web(Packet &pack, struct online_user &user, 
			    struct full_user_info &tuser)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num;

   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_USERxINFO_HWEB
              << (unsigned short)user.servseq
              << (unsigned short)seq2
              << (unsigned  long)user.uin
              << (unsigned  long)0x0000
	      << (unsigned short)(strlen(tuser.hpage)+1)
	      << tuser.hpage;

   DEBUG(100, ("Info: Sending user's %lu home page info to user %lu\n", 
	       tuser.uin, user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v3_send_indirect(reply_pack, user.uin, user.shm_index);
   
}

/**************************************************************************/
/* This funct send requested user basic info 		   	   	  */
/**************************************************************************/
void v3_send_basic_info(Packet &pack, struct online_user &user, 
			    struct full_user_info &tuser)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num;

   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_USERxINFO_BASIC
              << (unsigned short)user.servseq
              << (unsigned short)seq2
              << (unsigned  long)user.uin
              << (unsigned  long)0x0000
	      << (unsigned short)(strlen(tuser.nick)+1)
	      << tuser.nick
	      << (unsigned short)(strlen(tuser.first)+1)
	      << tuser.first
	      << (unsigned short)(strlen(tuser.last)+1)
	      << tuser.last
	      << (unsigned short)(strlen(tuser.email2)+1)
	      << tuser.email2
              << (char)tuser.auth;

   DEBUG(100, ("Info: Sending user's %lu basic info to user %lu\n", 
               tuser.uin, user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v3_send_indirect(reply_pack, user.uin, user.shm_index);
   
}


/************************************************************************/
/* This funct send requested user basic info in single packet  	   	*/
/************************************************************************/
void v3_send_basic_info_single(Packet &pack, struct online_user &user, 
			    struct full_user_info &tuser)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num;

   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_USERxINFO_SINGLE
              << (unsigned short)user.servseq
              << (unsigned short)seq2
              << (unsigned  long)user.uin
              << (unsigned  long)0x0000
	      << (unsigned short)(strlen(tuser.nick)+1)
	      << tuser.nick
	      << (unsigned short)(strlen(tuser.first)+1)
	      << tuser.first
	      << (unsigned short)(strlen(tuser.last)+1)
	      << tuser.last
	      << (unsigned short)(strlen(tuser.email2)+1)
	      << tuser.email2
              << (char)tuser.auth;

   DEBUG(100, ("Info: Sending user's %lu basic (single) info to user %lu\n", 
               tuser.uin, user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v3_send_indirect(reply_pack, user.uin, user.shm_index);
   
}


/**************************************************************************/
/* Notes reply packet (deliver notes to user)			   	  */
/**************************************************************************/
void v3_send_notes(Packet &pack, struct online_user &user, 
			    struct notes_user_info notes)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num;

   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_USERxNOTES
              << (unsigned short)user.servseq
              << (unsigned short)seq2
              << (unsigned  long)user.uin
              << (unsigned  long)0x0000
              << (unsigned short)(strlen(notes.notes)+1)
              << notes.notes
              << (unsigned  long)notes.nupdate
	      << (unsigned  long)notes.lastlogin
              << (unsigned  long)notes.ip_addr;

   DEBUG(100, ("Info: Sending user's %lu notes to user %lu\n", 
               notes.uin, user.uin));

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;
   
   /* send packet to client, we need confirm! */
   v3_send_indirect(reply_pack, user.uin, user.shm_index);
   
}


/**************************************************************************/
/* User want to get unknown user's notes/info			   	  */
/**************************************************************************/
void v3_send_invalid_user(Packet &pack, struct online_user &user)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num, to_uin, junk;
 
   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num >> junk >> to_uin;
 
   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_USERxINVALIDxUIN
	      << (unsigned short)user.servseq
	      << (unsigned short)seq2
	      << (unsigned long )user.uin
	      << (unsigned long )0x0000
	      << (unsigned long )to_uin;

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;

   /* send packet to client */
   v3_send_indirect(reply_pack, user.uin, user.shm_index);

}


/**************************************************************************/
/* Non-logged client tried to work with server 			   	  */
/**************************************************************************/
void v3_send_not_connected(Packet &pack)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num;
 
   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num;
 
   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_ERR_NOT_CONNECTED
	      << (unsigned short)0x0000
	      << (unsigned short)seq2
	      << (unsigned long )uin_num
	      << (unsigned long )0x0000;

   reply_pack.from_ip   = pack.from_ip;
   reply_pack.from_port = pack.from_port;

   /* send packet to client */
   udp_send_direct_packet(reply_pack);

}

/**************************************************************************/
/* Server should confirm *every* received packet (except client ack). 	  */
/**************************************************************************/
void v3_send_ack(Packet &pack)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num;

   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num;
   reply_pack.clearPacket();
   
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_ACK
              << (unsigned short)seq1
              << (unsigned short)seq2
              << (unsigned  long)uin_num
              << (unsigned  long)0x0000;
	    
   reply_pack.from_ip   = pack.from_ip;
   reply_pack.from_port = pack.from_port;
 
   /* sending packets to client */
   udp_send_direct_packet(reply_pack);
}


/**************************************************************************/
/* This packet sent if client can't be logged in for some reason   	  */
/**************************************************************************/
void v3_send_login_err(Packet &pack, char *errmessage)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num;
 
   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num;
 
   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_LOGIN_ERR
	      << (unsigned short)0x0000
	      << (unsigned short)seq2
	      << (unsigned long )uin_num
	      << (unsigned long )0x0000
	      << (unsigned short)(strlen(errmessage)+1)
	      << errmessage;

   reply_pack.from_ip   = pack.from_ip;
   reply_pack.from_port = pack.from_port;

   /* send packet to client */
   udp_send_direct_packet(reply_pack);
}


/**************************************************************************/
/* This packet sent if client's password wrong or it doesn't exist 	  */ 
/* in user database 						   	  */
/**************************************************************************/
void v3_send_pass_err(Packet &pack)
{
   unsigned short seq1, seq2, vvers, pcomm;
   unsigned long  uin_num;
 
   /* we need seq1, seq2 numbers and uin to */
   pack.reset();
   pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num;
 
   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_WRONGxPASSWD
	      << (unsigned short)0x0000
	      << (unsigned short)seq2
	      << (unsigned long )uin_num
	      << (unsigned long )0x0000;

   reply_pack.from_ip   = pack.from_ip;
   reply_pack.from_port = pack.from_port;

   /* send packet to client */
   udp_send_direct_packet(reply_pack);
}


/**************************************************************************/
/* This packet used to inform client that all his contact list 	   	  */
/* processed 							   	  */
/**************************************************************************/
void v3_send_end_contact(struct online_user &user)
{
 
   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_USERxLISTxDONE
 	      << (unsigned short)user.servseq
	      << (unsigned short)0x0000
	      << (unsigned long )user.uin
	      << (unsigned long )0x0000;

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;

   /* send packet to client */
   v3_send_indirect(reply_pack, user.uin, user.shm_index);
}


/**************************************************************************/
/* Login reply packet						   	  */
/**************************************************************************/
void v3_send_login_reply(Packet &pack, struct login_user_info &userinfo, 
			 struct online_user &user)
{
 unsigned short seq1, seq2, vvers, pcomm;
 unsigned long  uin_num;
 
 /* we need seq1, seq2 numbers and uin to */
 pack.reset();
 pack >> vvers >> pcomm >> seq1 >> seq2 >> uin_num;
 
 reply_pack.clearPacket();
 reply_pack << (unsigned short)V3_PROTO		     /* protocol id       */
            << (unsigned short)ICQ_CMDxSND_HELLO     /* login reply cmd   */
	    << (unsigned short)0x0000		     /* reserved    	  */
	    << (unsigned short)seq2		     /* server sequence   */
	    << (unsigned  long)uin_num		     /* client uin number */
	    << (unsigned  long)0x0000		     /* reserved 	  */
	    << (unsigned  long)ipToIcq(pack.from_ip) /* client ip addr    */
	    << (unsigned  long)0x0000		     /* reserved          */
	    << (unsigned  long)lp_externals_num()    /* externals number  */
	    << (unsigned  long)lp_deplist_vers()     /* deplist version   */
	    << (char)userinfo.ch_password	     /* muist change pass */
	    << (char)userinfo.can_broadcast	     /* can broadcast     */
	    << (unsigned  long)0x000000FA	     /* unknown 0xFA 	  */
	    << (unsigned short)(lp_v3_pingtime()-10) /* ping frequency    */
	    << (unsigned short)(lp_v3_timeout())     /* packet timeout    */
	    << (unsigned short)0x000A		     /* retry timeout     */
	    << (unsigned short)(lp_v3_retries());    /* num of retries    */

 reply_pack.from_ip   = pack.from_ip;
 reply_pack.from_port = pack.from_port;
 
 /* send packet to client */
 v3_send_indirect(reply_pack, uin_num, user.shm_index);
 
}

/**************************************************************************/
/* User online packet						   	  */
/**************************************************************************/
void v3_send_user_online(struct online_user &to_user, 
			 struct online_user &user)
{
   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_USERxONLINE
              << (unsigned short)to_user.servseq
              << (unsigned short)0x0000
              << (unsigned  long)to_user.uin
              << (unsigned  long)0x0000
              << (unsigned  long)user.uin;
   
   /* V3 clients TCP protection from other (99a, 99b, 2000, 98)           */
   /* WARN: v3 client crashes on non-v3 client connect, don't remove that */
   /* unless you have patched or selfmade v3 client                       */
   if ((user.protocol == V3_PROTO) || 
      (((user.protocol == V7_PROTO)) &&
        (lp_v7_direct_v3_connect())))
       
   {
      reply_pack << (unsigned  long)ipToIcq(user.ip)
                 << (unsigned  long)user.tcp_port
	         << (unsigned  long)ipToIcq(user.int_ip)
                 << (char)user.dc_type;
   }
   else
   {
      reply_pack << (unsigned  long)0x00000000
                 << (unsigned  long)0x00000000
	         << (unsigned  long)0x00000000
                 << (char)0x00;
   }
	      
   reply_pack << (unsigned short)user.status
	      << (unsigned short)user.estat
              << (unsigned short)user.tcpver
	      << (unsigned short)0x0000;

   reply_pack.from_ip   = to_user.ip;
   reply_pack.from_port = to_user.udp_port;
   
   /* send packets to client, we need confirm! */
   v3_send_indirect(reply_pack, to_user.uin, to_user.shm_index);
}


/**************************************************************************/
/* User change status packet					   	  */
/**************************************************************************/
void v3_send_user_status(struct online_user &to_user, 
			 struct online_user &user)
{
   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_USERxSTATUS
              << (unsigned short)to_user.servseq
              << (unsigned short)0x0000
              << (unsigned  long)to_user.uin
              << (unsigned  long)0x0000
              << (unsigned  long)user.uin
              << (unsigned short)user.status
	      << (unsigned short)user.estat;

   reply_pack.from_ip   = to_user.ip;
   reply_pack.from_port = to_user.udp_port;
   
   /* send packet to client, we need confirm! */
   v3_send_indirect(reply_pack, to_user.uin, to_user.shm_index);
}


/**************************************************************************/
/* User offline packet						   	  */
/**************************************************************************/
void v3_send_user_offline(struct online_user &to_user, unsigned long uin)
{
   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_USERxOFFLINE
              << (unsigned short)to_user.servseq
              << (unsigned short)0x0000
              << (unsigned  long)to_user.uin
              << (unsigned  long)0x0000
              << (unsigned  long)uin;

   reply_pack.from_ip   = to_user.ip;
   reply_pack.from_port = to_user.udp_port;

   /* send packet to client, we need confirm! */
   v3_send_indirect(reply_pack, to_user.uin, to_user.shm_index);
}


/**************************************************************************/
/* Send out of band packet to user				   	  */
/**************************************************************************/
void v3_send_oob(unsigned long uin_num)
{

}


/**************************************************************************/
/* Send busy packet to user					   	  */
/**************************************************************************/
void v3_send_busy(unsigned long uin_num, unsigned short seq2, 
		  struct in_addr from_ip, unsigned short from_port)
{
 Packet reply_pack;
 
 reply_pack << (unsigned short)V3_PROTO
            << (unsigned short)ICQ_CMDxSND_BUSY
	    << (unsigned short)0x0000
	    << (unsigned short)seq2
	    << (unsigned  long)uin_num
	    << (unsigned  long)0x0000;

 reply_pack.from_ip   = from_ip;
 reply_pack.from_port = from_port;

 udp_send_direct_packet(reply_pack); 
}


/**************************************************************************/
/* Disconnect packet						   	  */
/**************************************************************************/
void v3_send_srv_disconnect(struct online_user &user, char *errmessage)
{
 
   reply_pack.clearPacket();
   reply_pack << (unsigned short)V3_PROTO
              << (unsigned short)ICQ_CMDxSND_SETxOFFLINE
	      << (unsigned short)user.servseq
	      << (unsigned short)0x0000
	      << (unsigned long )user.uin
	      << (unsigned long )0x0000
	      << (unsigned short)(strlen(errmessage)+1)
	      << errmessage;

   reply_pack.from_ip   = user.ip;
   reply_pack.from_port = user.udp_port;

   DEBUG(10, ("Trying to send forced logoff notification to client...\n"));
   /* send packet to client */
   v3_send_indirect(reply_pack, user.uin, user.shm_index);
}

