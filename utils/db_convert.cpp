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
/* This program will convert your Micro$oft Access database to Postgress  */
/* sql users database. First of all you'll need export Access database    */
/* to text file with symbol ';' as delimiter. 				  */
/*									  */
/* Open ICQSDB.mdb, highlight icqusers_tb table, then select menu 	  */
/* file/export. In appeared window choose save as text file after that 	  */
/* you can copy it on your Unix machine and start converting.		  */
/*									  */
/* Before converting you can choose apropriate translate file from 	  */
/* translate directory							  */
/*									  */
/**************************************************************************/

#include "dincludes.h"

int fdbtxt;			 /* src database filehandler		  */
int readcnt;

int lines		= 0;	 /* lines processed			  */
int users		= 0;	 /* users processed (total)               */
int deadusers		= 0;	 /* number of "dead" users found          */
int newusers		= 0;	 /* number of new users found             */
int fields_total	= 0;	 /* number of filled fields in db         */
int fields_filled	= 0;	 /* number of unfilled fields in db       */
int bytes_processed	= 0;	 /* total bytes processed                 */
int fbuffer_idx		= 0;	 /* buffer index                          */
int rec_number		= 0;	 /* current record number                 */
int fld_number		= 0;	 /* current field number                  */
int curr_fld_idx	= 0;	 /* position in current field             */
int is_txt_fld		= 0;	 /* if we are in text mode                */
int was_text_mode	= 0;
int progres;
int fullness;			 /* percent of filled fields              */
char tchar;			 /* temp char variable                    */

/* Parse progres variables */
char trans_map_name[128];	  /* name of translation table            */
ITranslator ITrans;		  /* locale translate utility             */
char temp_str[128];		  /* temp string for term_write           */

char  db_fname[128];		  /* src database filename                */
char  mdb_port[32] = "\0";		  /* mutable postgres db port             */
char  mdb_user[32] = "\0";		  /* mutable postgres db username         */
char  mdb_addr[128] = "\0";		  /* mutable postgres db ipaddr           */
char  mdb_pass[32] = "\0";		  /* mutable postgres db password         */

char parse_str[255];		  /* string for parser                    */
char fbuffer[513];		  /* buffer for file parser               */

struct stat dbstat;
PGconn *users_dbconn  = NULL;

struct full_user_info valid_user; /* full validated userinfo record       */
fld_string dmb_usr[FIELDS_NUMBER];


/**************************************************************************/
/* This function add one message to console		        	  */
/**************************************************************************/
int term_write_mess(char *mess)
{
  printf("%s\n", mess);
  return(0);
}


/**************************************************************************/
/* This called when user record field contain illegal data 		  */
/**************************************************************************/
void term_field_error(int num)
{
  snprintf(temp_str, 128, "Unexpected data in field number %d", num);
  term_write_mess(temp_str);
  snprintf(temp_str, 128, "I'll skip this record (line number = %d)", lines);
}


/**************************************************************************/
/* This function write stat number to status window			  */
/**************************************************************************/
void write_status(int plines, int pfound, int dead_usr, int new_usr,
		  int fullness)
{
 printf("--------------------------------------------------\n");
 printf("==> Lines processed:   %06d\n", plines);
 printf("==> Users found:       %06d\n", pfound);
 printf("==> Dead users found:  %06d\n", dead_usr);
 printf("==> New users found:   %06d\n", new_usr);
 printf("==> Database fullness: %06d\n", fullness);
 printf("--------------------------------------------------\n");
}


/**************************************************************************/
/* Ask confirmation from user						  */
/**************************************************************************/
int user_confirm()
{
   printf("Are you sure ?: ");

   for(;;)
   {
     tchar = getchar();
     if ((tchar == 'Y') | (tchar == 'y')) return(1);
     if ((tchar == 'N') | (tchar == 'n')) return(0);
   }

   return(1);
}


/**************************************************************************/
/* Converter exit function - after call it allow user scroll 		  */
/* terminal window and exit if he (she)	pressed ESC			  */
/**************************************************************************/
void conv_exit()
{
  write_status(lines, users, deadusers, newusers, fullness);
  exit(0);
}


/**************************************************************************/
/* Small util func. 						   	  */
/**************************************************************************/
int is_bool(unsigned long num)
{
  if ((num == 1) | (num == 0)) return (1);
  return (0);
}


/**************************************************************************/
/* This function validate records in dmb_usr and move them to 	   	  */
/* full_user_info record 					   	  */
/**************************************************************************/
int is_valid_user(struct full_user_info &full_user)
{
  long temp_l;
  char **valid;

  /* user's uin number */
  if ( strcmp(dmb_usr[0],"") == 0 )
  {
    term_field_error(1);
    return (0);
  }
  else
  {
    valid = NULL;
    full_user.uin = strtoul(dmb_usr[0], valid, 10);
    if (valid != NULL)
    {
      term_field_error(1);
      return (0);
    }
  }

  /* user's last ip_addr (number) */
  if ( strcmp(dmb_usr[1], "") != 0 )
  {
    valid = NULL;
    temp_l = strtol(dmb_usr[1], valid, 10);
    if (valid != NULL)
    {
      term_field_error(2);
      return (0);
    }
    memcpy(&(full_user.ip_addr), &temp_l, sizeof(temp_l));
  }
  else
  {
    full_user.ip_addr = 0;
  }

  /* user's broadcast possibility (0/1) */
  if (strcmp(dmb_usr[2],"") == 0)
  {
    term_field_error(3);
    return 0;
  }
  else
  {
    full_user.can_broadcast = atoi(dmb_usr[2]);
    if (!is_bool(full_user.can_broadcast))
    {
      term_field_error(3);
      return (0);
    }
  }

  /* if user should change password (0/1) */
  if ( strcmp(dmb_usr[3],"") == 0)
  {
    term_field_error(4);
    return 0;
  }
  else
  {
    full_user.ch_password = atoi(dmb_usr[3]);
    if (!is_bool(full_user.ch_password))
    {
      term_field_error(4);
      return (0);
    }
  }

  /* lock account field (0/1) */
  if (strcmp(dmb_usr[4],"") == 0)
  {
    term_field_error(5);
    return 0;
  }
  else
  {
    full_user.disabled = atoi(dmb_usr[4]);
    if (!is_bool(full_user.disabled))
    {
      term_field_error(5);
      return (0);
    }
  }

  /* lastlogin time */
  if (strcmp(dmb_usr[5],"") != 0)
  {
    valid = NULL;
    full_user.lastlogin = strtoul(dmb_usr[5], valid, 10);
    if (valid != NULL)
    {
      term_field_error(6);
      return (0);
    }
  }
  else
  {
    full_user.lastlogin = -1;
  }

  /* create date & time */
  if (strcmp(dmb_usr[6],"") != 0)
  {
    valid = NULL;
    full_user.cr_date = strtoul(dmb_usr[6], valid, 10);
    if (valid != NULL)
  {
      term_field_error(7);
      return (0);
    }
  }
  else
  {
    full_user.cr_date = -1;
  }

  /* text fields */
  strncpy(full_user.passwd, dmb_usr[7], 32);
  ITrans.translateToServer(full_user.passwd);   /* bad idea ?? */
  strncpy(full_user.nick, dmb_usr[8], 32);
  ITrans.translateToServer(full_user.nick);
  strncpy(full_user.first, dmb_usr[9], 32);
  ITrans.translateToServer(full_user.first);
  strncpy(full_user.last, dmb_usr[10], 32);
  ITrans.translateToServer(full_user.last);
  strncpy(full_user.email1, dmb_usr[11], 32);
  ITrans.translateToServer(full_user.email1);
  strncpy(full_user.email2, dmb_usr[11], 32);
  ITrans.translateToServer(full_user.email2);

  /* if authorization needed */
  if (strcmp(dmb_usr[12], "")==0)
  {
    term_field_error(13);
    return 0;
  }
  else
  {
    full_user.auth = atoi(dmb_usr[12]);
    if (!is_bool(full_user.auth))
    {
      term_field_error(13);
      return(0);
    }
  }

  /* user's gender */
  if (strcmp(dmb_usr[13],"") == 0)
  {
    term_field_error(14);
    return 0;
  }
  else
  {
    full_user.gender = atoi(dmb_usr[13]);
        if (!is_bool(full_user.gender) & (full_user.gender != -1) &
	   (full_user.gender != 2))
    {
      term_field_error(14);
      return(0);
    }
  }

  /* user's age */
  if (strcmp(dmb_usr[14],"") != 0)
  {
    valid = NULL;
    full_user.age = strtol(dmb_usr[14], valid, 10);
    if (valid != NULL)
    {
      term_field_error(15);
      return (0);
    }
  }
  else
  {
    full_user.age = -1;
  }

  /* user's birth day */
  if (strcmp(dmb_usr[15],"") != 0)
  {
    valid = NULL;
    full_user.bday = strtol(dmb_usr[15], valid, 10);
    if (valid != NULL)
    {
      term_field_error(16);
      return (0);
    }
  }
  else
  {
    full_user.bday = -1;
  }

  /* user's birth month */
  if (strcmp(dmb_usr[16],"") != 0)
  {
    valid = NULL;
    full_user.bmonth = strtol(dmb_usr[16], valid, 10);
    if (valid != NULL)
    {
      term_field_error(17);
      return (0);
    }
  }
  else
  {
    full_user.bmonth = -1;
  }

  /* user's birth year */
  if (strcmp(dmb_usr[17],"") != 0)
  {
    valid = NULL;
    full_user.byear = strtol(dmb_usr[17], valid, 10);
    if (valid != NULL)
    {
      term_field_error(18);
      return (0);
    }
  }
  else
  {
    full_user.byear = -1;
  }

   /* work company details */
  strncpy(full_user.waddr, dmb_usr[18], 64);
  ITrans.translateToServer(full_user.waddr);
  strncpy(full_user.wcity, dmb_usr[19], 32);
  ITrans.translateToServer(full_user.wcity);
  strncpy(full_user.wstate, dmb_usr[20], 32);
  ITrans.translateToServer(full_user.wstate);

  if (strcmp(dmb_usr[21],"") != 0)
  {
    valid = NULL;
    full_user.wcountry = strtol(dmb_usr[21], valid, 10);
    if (valid != NULL)
    {
      term_field_error(22);
      return (0);
    }
  }
  else
  {
    full_user.wcountry = -1;
  }

  strncpy(full_user.wcompany,  dmb_usr[22], 32);
  ITrans.translateToServer(full_user.wcompany);
  strncpy(full_user.wtitle,    dmb_usr[23], 32);
  ITrans.translateToServer(full_user.wtitle);
  strncpy(full_user.wdepart,   dmb_usr[24], 32);
  ITrans.translateToServer(full_user.wdepart);
  strncpy(full_user.wphone,    dmb_usr[25], 32);
  ITrans.translateToServer(full_user.wphone);
  strncpy(full_user.wfax,      dmb_usr[26], 32);
  ITrans.translateToServer(full_user.wfax);
  strncpy(full_user.wpager,    dmb_usr[27], 32);
  ITrans.translateToServer(full_user.wpager);
  strncpy(full_user.wzip,      dmb_usr[28], 32);
  ITrans.translateToServer(full_user.wzip);
  strncpy(full_user.wpage,     dmb_usr[29], 128);
  ITrans.translateToServer(full_user.wpage);

  /* home details */
  strncpy(full_user.haddr,  dmb_usr[31], 64);
  ITrans.translateToServer(full_user.haddr);
  strncpy(full_user.hcity,  dmb_usr[32], 32);
  ITrans.translateToServer(full_user.hcity);
  strncpy(full_user.hstate, dmb_usr[33], 32);
  ITrans.translateToServer(full_user.hstate);

  if (strcmp(dmb_usr[34],"") != 0)
  {
    valid = NULL;
    full_user.hcountry = strtol(dmb_usr[34], valid, 10);
    if (valid != NULL)
    {
      term_field_error(35);
      return (0);
    }
  }
  else
  {
    full_user.hcountry = -1;
  }

  strncpy(full_user.hphone,dmb_usr[35], 32);
  ITrans.translateToServer(full_user.hphone);
  strncpy(full_user.hfax,  dmb_usr[36], 32);
  ITrans.translateToServer(full_user.hfax);
  strncpy(full_user.hcell, dmb_usr[37], 32);
  ITrans.translateToServer(full_user.hcell);
  strncpy(full_user.hzip,  dmb_usr[38], 32);
  ITrans.translateToServer(full_user.hzip);
  strncpy(full_user.hpage, dmb_usr[39], 128);
  ITrans.translateToServer(full_user.hpage);

  /* user's notes */
  strncpy(full_user.notes, dmb_usr[30], 255);
  ITrans.translateToServer(full_user.notes);

  /* notes update time */
  if (strcmp(dmb_usr[40],"") != 0)
  {
    valid = NULL;
    full_user.nupdate = strtoul(dmb_usr[40], valid, 10);
    if (valid != NULL)
    {
      term_field_error(41);
      return (0);
    }
  }
  else
  {
    full_user.nupdate = 0;
  }

  strncpy(full_user.email3, "", 64);
  full_user.pemail1 = 0;
  full_user.gmt_offset = 0;
  full_user.wocup = -1;

  return (1);
}


/**************************************************************************/
/* This function add user record to database			   	  */
/**************************************************************************/
int insert_new_user(struct full_user_info &user)
{
  PGresult *res;

  typedef char ins_string[60000];
  ins_string insert_str;

#define ADD_CVA352 \
"INSERT INTO Users_Info_Ext VALUES \
 ( \
    '%lu', '%s', '%d', '%lu', '%lu', '%d', '%lu', '%d', '%s', '%s', \
    '%s', '%s', '%s', '%s', '%d', '%d', '%d', '%d', '%d', '%d', \
    '%d', '%d', '%s', '%s', '%s', '%d', '%s', '%s', '%d', '%s', \
    '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%d', \
    '%s', '%s', '%s', '%s', '%s', '%lu' \
 )"

  /* now prepare sql command string */
  snprintf(insert_str, 60000, ADD_CVA352,
        user.uin, user.passwd, user.disabled, user.lastlogin, user.ip_addr,
	user.can_broadcast, user.cr_date, user.ch_password, user.nick,
	user.first, user.last, user.email1, user.email2, user.email3,
	user.pemail1, user.gmt_offset, user.auth, user.gender, user.age,
	user.bday, user.bmonth, user.byear, user.waddr, user.wcity,
	user.wstate, user.wcountry, user.wcompany, user.wtitle, user.wocup,
	user.wdepart, user.wphone, user.wfax, user.wpager, user.wzip,
	user.wpage, user.notes, user.haddr, user.hcity, user.hstate,
	user.hcountry, user.hphone, user.hfax, user.hcell, user.hzip,
	user.hpage, user.nupdate);

      res = PQexec(users_dbconn, insert_str);
      if (PQresultStatus(res) != PGRES_COMMAND_OK)
      {
         snprintf(temp_str, 128, "Error number: %d", PQresultStatus(res));
	 term_write_mess(temp_str);
         term_write_mess("SQL command failed...");
	 term_write_mess((char *)PQresultErrorMessage(res));
         PQclear(res);
	 conv_exit();
      }
      else
      {
         PQclear(res);
      }

      return(0);
}


/**************************************************************************/
/* This function will move all records from users_info_ext table	  */
/* to old_users_info table, if old_users_info table exist, it 		  */
/* will be deleted 					      		  */
/**************************************************************************/
void move_tbl2old()
{
   PGresult *res;

   cmd_string dbcomm_str;
   snprintf(dbcomm_str, 255, "DROP TABLE old_users_info_ext");
   res = PQexec(users_dbconn, dbcomm_str); PQclear(res);

   snprintf(dbcomm_str, 255, "SELECT * INTO TABLE Old_Users_Info FROM Users_Info_Ext");
   res = PQexec(users_dbconn, dbcomm_str);

   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      term_write_mess("Can't copy old records to 'old_users_info'...\n");
      term_write_mess((char *)PQresultErrorMessage(res));
      term_write_mess("Press Q, Enter or Space to exit...");
      PQclear(res);
      conv_exit();
   }
   else PQclear(res);

   snprintf(dbcomm_str, 255, "DELETE FROM Users_Info_Ext");
   res = PQexec(users_dbconn, dbcomm_str);

   if (PQresultStatus(res) != PGRES_COMMAND_OK)
   {
      term_write_mess("Can't delete old records from 'users_info'...\n");
      term_write_mess((char *)PQresultErrorMessage(res));
      term_write_mess("Press Q, Enter or Space to exit...");
      PQclear(res);
      conv_exit();
   }
   else
   {
      snprintf(temp_str, 128, "Moved %s records to 'old_users_info'...",
      PQcmdTuples(res));
      PQclear(res);
   }

   return;
}


/**************************************************************************/
/* This function check if there is any user record in postrgres db 	  */
/* and return number of records in table 			   	  */
/**************************************************************************/
int is_table_filled()
{
   PGresult *res;
   int number;

   cmd_string dbcomm_str;
   snprintf(dbcomm_str, sizeof(dbcomm_str)-1, "SELECT uin FROM Users_Info_Ext");

   res = PQexec(users_dbconn, dbcomm_str);
   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
       term_write_mess("SQL select command failed...\n");
      term_write_mess((char *)PQresultErrorMessage(res));
      PQclear(res);
      conv_exit();
   }

   number = PQntuples(res);
   if (number != 0)
   {
     snprintf(temp_str, 128, "Warning: Target table contain %d records.",
              number);
     term_write_mess(temp_str);
   }
   PQclear(res);
   return(number);
}


/**************************************************************************/
/* This function make connection to users database		 	  */
/**************************************************************************/
void usersdb_connect()
{
   char	    *pghost, *pgport,  *pgoptions, *pgtty;
   char	    *dbName, *dblogin, *dbpassw;

   pghost  = mdb_addr;		/* address of db server if NULL - unix */
   pgport  = mdb_port;		/* port of the backend */
   dbName  = "ICQ";			/* online database name */
   dblogin = mdb_user;		/* database user */
   dbpassw = mdb_pass;		/* database password */

   pgoptions = NULL;		/* special options for backend server */
   pgtty     = NULL;		/* debugging tty for the backend server */


				/* make a connection to the database */
   users_dbconn = PQsetdbLogin(pghost, pgport, pgoptions, pgtty, dbName,
                               dblogin, dbpassw);

   if (PQstatus(users_dbconn) == CONNECTION_BAD)
   {
      term_write_mess("Connection to users database failed.");
      term_write_mess(PQerrorMessage(users_dbconn));
      conv_exit();

   }
   else
   {
      term_write_mess("Successfully connected to postgres users db.");
   }
}


/**************************************************************************/
/* text db parser						 	  */
/**************************************************************************/
void parse_db_file(int fdbh)
{
  /* now we should determine file size */
  if (fstat(fdbh, &dbstat) == -1)  {
    term_write_mess("ERROR: Can't stat src database file.");
    snprintf(temp_str, 128, "Returned error: %s", strerror(errno));
    term_write_mess(temp_str);
    conv_exit();
  }

  /* print terminal message for user */
  snprintf(temp_str, 128, "Info: Opened file size = %d bytes",
          (int)(dbstat.st_size));
  term_write_mess(temp_str);

  for (;;)
  {
    readcnt = read(fdbh, fbuffer, 513);	/* read first portion of file. */
    if (readcnt == 1) break;		/* we are using seek(-1) :) */
    if (readcnt < 513) readcnt++;	/* correct counter for last block */
    lseek(fdbh, -1, SEEK_CUR);

    /* i don't want waist time on error handling */
    if (readcnt == -1)
    {
      term_write_mess("Error encountered while read block from file...");
      snprintf(temp_str, 128, "Error: \'%s\'", strerror(errno));
      term_write_mess("Press any key to exit...");
      conv_exit();
    }

    readcnt--;

    /* Now we should parse ready block */
    for (int i=0; i<readcnt; i++, bytes_processed++)
    {
       /* increment number of processed lines */
       if (fbuffer[i] == 0x0a) lines++;

       /* i've split all parse function into two parts - text mode and */
       /* non-text mode. Text mode - if we are located between quotes, */
       /* and non-text mode if we are out. In text mode we can skip ';'*/
       /* 0x0a, 0x0d handling, we only should handle ", "" and \\      */
       if (is_txt_fld)
       {
          /* is it single quote - end text mode char ? */
          if ((fbuffer[i] == '"') & (fbuffer[i+1]) != '"')
	  {
	    is_txt_fld = 0;
	    was_text_mode = 1;
	    continue;
	  }

	  /* is it covered dual quote ? */
	  if ((fbuffer[i] == '"') & (fbuffer[i+1] == '"'))
	  {
	    dmb_usr[fld_number][curr_fld_idx] = '"';
	    if (curr_fld_idx+1 < 255) {curr_fld_idx++; } else
	    {
	       term_write_mess("Error: field too big or db structure error");
	       conv_exit();
	    }

	    if (i+1 == readcnt) { lseek(fdbh, 1, SEEK_CUR); } else i+=1;

	    continue;
	  }

	  /* it is only data char */
	  dmb_usr[fld_number][curr_fld_idx] = fbuffer[i];

	  /* we should cover single quote with another one */
	  if (fbuffer[i] == 0x27)
	  {
 	     curr_fld_idx++;
	     dmb_usr[fld_number][curr_fld_idx] = 0x27;;
	  }

	  /* we also should cover backslash */
	  if (fbuffer[i] == 0x5C)
	  {
 	     curr_fld_idx++;
	     dmb_usr[fld_number][curr_fld_idx] = 0x5C;;
	  }

	  /* sanity checks */
	  if (curr_fld_idx < 255) {curr_fld_idx++; continue; } else
	  {
	     term_write_mess("Error: field too big or db structure error");
	     conv_exit();
	  }

       } else {		/* we are in non-text mode */

	 /* is it single quote ? */
         if (fbuffer[i] == '"')
	 {
	   if (was_text_mode)
	   {
	     snprintf(temp_str, 128, "Error in db structure [str: %d, byte: %d]", lines, bytes_processed);
	     term_write_mess(temp_str);
	     term_write_mess("Press any key to exit...");
	     conv_exit();

	   }
	   is_txt_fld = 1;
	   continue;
	 }

	 if (fbuffer[i] == 0x0d) continue;  /* we don't need this char */

	 /* is it delimiter ? */
	 if (fbuffer[i] == ';')
	 {
	   was_text_mode = 0;
	   fields_total++;

	   /* check if this field is filled */
	   if (strlen(dmb_usr[fld_number]) > 1) fields_filled++;
	   fld_number++;

	   /* sanity check */
	   if (fld_number > FIELDS_NUMBER)
	   {
	     snprintf(temp_str, 128, "Too many fields in record [str: %d, byte: %d]",
	             lines, bytes_processed);
	     term_write_mess(temp_str);
	     term_write_mess("Press any key to exit...");
	     conv_exit();
	   }

	   curr_fld_idx = 0;
	   continue;
	 }

	 /* is it end-user delimiter ? */
	 if (fbuffer[i] == 0x0a)
	 {
	   users++;

	   /* if this user never login (ip_addr field == 0) */
	   if (strlen(dmb_usr[1]) < 1) newusers++;

	   /* write to term uin, nick, name and lname */

           snprintf(temp_str, 128, "Found user: %s [%s %s]",
	                     dmb_usr[0], dmb_usr[9], dmb_usr[10]);

	   ITrans.translateToServer(temp_str);
	   term_write_mess(temp_str);

	   if (is_valid_user(valid_user)) insert_new_user(valid_user);

	   /* we should clear fields array for next user data */
	   for (int is=0;is<FIELDS_NUMBER;is++) memset(dmb_usr[is], 0, 255);


	   fld_number = 0;
	   curr_fld_idx = 0;
	   was_text_mode = 0;

	   continue;
	 }

	 /*in sert char in non-text mode - usually numbers */
	 dmb_usr[fld_number][curr_fld_idx] = fbuffer[i];

	 /* sanity check */
	 if (curr_fld_idx+1 < 255) {curr_fld_idx++; continue; } else
	 {
	    term_write_mess("Error: field too big or db structure error");
	    conv_exit();
	 }
       } /* else !txt_mode */
    } /* block for end bracket */
  }   /* read for end bracket */
}


/**************************************************************************/
/* Displays usage infornation				    		  */
/**************************************************************************/
void usage_info_print()
{
    printf("Usage:    iserver_db_convert -d db_file [options] \n");
    printf("Please set port, user, address and password for DB\n");
    printf("Options:  -x <tblname> [ locale translate table name ]\n");
    printf("          -a <addr>    [ database server address ]\n");
    printf("          -p <prt>     [ database server port ]\n");
    printf("          -u <user>    [ database user name ]\n");
    printf("          -w <pass>    [ database user password ]\n");
    exit(-1);
}


/**************************************************************************/
/* Program entry point					    		  */
/**************************************************************************/
int main(int argc, char **argv, char **envp)
{
  char         flag;
  extern char  *optarg;
  int 	       no_db_name = 1;

  /* OPTSTR  "d:x:p:u:a:w:" */
  while ((flag = getopt(argc, argv, OPTSTR)) != (int)EOF)
  {
     switch (flag)
     {
	 /* translate table name option */
         case 'x':
	    snprintf(trans_map_name, sizeof(trans_map_name), optarg);
            break;

	 /* database filename name option */
	 case 'd':
	    no_db_name = 0;
	    snprintf(db_fname, sizeof(db_fname), optarg);
	    break;

	 /* database port number option */
         case 'p':
	    snprintf(mdb_port, sizeof(mdb_port), optarg);
            break;

	 /* database username option */
         case 'u':
	    snprintf(mdb_user, sizeof(mdb_user), optarg);
            break;

	 /* database addres option */
         case 'a':
            snprintf(mdb_addr, sizeof(mdb_addr), optarg);
	    break;

	 case 'w':
	    snprintf(mdb_pass, sizeof(mdb_pass), optarg);
	    break;

	 /* default action */
         case '?':
         default:  usage_info_print();

     }
  }

  /* Check for database filename */
  if (no_db_name ||
    (mdb_port == NULL) || (mdb_port[0] == '\0') ||
    (mdb_user == NULL) || (mdb_user[0] == '\0') ||
    (mdb_addr == NULL) || (mdb_addr[0] == '\0') ||
    (mdb_pass == NULL) || (mdb_pass[0] == '\0')) usage_info_print();

  term_write_mess("IServerd database converter started");
  term_write_mess("--------------------------------------------------");

  /* translator initialization */
  snprintf(temp_str, 128, "Translate file: %s", trans_map_name);
  term_write_mess(temp_str);

  snprintf(temp_str, 128, TRANSLATE_FILE, trans_map_name);
  ITrans.setTranslationMap(temp_str);

  /* Now it is time to open db file... :) */
  fdbtxt = open(db_fname, O_RDONLY);
  if (fdbtxt == -1) {
     term_write_mess("Error: Can't open database file. ");
     snprintf(temp_str, 128, "Error: \'%s\'", strerror(errno));
     term_write_mess(temp_str);
     term_write_mess("Press any key to finish.");
     conv_exit();
  }
  else
  {
     term_write_mess("Successfully opened database file...");
  }

  /* trying to connect to database */
  usersdb_connect();
  if (is_table_filled())
  {
    if (!user_confirm())
    {
      term_write_mess("Press space or enter to exit...");
      conv_exit();
    }

    term_write_mess("Moving records to old_users...");
    move_tbl2old();

  }
  else
  {
    term_write_mess("Target table clean. Continue convertation...");
  }

  parse_db_file(fdbtxt);

  term_write_mess("Convertation successfully finished.");
  term_write_mess("Press any key to exit.");

  /* Now we should close all windows */
  conv_exit();
}

