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
/* Various functions to work with strings 				  */
/*                                                                        */
/**************************************************************************/

#include "includes.h"
static char *last_ptr=NULL;

/**************************************************************************/
/* Encode raw data buffer into string (binhex algorihm)                   */
/**************************************************************************/
int hexencode(char *rstr, char *buf, int buf_size) 
{ 
   int sind, rind;
   int slen = buf_size;
   char *hexchars = "0123456789ABCDEF";

   for (sind=0,rind=0;sind<slen;sind++)
   {
      rstr[rind++] = hexchars[(buf[sind] & 0xF0) >> 4];
      rstr[rind++] = hexchars[buf[sind] & 0x0F];      
   }
   
   rstr[rind] = 0;
   return(0);
} 


/**************************************************************************/
/* Decode string coded by hexencode() into raw data buffer                */
/**************************************************************************/
int hexdecode(char *buf, char *sstr, int buflen) 
{ 
   int sind, rind;
   int slen = strlen(sstr);
   char tstr[5];
   tstr[0] = '0';
   tstr[1] = 'x';
   tstr[4] =  0;

   for (rind=0,sind=0;sind<slen;sind++)
   {
      tstr[2] = sstr[sind++];
      tstr[3] = sstr[sind];      
      buf[rind++] = ahextoul(tstr);
      if (rind > buflen) break;
   }
   
   return(rind);
} 


/**************************************************************************/
/* This func compare strlen with given value and send alarm if it greater */
/**************************************************************************/
BOOL islen_valid(int str_len, int limit, char *fname, 
		 struct online_user &user)
{
  if (str_len > limit) 
  { 
     LOG_ALARM(0, ("Warning: User %lu from %s:%ld sent too big %s (%d)...\n", 
              user.uin, inet_ntoa(user.ip), user.udp_port, fname, str_len));
     return(False);
  }
  
  return(True);
}


/**************************************************************************/
/* This func convert string to unsigned long number			  */
/**************************************************************************/
unsigned long atoul(char *string)
{
  char **valid; valid = NULL;
  return(strtoul(string, valid, 10));
}


/**************************************************************************/
/* This func convert hex string to unsigned long number			  */
/**************************************************************************/
unsigned long ahextoul(char *string)
{
   unsigned short i;
   unsigned long  resnum = 0;
   unsigned char  lonybble;
   char           *hexchars = "0123456789ABCDEF";
   char           *p1 = NULL;

   for (i = 2; string[i] != 0; i++)
   {
      if (!(p1 = strchr(hexchars, toupper(string[i]))))
      {
         break;
      }

      /* get the two nybbles */
      lonybble = PTR_DIFF(p1, hexchars);
      resnum  = resnum << 4;
      resnum += lonybble;
   }
   
   return(resnum);
}


/**************************************************************************/
/* This func compare strlen with given value and send alarm if it greater */
/**************************************************************************/
BOOL islen_valid(int str_len, int limit, char *fname, Packet &pack)
{
  if (str_len > limit) 
  { 
     LOG_ALARM(0, ("Warning: New user from %s:%d sent too big %s (%d)...\n", 
                 inet_ntoa(pack.from_ip), pack.from_port, fname, str_len));
     return(False);
  }
  
  return(True);
}


/*************************************************************************/
/* This func convert string to postrgres format (' -> '', \ -> \\ etc)   */
/*************************************************************************/
void convert_to_postgres(char *string, int tsize)
{
   if (strlen(string) > 2048) return;
   
   for (unsigned int i=0, t=0; i<strlen(string)+1; i++, t++)
   {
     tempst3[t]   = string[i];
   
     if ((string[i] == '\\') | 
         (string[i] == '\'') )
     {
       t++; 
       tempst3[t] = string[i];
       if (t > 2047) return;
     }
   }
   
   ITrans.translateToServer(tempst3);
   strncpy(string, tempst3, tsize-1);
   
   return;
}


/*************************************************************************/
/* This func convert string to postrgres format (' -> '', \ -> \\ etc)   */
/*************************************************************************/
void convert_to_postgres(char *target_string, int tsize, char *string)
{
   
   for (unsigned int i=0, t=0; i<strlen(string)+1; i++, t++)
   {
     target_string[t]   = string[i];
   
     if ((string[i] == '\\') | 
         (string[i] == '\'') )
     {
       t++; target_string[t] = string[i];
     }
   }
   
   ITrans.translateToServer(target_string);
   
   return;
}


/*************************************************************************/
/* This func convert string to postrgres format (' -> '', \ -> \\ etc)   */
/*************************************************************************/
void convert_to_postgres2(char *target_string, int tsize, char *string)
{
   
   for (unsigned int i=0, t=0; i<strlen(string)+1; i++, t++)
   {
     target_string[t]   = string[i];
   
     if ((string[i] == '\\') | 
         (string[i] == '\'') )
     {
       t++; target_string[t] = string[i];
     }
   }
   
   return;
}


void set_first_token(char *ptr)
{
	last_ptr = ptr;
}


/**************************************************************************/
/* Get the next token from a string, return False if none found		  */
/* handles double-quotes. 						  */
/**************************************************************************/
BOOL next_token(char **ptr,char *buff,char *sep, size_t bufsize)
{
  char *s;
  BOOL quoted;
  size_t len=1;

  if (!ptr) ptr = &last_ptr;
  if (!ptr) return(False);

  s = *ptr;

  /* default to simple separators */
  if (!sep) sep = " \t\n\r";

  /* find the first non sep char */
  while(*s && strchr(sep,*s)) s++;

  /* nothing left? */
  if (! *s) return(False);

  /* copy over the token */
  for (quoted = False; len < bufsize && *s && (quoted || !strchr(sep,*s)); s++)
    {
	    if (*s == '\"') {
		    quoted = !quoted;
	    } else {
		    len++;
		    *buff++ = *s;
	    }
    }

  *ptr = (*s) ? s+1 : s;  
  *buff = 0;
  last_ptr = *ptr;

  return(True);
}


/**************************************************************************/
/* Convert list of tokens to array; dependent on above routine.		  */
/* Uses last_ptr from above - bit of a hack.				  */
/**************************************************************************/
char **toktocliplist(int *ctok, char *sep)
{
  char *s=last_ptr;
  int ictok=0;
  char **ret, **iret;

  if (!sep) sep = " \t\n\r";

  while(*s && strchr(sep,*s)) s++;

  /* nothing left? */
  if (!*s) return(NULL);

  do {
    ictok++;
    while(*s && (!strchr(sep,*s))) s++;
    while(*s && strchr(sep,*s)) *s++=0;
  } while(*s);

  *ctok=ictok;
  s=last_ptr;

  if (!(ret=iret=(char **)malloc(ictok*sizeof(char *)))) return NULL;
  
  while(ictok--) {    
    *iret++=s;
    while(*s++);
    while(!*s) s++;
  }

  return ret;
}


/**************************************************************************/
/* Case insensitive string compararison					  */
/**************************************************************************/
int StrCaseCmp(const char *s, const char *t)
{
  {
    while (*s && *t && toupper(*s) == toupper(*t))
    {
      s++;
      t++;
    }
    return(toupper(*s) - toupper(*t));
  }
}


/**************************************************************************/
/* Case insensitive string compararison, length limited			  */
/**************************************************************************/
int StrnCaseCmp(const char *s, const char *t, size_t n)
{
  {
    while (n && *s && *t && toupper(*s) == toupper(*t))
    {
      s++;
      t++;
      n--;
    }

    if (n) 
      return(toupper(*s) - toupper(*t));

    return(0);
  }
}


/**************************************************************************/
/* Compare 2 strings 							  */
/**************************************************************************/
BOOL strequal(const char *s1, const char *s2)
{
  if (s1 == s2) return(True);
  if (!s1 || !s2) return(False);
  
  return(StrCaseCmp(s1,s2)==0);
}


/**************************************************************************/
/* Compare 2 strings up to and including the nth char.			  */
/**************************************************************************/
BOOL strnequal(const char *s1,const char *s2,size_t n)
{
  if (s1 == s2) return(True);
  if (!s1 || !s2 || !n) return(False);
  
  return(StrnCaseCmp(s1,s2,n)==0);
}


/**************************************************************************/
/* Compare 2 strings (case sensitive)					  */
/**************************************************************************/
BOOL strcsequal(const char *s1,const char *s2)
{
  if (s1 == s2) return(True);
  if (!s1 || !s2) return(False);
  
  return(strcmp(s1,s2)==0);
}


/**************************************************************************/
/* Convert a string to lower case					  */
/**************************************************************************/
void strlower(char *s)
{
  while (*s)
  {
     size_t skip = 0;
     if( skip != 0 )  s += skip;
     else
     {
       if (isupper(*s))  *s = tolower(*s);
       s++;
     }
  }
}


/**************************************************************************/
/* Convert a string to upper case					  */
/**************************************************************************/
void strupper(char *s)
{
  while (*s)
  {
     size_t skip = 0;
     if( skip != 0 )  s += skip;
     else
     {
       if (islower(*s)) *s = toupper(*s);
       s++;
     }
  }
}


/**************************************************************************/
/* Convert a string to "normal" form					  */
/**************************************************************************/
void strnorm(char *s)
{
  extern int case_default;
  if (case_default == CASE_UPPER)
    strupper(s);
  else
    strlower(s);
}


/**************************************************************************/
/* Check if a string is in "normal" case				  */
/**************************************************************************/
BOOL strisnormal(char *s)
{
  extern int case_default;
  if (case_default == CASE_UPPER)
    return(!strhaslower(s));

  return(!strhasupper(s));
}


/**************************************************************************/
/* String replace							  */
/**************************************************************************/
void string_replace(char *s,char oldc,char newc)
{
  while (*s) {
      if (oldc == *s)
        *s = newc;
      s++;
    }
}


/**************************************************************************/
/* Skip past some strings in a buffer					  */
/**************************************************************************/
char *skip_string(char *buf,size_t n)
{
  while (n--)
    buf += strlen(buf) + 1;
  return(buf);
}


/**************************************************************************/
/* Count the number of characters in a string.				  */
/**************************************************************************/
size_t str_charnum(const char *s)
{
  return strlen(s);
}


/**************************************************************************/
/* Trim the specified elements off the front and back of a string	  */
/**************************************************************************/
BOOL trim_string(char *s,const char *front,const char *back)
{
  BOOL ret = False;
  size_t front_len = (front && *front) ? strlen(front) : 0;
  size_t back_len = (back && *back) ? strlen(back) : 0;
  size_t s_len;

  while (front_len && strncmp(s, front, front_len) == 0)
  {
    char *p = s;
    ret = True;
    while (1)
    {
      if (!(*p = p[front_len]))
        break;
      p++;
    }
  }

  if(back_len)
  {
      s_len = strlen(s);
      while ((s_len >= back_len) && 
             (strncmp(s + s_len - back_len, back, back_len)==0))  
      {
        ret = True;
        s[s_len - back_len] = '\0';
        s_len = strlen(s);
      }
  }

  return(ret);
}


/**************************************************************************/
/* Does a string have any uppercase chars in it?			  */
/**************************************************************************/
BOOL strhasupper(const char *s)
{
  while (*s) 
  {
     size_t skip = 0;
     if( skip != 0 ) s += skip;
     else
     {
       if (isupper(*s))  return(True);
       s++;
     }
  }
  return(False);
}


/**************************************************************************/
/* Does a string have any lowercase chars in it?			  */
/**************************************************************************/
BOOL strhaslower(const char *s)
{
  while (*s) 
  {
     size_t skip = 0;
     if( skip != 0 )  s += skip;
     else
     {
       if (islower(*s))  return(True);
        s++;
     }
  }

  return(False);
}


/**************************************************************************/
/* Find the number of chars in a string					  */
/**************************************************************************/
size_t count_chars(const char *s,char c)
{
   size_t count=0;
   while (*s) 
   {
      size_t skip = 0;
      if( skip != 0 )  s += skip;
      else
      {
        if (*s == c)  count++;
        s++;
      }
  }
  return(count);
}


/**************************************************************************/
/* Return True if a string consists only of one particular character.	  */
/**************************************************************************/
BOOL str_is_all(const char *s,char c)
{
  if(s == NULL) return False;
  if(!*s) return False;
  {
    while (*s)
    {
      size_t skip = 0;
      if( skip != 0 )  s += skip;
      else
      {
        if (*s != c)  return False;
        s++;
      }
    }
  }
  return True;
}


/**************************************************************************/
/* safe string copy into a known length string. maxlength does not	  */
/* include the terminating zero.					  */
/**************************************************************************/
char *safe_strcpy(char *dest,const char *src, size_t maxlength)
{
    size_t len;

    if (!dest)  return NULL;

    if (!src)
    {
       *dest = 0;
       return dest;
    }  

    len = strlen(src);

    if (len > maxlength) len = maxlength;
      
    memcpy(dest, src, len);
    dest[len] = 0;
    return dest;
}  


/**************************************************************************/
/* Safe string cat into a string. maxlength does not			  */
/* include the terminating zero.					  */
/**************************************************************************/
char *safe_strcat(char *dest, const char *src, size_t maxlength)
{
    size_t src_len, dest_len;

    if (!dest)  return NULL;
    if (!src)   return dest;

    src_len = strlen(src);
    dest_len = strlen(dest);

    if (src_len + dest_len > maxlength)
    {
       src_len = maxlength - dest_len;
    }
      
    memcpy(&dest[dest_len], src, src_len);
    dest[dest_len + src_len] = 0;
    return dest;
}


/**************************************************************************/
/* Paranoid strcpy into a buffer of given length (includes terminating	  */
/* zero. Strips out all but 'a-Z0-9' and replaces with '_'.		  */
/**************************************************************************/
char *alpha_strcpy(char *dest, const char *src, size_t maxlength)
{
   size_t len, i;

   if (!dest)  return NULL;

   if (!src)
   {
      *dest = 0;
      return dest;
   }

   len = strlen(src);
   if (len >= maxlength) len = maxlength - 1;

   for(i = 0; i < len; i++)
   {
      int val = (src[i] & 0xff);
      if(isupper(val) ||islower(val) || isdigit(val))
      {
         dest[i] = src[i];
      }
      else
      {
         dest[i] = '_';
      }
   }

   dest[i] = '\0';
   return dest;
}


/**************************************************************************/
/* Like strncpy but always null terminates. Make sure there is room!	  */
/* The variable n should always be one less than the available size.	  */
/**************************************************************************/
char *StrnCpy(char *dest,const char *src,size_t n)
{
  char *d = dest;

  if (!dest) return(NULL);
  if (!src)
  {
     *dest = 0;
     return(dest);
  }

  while (n-- && (*d++ = *src++)) ;

  *d = 0;
  return(dest);
}


/**************************************************************************/
/* Like strncpy but copies up to the character marker. Always null 	  */
/* terminates. Returns a pointer to the character marker in the source 	  */
/* string (src).							  */
/**************************************************************************/
char *strncpyn(char *dest, char *src,size_t n, char c)
{
   char *p;
   size_t str_len;

   p = strchr(src, c);
   if (p == NULL)
   {
      return NULL;
   }

   str_len = PTR_DIFF(p, src);
   strncpy(dest, src, MIN(n, str_len));
   dest[str_len] = '\0';

   return p;
}


/**************************************************************************/
/* Routine to get hex characters and turn them into a 16 byte array.	  */
/* the array can be variable length, and any non-hex-numeric		  */
/* characters are skipped.  "0xnn" or "0Xnn" is specially catered	  */
/* for. Valid examples: "0A5D15"; "0x15, 0x49, 0xa2"; "59\ta9\te3\n"	  */
/**************************************************************************/
size_t strhex_to_str(char *p, size_t len, const char *strhex)
{
   size_t i;
   size_t num_chars = 0;
   unsigned char   lonybble, hinybble;
   char           *hexchars = "0123456789ABCDEF";
   char           *p1 = NULL, *p2 = NULL;

   for (i = 0; i < len && strhex[i] != 0; i++)
   {
      if (strnequal(strhex, "0x", 2))
      {
         i++; /* skip two chars */
	 continue;
      }

      if (!(p1 = strchr(hexchars, toupper(strhex[i]))))
      {
         break;
      }

      i++;

      if (!(p2 = strchr(hexchars, toupper(strhex[i]))))
      {
         break;
      }

      /* get the two nybbles */
      hinybble = PTR_DIFF(p1, hexchars);
      lonybble = PTR_DIFF(p2, hexchars);

      p[num_chars] = (hinybble << 4) | lonybble;
      num_chars++;

      p1 = NULL;
      p2 = NULL;
   }

   return num_chars;
}


void strhex_to_arr(char *p, int len, const char *strhex)
{
   size_t i;
   size_t num_chars = 0;
   unsigned char   lonybble, hinybble;
   char           *hexchars = "0123456789ABCDEF";
   char           *p1 = NULL, *p2 = NULL;

   for (i = 0; (int)i < len && strhex[i] != 0; i++)
   {
      if (!(p1 = strchr(hexchars, toupper(strhex[i]))))
      {
         break;
      }

      i++;

      if (!(p2 = strchr(hexchars, toupper(strhex[i]))))
      {
         break;
      }

      /* get the two nybbles */
      hinybble = PTR_DIFF(p1, hexchars);
      lonybble = PTR_DIFF(p2, hexchars);

      p[num_chars] = (hinybble << 4) | lonybble;
      num_chars++;
      
      if (num_chars > 7) return;

      p1 = NULL;
      p2 = NULL;
   }
}

/**************************************************************************/
/* Check if a string is part of a list					  */
/**************************************************************************/
BOOL in_list(char *s,char *list,BOOL casesensitive)
{
  pstring tok;
  char *p=list;

  if (!list) return(False);

  while (next_token(&p,tok,LIST_SEP,sizeof(tok)))
  {
     if (casesensitive)
     {
        if (strcmp(tok,s) == 0)  return(True);
     }
     else
     {
        if (StrCaseCmp(tok,s) == 0)  return(True);
     }
  }
  return(False);
}


/* this is used to prevent lots of mallocs of size 1 */
static char *null_string = NULL;
/**************************************************************************/
/* Set a string value, allocing the space for the string		  */
/**************************************************************************/
BOOL string_init(char **dest,const char *src)
{
  size_t l;
  if (!src)  src = "";

  l = strlen(src);

  if (l == 0)
  {
     if (!null_string)
     {
        if((null_string = (char *)malloc(1)) == NULL)
        {
           return False;
        }
        *null_string = 0;
      }
      *dest = null_string;
   }
   else
   {
      (*dest) = (char *)malloc(l+1);
      if ((*dest) == NULL)
      {
         return False;
      }

      pstrcpy(*dest,src);
   }
   return(True);
}


/**************************************************************************/
/* Free a string value							  */
/**************************************************************************/
void string_free(char **s)
{
  if (!s || !(*s)) return;
  if (*s == null_string)  *s = NULL;
  if (*s) free(*s);
  *s = NULL;
}


/**************************************************************************/
/* Set a string value, allocing the space for the string, and 		  */
/* deallocating any existing space					  */
/**************************************************************************/
BOOL string_set(char **dest,const char *src)
{
  string_free(dest);

  return(string_init(dest,src));
}


/**************************************************************************/
/* Substitute a string for a pattern in another string. 		  */
/**************************************************************************/
void string_sub(char *s,const char *pattern,const char *insert, size_t len)
{
   char *p;
   ssize_t ls,lp,li, i;

   if (!insert || !pattern || !s) return;

   ls = (ssize_t)strlen(s);
   lp = (ssize_t)strlen(pattern);
   li = (ssize_t)strlen(insert);

   if (!*pattern) return;

   while (lp <= ls && (p = strstr(s,pattern)))
   {
      if (len && ((ls + (li-lp)) >= (int)len))
      {
         break;
      }

      if (li != lp)
      {
         memmove(p+li,p+lp,strlen(p+lp)+1);
      }

      for (i=0;i<li;i++)
      {
         switch (insert[i])
         {
	    case '`' :
	    case '"' :
	    case '\'':
	    case ';' :
	    case '$' :
	    case '%' :
	    case '\r':
	    case '\n': p[i] = '_'; break;
	    default  : p[i] = insert[i];
         }
      }
      s = p + li;
      ls += (li-lp);
   }
}


/**************************************************************************/
/* Substitute a string for a pattern in another string. 		  */
/**************************************************************************/
int count_subs(char *s,const char *pattern)
{
   char *p;
   ssize_t ls,lp,subs_count = 0;

   if (!pattern || !s) return(0);

   ls = (ssize_t)strlen(s);
   lp = (ssize_t)strlen(pattern);
   if (ls == 0) return(0);
   if (lp == 0) return(0);

   while (lp <= ls && (p = strstr(s,pattern)))
   {
      subs_count++;
      s = p + lp;
      ls -= lp;
   }
   
   DEBUG(450, ("subs: ptt=\"%s\", cnt=%d\n", pattern, subs_count));
   return(subs_count);
}


void fstring_sub(char *s,const char *pattern,const char *insert)
{
	string_sub(s, pattern, insert, sizeof(fstring));
}


void pstring_sub(char *s,const char *pattern,const char *insert)
{
	string_sub(s, pattern, insert, sizeof(pstring));
}


/***************************************************************************/
/* Similar to string_sub() but allows for any character to be substituted. */
/***************************************************************************/
void all_string_sub(char *s,const char *pattern,const char *insert, size_t len)
{
   char *p;
   ssize_t ls,lp,li;

   if (!insert || !pattern || !s) return;

   ls = (ssize_t)strlen(s);
   lp = (ssize_t)strlen(pattern);
   li = (ssize_t)strlen(insert);

   if (!*pattern) return;
	
   while (lp <= ls && (p = strstr(s,pattern)))
   {
      if (len && (ls + (li-lp) >= (int)len))
      {
         break;
      }

      if (li != lp)
      {
         memmove(p+li,p+lp,strlen(p+lp)+1);
      }

      memcpy(p, insert, li);
      s = p + li;
      ls += (li-lp);
   }
}


/**************************************************************************/
/* Splits out the front and back at a separator.			  */
/**************************************************************************/
void split_at_first_component(char *path, char *front, char sep, char *back)
{
   char *p = strchr(path, sep);

   if (p != NULL)  *p = 0;
   if (front != NULL) pstrcpy(front, path);

   if (p != NULL)
   {
      if (back != NULL)
      {
         pstrcpy(back, p+1);
      }

      *p = sep;
   }
   else
   {
      if (back != NULL)
      {
         back[0] = 0;
      }
   }
}


/**************************************************************************/
/* Splits out the front and back at a separator.			  */
/**************************************************************************/
void split_at_last_component(char *path, char *front, char sep, char *back)
{
   char *p = strrchr(path, sep);

   if (p != NULL)
   {
      *p = 0;
   }

   if (front != NULL)
   {
      pstrcpy(front, path);
   }
   else if (back != NULL)
   {
      pstrcpy(back, path);
   }

   if (p != NULL)
   {
      if (back != NULL)
      {
         pstrcpy(back, p+1);
      }
      *p = sep;
   }
   else
   {
      if (back != NULL && front != NULL)
      {
         back[0] = 0;
      }
   }
}


/**************************************************************************/
/* Truncate a string at a specified length				  */
/**************************************************************************/
char *string_truncate(char *s, int length)
{
   if (s && ((int)strlen(s) > length))  s[length] = 0; 
   return s;
}


/**************************************************************************/
/* Convert number 2 string						  */
/**************************************************************************/
char *n2str(unsigned long num)
{
   static cstring result;

   snprintf(result, sizeof(cstring)-1, "%lu", num);
   return(result);       
}


/*************************************************************************/
/* This func convert string to unicode					 */
/*************************************************************************/
void convert_to_unicode(char *string, int tsize)
{
#ifdef HAVE_ICONV

   if (strlen(string) > 2048) return;

   char from_locale[255];

   strncpy(from_locale, lp_v7_table_charset(), 254);

   iconv_t cd;
   size_t f, t;
   const char* in = string;
   char b[2049];
   char* out = b;

   memset(&b,0,2049);

   cd = iconv_open("UTF-8", from_locale);
   if( cd == (iconv_t)(-1) )
    return;

   t = 2048;
   f = strlen(string);

   iconv(cd, &in, &f, &out, &t);

   iconv_close(cd);

   safe_strcpy(string,b,tsize);

#endif
}


/*************************************************************************/
/* This func convert string from unicode				 */
/*************************************************************************/
void convert_from_unicode(char *string, int tsize)
{
#ifdef HAVE_ICONV
   if (strlen(string) > 2048) return;

   char to_locale[255];

   strncpy(to_locale, lp_v7_table_charset(), 254);

   iconv_t cd;
   size_t f, t;
   const char* in = string;
   char b[2049];
   char* out = b;

   memset(&b,0,2049);

   cd = iconv_open(to_locale, "UTF-8");
   if( cd == (iconv_t)(-1) )
    return;

   t = 2048;
   f = strlen(string);

   iconv(cd, &in, &f, &out, &t);

   iconv_close(cd);

   safe_strcpy(string,b,tsize);

#endif
}
