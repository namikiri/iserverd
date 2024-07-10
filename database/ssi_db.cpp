/**************************************************************************/
/*									  */
/* Copyright (c) 2000-2005 by Alexandr V. Shutko, Khabarovsk, Russia	  */
/* All rights reserved.							  */
/*                                                                        */
/* Redistribution and use in source and binary forms, with or without	  */
/* modification, are permitted provided that the following conditions	  */
/* are met:								  */
/* 1. Redistributions of source code must retain the above copyright	  */
/*    notice, this list of conditions and the following disclaimer.	  */
/* 2. Redistributions in binary form must reproduce the above copyright	  */
/*    notice, this list of conditions and the following disclaimer in 	  */
/*    the documentation and/or other materials provided with the 	  */
/*    distribution.							  */
/*                                                                        */
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
/* This unit implements database functions related ssi processing         */
/*                                                                        */
/**************************************************************************/

#include "includes.h"

extern unsigned short type_limits[SSI_TYPES_NUM];

/* table to convert item type to internal item type */
unsigned short itype_conv[SSI_TYPES_NUM] =
{
   NORMAL_CONTACT, 	/* item 0x0000 - buddy     */
   0x0FFF, 		/* item 0x0001 - group     */
   VISIBLE_CONTACT, 	/* item 0x0002 - visible   */
   INVISIBLE_CONTACT,	/* item 0x0003 - invisible */
   0x0FFF, 		/* item 0x0004 - rights    */
   0x0FFF, 		/* item 0x0005 - presense  */
   0x0FFF, 		/* item 0x0006             */
   0x0FFF, 		/* item 0x0007             */
   0x0FFF, 		/* item 0x0008             */
   0x0FFF, 		/* item 0x0009 - icqtic    */
   0x0FFF, 		/* item 0x000a             */
   0x0FFF, 		/* item 0x000b             */
   0x0FFF, 		/* item 0x000c             */
   0x0FFF, 		/* item 0x000d             */
   0x0FFF, 		/* item 0x000e - ignore    */
   0x0FFF, 		/* item 0x000f             */
   0x0FFF, 		/* item 0x0010 - non-icq   */
   0x0FFF, 		/* item 0x0011             */
   0x0FFF, 		/* item 0x0012             */
   0x0FFF, 		/* item 0x0013 - import    */
   0x0FFF 		/* item 0x0014             */
};

/**************************************************************************/
/* Retrieve number of items for specified type to check limits            */
/**************************************************************************/
unsigned short db_ssi_get_itemnum(unsigned long uin, unsigned short itype)
{
   PGresult *res;
   fstring dbcomm_str;
   unsigned short item_num = 0;

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT count(*) FROM users_ssi_data WHERE (ouin=%lu) AND (type=%d)",
	    uin, itype);

   res = PQexec(users_dbconn, dbcomm_str);

   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[GET SSI ITEMNUM]");
      return(0);
   }

   if (PQntuples(res) > 0)
   {
      item_num = atoul(PQgetvalue(res, 0, 0));
      DEBUG(10, ("Limit check returned %d records...\n", item_num));

      PQclear(res);
      return(item_num);
   }

   PQclear(res);
   return(0);
}


/**************************************************************************/
/* This function retrieve user SSI items number and modification date     */
/**************************************************************************/
int db_ssi_get_modinfo(struct online_user *user, unsigned long &mod_date,
                       unsigned short &item_num)
{
   PGresult *res;
   fstring dbcomm_str;

   mod_date = 0;
   item_num = 0;
   int err_flag = 1;

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT udate FROM users_ssi_data WHERE (ouin=%lu) AND (type=%d)",
	    user->uin, ITEM_REVISION);

   res = PQexec(users_dbconn, dbcomm_str);

   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[GET USER SSI REVISION]");
      return(1);
   }

   if (PQntuples(res) > 0)
   {
      mod_date = atoul(PQgetvalue(res, 0, 0));
      err_flag = 0;
   }

   PQclear(res);

   /* Now we should retrieve SSI items count */
   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT count(*) FROM users_ssi_data WHERE  (ouin=%lu) AND (type<>%d)",
	    user->uin, ITEM_REVISION);

   res = PQexec(users_dbconn, dbcomm_str);

   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[GET USER SSI ITEMNUM]");
      return(1);
   }

   if (PQntuples(res) > 0)
   {
      item_num = atoul(PQgetvalue(res, 0, 0));
      DEBUG(100, ("Contact check: items_num=%d, date=%08lX\n", item_num, mod_date));

      /* fix for missing mod_date entry */
      if ((item_num != 0) && (err_flag)) err_flag = db_ssi_create_modinfo(user);

      PQclear(res);
      return(err_flag);
   }

   PQclear(res);
   return(1);
}


/**************************************************************************/
/* This function update user SSI revision number and modification date    */
/**************************************************************************/
int db_ssi_set_modinfo(struct online_user *user, BOOL import)
{
   PGresult *res;
   fstring dbcomm_str;
   unsigned long  udate;
   unsigned short item_num;
   int get_result = 0;

   get_result = db_ssi_get_modinfo(user, udate, item_num);

   if (get_result == 0)
   {
      /* ITEM_REVISION record exist for this user */
      slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
              "UPDATE users_ssi_data SET udate=%lu, revnum=revnum+1 WHERE (ouin=%lu) AND (type=%d);",
	       time(NULL), user->uin, ITEM_REVISION);
   }
   else
   {
      /* ITEM_REVISION record doesn't exist for this user */
      slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
              "INSERT INTO users_ssi_data VALUES (%lu,0,0,0,%d,%d,0,%lu,%d,'','','',0,0,0,0,1);",
	       user->uin, ITEM_REVISION, 0x0FFF, time(NULL), 1);
   }

   res = PQexec(users_dbconn, dbcomm_str);

   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[UPDATE USER SSI REVISION]");
      return(1);
   }

   /* insert/update completed successfully */
   PQclear(res);

   if (import) update_import_item(user->uin);
   return(0);
}


/**************************************************************************/
/* This function create/update import revision record			  */
/**************************************************************************/
int update_import_item(unsigned long uin)
{
   PGresult *res;
   fstring dbcomm_str;

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
            "UPDATE users_ssi_data SET udate=%lu WHERE (ouin=%lu) AND (type=%d)",
	     time(NULL), uin, ITEM_IMPORT);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[UPDATE USER SSI IMPORT]");
      return(1);
   }

   /* ------------- */
   if (atoul(PQcmdTuples(res)) != 0) { PQclear(res); return(0); }
   PQclear(res);
   /* ------------- */

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "INSERT INTO users_ssi_data VALUES (%lu,0,0,0,%d,%d,0,%lu,0,'Import time','','',0,0,0,0,1)",
            uin, ITEM_IMPORT, 0xFFF, time(NULL));

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[INSERT USER SSI REVISION]");
      return(1);
   }

   PQclear(res);
   return(0);
}


/**************************************************************************/
/* This function create user SSI revision record			  */
/**************************************************************************/
int db_ssi_create_modinfo(struct online_user *user)
{
   PGresult *res;
   fstring dbcomm_str;

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "INSERT INTO users_ssi_data VALUES (%lu,0,0,0,%d,%d,0,%lu,%d,'','','',0,0,0,0,1)",
	    user->uin, ITEM_REVISION, 0x0FFF, time(NULL), 1);

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[CREATE USER SSI REVISION]");
      return(1);
   }

   PQclear(res);
   return(0);
}


/**************************************************************************/
/* This function add new ssi item to database				  */
/**************************************************************************/
int ssi_db_add_item(struct online_user *user, char *item_name,
                    unsigned short gid, unsigned short iid,
		    unsigned short itype, class tlv_chain_c &tlv_chain)
{
   PGresult *res;
   unsigned short int_itype = itype_conv[itype];
   unsigned short auth = 0;
   unsigned long  tuin = 0;
   unsigned long  iperm = 0, vclass = 0, perms = 0;
   char privacy = 0;
   class tlv_c *tlv = NULL;
   fstring nickname, nickname2, item_name2;

   strncpy(nickname, "", 64);

   /* First I should check for item limit */
   if (db_ssi_get_itemnum(user->uin, itype) + 1 > type_limits[itype])
   {
      return(-2);
   }

   /* Check for buddy item nickname */
   if ((itype == ITEM_BUDDY) && ((tlv = tlv_chain.get(SSI_TLV_NICKNAME)) != NULL))
   {
      if (!v7_extract_string(nickname, *tlv, 128))
      {
         DEBUGADD(10, ("SSI_ERROR: TLV(131) problem... Nickname too big\n"));
         return(False);
      }
      tlv_chain.remove(SSI_TLV_NICKNAME);
   }

   /* Check if this user added with auth flag */
   if ((tlv = tlv_chain.get(SSI_TLV_AUTH)) != NULL)
   {
      auth = 1;
      tlv_chain.remove(SSI_TLV_AUTH);
   }

   /* Check for TLV(C9) in presence/visibility item */
   if ((tlv = tlv_chain.get(SSI_TLV_IDLEPERMS)) != NULL)
   {
      if (itype == ITEM_PRESENCE)
      {
         *tlv >> iperm;
      }

      tlv_chain.remove(SSI_TLV_IDLEPERMS);
   }

   /* Check for TLV(CA) in rights item */
   if ((tlv = tlv_chain.get(SSI_TLV_PRIVACY)) != NULL)
   {
      if (itype == ITEM_RIGHTS)
      {
         *tlv >> privacy;
      }

      tlv_chain.remove(SSI_TLV_PRIVACY);
   }

   /* Check for TLV(CB) in rights item */
   if ((tlv = tlv_chain.get(SSI_TLV_RIGHTS)) != NULL)
   {
      if (itype == ITEM_RIGHTS)
      {
         *tlv >> vclass;
      }

      tlv_chain.remove(SSI_TLV_RIGHTS);
   }

   /* Check for TLV(CC) in rights item */
   if ((tlv = tlv_chain.get(SSI_TLV_PRESENSE)) != NULL)
   {
      if (itype == ITEM_RIGHTS)
      {
         *tlv >> perms;
      }

      tlv_chain.remove(SSI_TLV_PRESENSE);
   }

   /* Check if we should convert target uin value */
   /* I'll add screenname -> uin converter later  */
   tuin = atoul(item_name);

   /* Now time to code remaining TLVs into blob ascii string */
   tlv_chain.encode(tempst);
   convert_to_postgres2(nickname2, sizeof(fstring)-1, nickname);
   convert_to_postgres2(item_name2, sizeof(fstring)-1, item_name);

   DEBUGADD(50, ("SSI_ADD: ou=%lu,tu=%lu,gid=%d,iid=%d,it=%d,t=%d,auth=%d,tlvnum=%d\n",
                  user->uin, tuin, gid, iid, int_itype, itype, auth, tlv_chain.num()));

   if (tlv_chain.num()) { DEBUGADD(100, ("Tlv chain: %s\n", tempst)); }

/* Now we can define and run sql insert db query   */
/* We'll use #define to avoid multistring literals */
#define SSI_IIN195 \
  "INSERT INTO users_ssi_data VALUES \
   ( \
      %lu, %lu, %d, %d, %d, %d, %d, %d, %d, '%s', '%s', '%s', %lu, %d, %lu, %lu, 0 \
   )"

   snprintf(msg_buff, sizeof(msg_buff)-1, SSI_IIN195, user->uin, tuin,
            gid, iid, itype, int_itype, auth, 0, 0, item_name2, nickname2, tempst,
	    iperm, privacy, vclass, perms);

   res = PQexec(users_dbconn, msg_buff);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[INSERT SSI_ITEM]");
      return(-1);
   }
   else
   {
      PQclear(res);
   }

   return(0);
}


/**************************************************************************/
/* This function delete new ssi item from database			  */
/**************************************************************************/
int ssi_db_del_item(struct online_user *user, char *item_name,
                    unsigned short gid, unsigned short iid,
		    unsigned short itype, class tlv_chain_c &tlv_chain)
{
   PGresult *res;

   DEBUGADD(50, ("SSI_DEL: ou=%lu,gid=%d,iid=%d,t=%d,tlvnum=%d\n",
              user->uin, gid, iid, itype, tlv_chain.num()));

   /* Now we can define and run sql delete query */

#define SSI_ID292 \
  "DELETE FROM users_ssi_data \
   WHERE ouin=%lu AND gid=%d AND iid=%d AND type=%d AND readonly=0"

   snprintf(msg_buff, sizeof(msg_buff)-1, SSI_ID292, user->uin, gid, iid, itype);

   res = PQexec(users_dbconn, msg_buff);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[DELETE SSI_ITEM]");
      return(-1);
   }
   else
   {
      if (atoul(PQcmdTuples(res)) != 0)
      {
         PQclear(res);
         return(0);
      }
      else
      {
         PQclear(res);
         return(-1);
      }
   }

   return(0);
}


/**************************************************************************/
/* This function update ssi item 					  */
/**************************************************************************/
int ssi_db_update_item(struct online_user *user, char *item_name,
                       unsigned short gid, unsigned short iid,
		       unsigned short itype, class tlv_chain_c &tlv_chain)
{
   PGresult *res;
   unsigned short int_itype = itype_conv[itype];
   unsigned short auth = 0;
   unsigned long  tuin = 0;
   unsigned long  iperm = 0, vclass = 0, perms = 0;
   char privacy = 0;
   class tlv_c *tlv = NULL;
   fstring nickname, nickname2, item_name2;

   strncpy(nickname, "", 64);

   /* Check for buddy item nickname */
   if ((itype == ITEM_BUDDY) && ((tlv = tlv_chain.get(SSI_TLV_NICKNAME)) != NULL))
   {
      if (!v7_extract_string(nickname, *tlv, 128))
      {
         DEBUGADD(0, ("SSI_ERROR: TLV(131) problem... Nickname too big\n"));
         return(False);
      }
      tlv_chain.remove(SSI_TLV_NICKNAME);
   }

   /* Check if this user added with auth flag */
   if ((tlv = tlv_chain.get(SSI_TLV_AUTH)) != NULL)
   {
      auth = 1;
      tlv_chain.remove(SSI_TLV_AUTH);
   }

   /* Check for TLV(C9) in presence/visibility item */
   if ((tlv = tlv_chain.get(SSI_TLV_IDLEPERMS)) != NULL)
   {
      if (itype == ITEM_PRESENCE)
      {
         *tlv >> iperm;
      }

      tlv_chain.remove(SSI_TLV_IDLEPERMS);
   }

   /* Check for TLV(CA) in rights item */
   if ((tlv = tlv_chain.get(SSI_TLV_PRIVACY)) != NULL)
   {
      if (itype == ITEM_RIGHTS)
      {
         *tlv >> privacy;
      }

      tlv_chain.remove(SSI_TLV_PRIVACY);
   }

   /* Check for TLV(CB) in rights item */
   if ((tlv = tlv_chain.get(SSI_TLV_RIGHTS)) != NULL)
   {
      if (itype == ITEM_RIGHTS)
      {
         *tlv >> vclass;
      }

      tlv_chain.remove(SSI_TLV_RIGHTS);
   }

   /* Check for TLV(CC) in rights item */
   if ((tlv = tlv_chain.get(SSI_TLV_PRESENSE)) != NULL)
   {
      if (itype == ITEM_RIGHTS)
      {
         *tlv >> perms;
      }

      tlv_chain.remove(SSI_TLV_PRESENSE);
   }

   /* Check if we should convert target uin value */
   /* I'll add screenname -> uin converter later  */
   tuin = atoul(item_name);

   /* Now time to code remaining TLVs into blob ascii string */
   tlv_chain.encode(tempst);
   convert_to_postgres2(nickname2, sizeof(fstring)-1, nickname);
   convert_to_postgres2(item_name2, sizeof(fstring)-1, item_name);

   DEBUGADD(50, ("SSI_UPDATE: ou=%lu,tu=%lu,gid=%d,iid=%d,it=%d,t=%d,auth=%d,tlvnum=%d\n",
                 user->uin, tuin, gid, iid, int_itype, itype, auth, tlv_chain.num()));

   if (tlv_chain.num()) { DEBUGADD(50, ("Tlv chain string: %s\n", tempst)); }

/* Now we can define and run sql insert db query   */
/* We'll use #define to avoid multistring literals */
#define SSI_IUOA473 \
  "UPDATE users_ssi_data SET \
      auth=%d, \
      iname='%s', \
      nick='%s', \
      blob='%s', \
      iperm=%lu, \
      privacy=%d, \
      vclass=%lu, \
      perms=%lu \
   WHERE (ouin=%lu) AND (iid=%d) AND \
         (gid=%d) AND (type=%d) AND (readonly=0)"

/* do not allow user to change a buddy name */
#define SSI_IUOB473 \
  "UPDATE users_ssi_data SET \
      auth=%d, \
      nick='%s', \
      blob='%s', \
      iperm=%lu, \
      privacy=%d, \
      vclass=%lu, \
      perms=%lu \
   WHERE (ouin=%lu) AND (iid=%d) AND \
         (gid=%d) AND (type=%d) AND (readonly=0)"

   if (itype == ITEM_BUDDY)
   {
      snprintf(msg_buff, sizeof(msg_buff)-1, SSI_IUOB473, auth, nickname2,
               tempst, iperm, privacy, vclass, perms, user->uin, iid, gid, itype);
   }
   else
   {
      snprintf(msg_buff, sizeof(msg_buff)-1, SSI_IUOA473, auth, item_name2, nickname2,
               tempst, iperm, privacy, vclass, perms, user->uin, iid, gid, itype);
   }

   res = PQexec(users_dbconn, msg_buff);
   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[UPDATE SSI_ITEM]");
      return(-1);
   }
   else
   {
      if (atoul(PQcmdTuples(res)) != 0)
      {
         PQclear(res);
         return(0);
      }
      else
      {
         PQclear(res);
         return(1);
      }
   }

   PQclear(res);
   return(0);
}


/**************************************************************************/
/* Check if there was authorization granted for specified user            */
/**************************************************************************/
BOOL db_ssi_get_auth(unsigned long uin, unsigned long tuin)
{
   PGresult *res;
   unsigned short auth=0;
   fstring dbcomm_str;

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT count(*) FROM users_perms WHERE (uin=%lu) AND (tuin=%lu) AND (type=%d)",
	    uin, tuin, PERMS_AUTH);

   res = PQexec(users_dbconn, dbcomm_str);

   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[GET SSI AUTH]");
      return(0);
   }

   if (PQntuples(res) > 0)
   {
      auth = atoul(PQgetvalue(res, 0, 0));

      PQclear(res);

      if (auth == 0)
      {
         return(False);
      }
      else
      {
         return(True);
      }
   }

   PQclear(res);
   return(False);
}


/**************************************************************************/
/* Return item count					                  */
/**************************************************************************/
int db_item_count(unsigned long uin, unsigned short type, int gid, int iid)
{
   PGresult *res;
   fstring dbcomm_str;
   fstring clause_temp;
   int count = 0;

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT count(*) FROM Users_SSI_Data WHERE (ouin=%lu) ", uin);

   if (type > 0)
   {
      snprintf(clause_temp, 255, "AND (type=%d) ", type);
      safe_strcat(dbcomm_str, clause_temp, 255);
   }

   if (gid > 0)
   {
      snprintf(clause_temp, 255, "AND (gid=%d) ", gid);
      safe_strcat(dbcomm_str, clause_temp, 255);
   }

   if (iid > 0)
   {
      snprintf(clause_temp, 255, "AND (iid=%d) ", iid);
      safe_strcat(dbcomm_str, clause_temp, 255);
   }

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[ITEM COUNT]");
      return(False);
   }


   if (PQntuples(res) > 0)
   {
      count = atoul(PQgetvalue(res, 0, 0));
      DEBUG(50, ("Check item count (%lu, type=%d, gid=%d, iid=%d) = %d\n",
                  uin, type, gid, iid, count));

      PQclear(res);
      return(count);
   }

   PQclear(res);
   return(0);
}


/**************************************************************************/
/* This function creates master ssi group for stupid miranda-im client    */
/**************************************************************************/
int db_ssi_check_dgroup(struct online_user *user)
{
   PGresult *res;
   fstring dbcomm_str;
   BOOL ssi_changed = False;

   if (db_item_count(user->uin, ITEM_GROUP, 0, -1) == 0)
   {

      DEBUG(10, ("Master group for %lu doesn't exist. Creating new one...\n", user->uin));
      slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
              "INSERT INTO users_ssi_data VALUES (%lu,0,0,0,%d,%d,0,0,0,'','','',0,0,0,0,0);",
	       user->uin, ITEM_GROUP, 0x0FFF);

      res = PQexec(users_dbconn, dbcomm_str);
      if (PQresultStatus(res) != PGRES_COMMAND_OK)
      {
         handle_database_error(res, "[CREATE MASTER SSI GROUP]");
         return(1);
      }

      ssi_changed = True;
      PQclear(res);
   }

   if (db_item_count(user->uin, ITEM_GROUP, -1, -1) <= 1)
   {
      /* And one more thing... Miranda wants group with gid!=0 and strlen(name) > 0  */
      /* to upload contacts. Why don't to create it ? Miranda bugs make me so angry. */

      DEBUG(10, ("General group for %lu doesn't exist. Creating new one...\n", user->uin));
      slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
              "INSERT INTO users_ssi_data VALUES (%lu,0,1,0,%d,%d,0,0,0,'General','','',0,0,0,0,0);",
	       user->uin, ITEM_GROUP, 0x0FFF);

      res = PQexec(users_dbconn, dbcomm_str);
      if (PQresultStatus(res) != PGRES_COMMAND_OK)
      {
         handle_database_error(res, "[CREATE GENERAL SSI GROUP]");
         return(1);
      }

      ssi_changed = True;
      PQclear(res);
   }

   if (ssi_changed) db_ssi_set_modinfo(user, False);
   return(0);
}


/**************************************************************************/
/* Remove TLV(66) from buddy record (grant authorization)	          */
/**************************************************************************/
BOOL db_ssi_auth_grant(unsigned long fuin, unsigned long tuin)
{
   PGresult *res;
   fstring dbcomm_str;

/* Now we can define and run sql insert db query   */
/* We'll use #define to avoid multistring literals */
#define SSI_UAG648 \
  "UPDATE users_ssi_data SET auth=0 WHERE (ouin=%lu) AND \
      (uin=%lu) AND (type=%d) AND (auth=1)"

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, SSI_UAG648, tuin, fuin, ITEM_BUDDY);
   res = PQexec(users_dbconn, dbcomm_str);

   DEBUG(50, ("AGRANT dbcomm string: %s\n", dbcomm_str));

   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      handle_database_error(res, "[SSI GRANT AUTH]");
      return(False);
   }
   else
   {
      if (atoul(PQcmdTuples(res)) != 0)
      {
         PQclear(res);
         return(True);
      }
      else
      {
         PQclear(res);
         return(False);
      }
   }

   PQclear(res);
   return(False);
}


/**************************************************************************/
/* Add user permission to perms table 				          */
/**************************************************************************/
void db_ssi_auth_add(unsigned long uin, unsigned long tuin, unsigned long rid)
{
   PGresult *res;
   fstring dbcomm_str;

/* Now we can define and run sql insert db query   */
/* We'll use #define to avoid multistring literals */
#define SSI_IAP694 \
  "INSERT INTO Users_Perms VALUES (%lu, %lu, %d, %lu)"

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, SSI_IAP694, uin, tuin, PERMS_AUTH, rid);
   res = PQexec(users_dbconn, dbcomm_str);
   PQclear(res);
}


/**************************************************************************/
/* Check if there was 'You were added' message from this user             */
/**************************************************************************/
BOOL db_ssi_get_added(unsigned long uin, unsigned long by_uin)
{
   PGresult *res;
   unsigned short added=0;
   fstring dbcomm_str;

   DEBUG(50, ("db_ssi_get_added(%lu, %lu) called...\n", uin, by_uin));

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1,
           "SELECT count(*) FROM users_perms WHERE (uin=%lu) AND (tuin=%lu) AND (type=%d)",
	    uin, by_uin, PERMS_ADDED);

   res = PQexec(users_dbconn, dbcomm_str);

   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
      handle_database_error(res, "[GET SSI ADDED]");
      return(0);
   }

   if (PQntuples(res) > 0)
   {
      added = atoul(PQgetvalue(res, 0, 0));

      PQclear(res);

      if (added == 0)
      {
         return(False);
      }
      else
      {
         return(True);
      }
   }

   PQclear(res);
   return(False);
}


/**************************************************************************/
/* Add user 'added' permission to perms table 			          */
/**************************************************************************/
void db_ssi_added_add(unsigned long uin, unsigned long by_uin, unsigned long rid)
{
   PGresult *res;
   fstring dbcomm_str;

/* Now we can define and run sql insert db query   */
/* We'll use #define to avoid multistring literals */
#define SSI_IAP694 \
  "INSERT INTO Users_Perms VALUES (%lu, %lu, %d, %lu)"

   slprintf(dbcomm_str, sizeof(dbcomm_str)-1, SSI_IAP694, uin, by_uin, PERMS_ADDED, rid);
   res = PQexec(users_dbconn, dbcomm_str);
   PQclear(res);
}


