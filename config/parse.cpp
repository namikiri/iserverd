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
/*                                                                        */
/**************************************************************************/

#include "includes.h"
#define BUFR_INC 1024

static char *bufr  = NULL;
static int   bSize = 0;


typedef struct
{

   char *buf;
   char *p;
   size_t size;

} myFILE;


static int mygetc(myFILE *f)
{
   if (f->p >= f->buf+f->size) return EOF;
   return (int)*(f->p++);
}

static void myfile_close(myFILE *f)
{
   if (!f) return;
   if (f->buf) free(f->buf);
   free(f);
}

/* Delete spaces from line */
static int EatWhitespace( myFILE *InFile )
{
   int c;
   for( c = mygetc( InFile );
        isspace( c ) && ('\n' != c);
        c = mygetc( InFile ) );

   return( c );
}


/* Delete comment from line */
static int EatComment( myFILE *InFile )
{
   int c;
   for( c = mygetc( InFile );
	('\n'!=c) && (EOF!=c) && (c>0);
	c = mygetc( InFile ) );

   return( c );
}

/* Determine \ as string continuation */
static int Continuation( char *line, int pos )
{
   int pos2 = 0;
   pos--;
   while( (pos >= 0) && isspace(line[pos]) )
   pos--;

   while(pos2 <= pos)
   {
      size_t skip = 0;
      skip = 0;
      if (skip)
      {
        pos2 += skip;
      }
      else if (pos == pos2)
      {
         return( ((pos >= 0) && ('\\' == line[pos])) ? pos : -1 );
      }
      else
      {
         pos2++;
      }
   }
   return (-1);
}


/* Process section */
static BOOL Section( myFILE *InFile, BOOL (*sfunc)(char *) )
{
   int   c;
   int   i;
   int   end;

   i = 0; end = 0;
   c = EatWhitespace( InFile );

   while( (EOF != c) && (c > 0) )
   {
      if( i > (bSize - 2) )
      {
         bSize += BUFR_INC;
         bufr   = (char *)Realloc( bufr, bSize );
         if( NULL == bufr )
         {
            return( False );
         }
      }

      switch( c )
      {
         case ']':
         bufr[end] = '\0';
         if( 0 == end )
         {
	    DEBUG(10, ("WARNING: Empty section name in configuration file.\n"));
            return( False );
         }

         if( !sfunc( bufr ) ) return( False );
         (void)EatComment( InFile );
         return( True );

         case '\n':
            i = Continuation( bufr, i );
            if( i < 0 )
            {
               bufr[end] = '\0';
	       LOG_SYS(0, ("WARNING: Badly formed line in configuration file: %s\n", bufr ));
               return( False );
            }

            end = ( (i > 0) && (' ' == bufr[i - 1]) ) ? (i - 1) : (i);
            c = mygetc( InFile );
            break;

         default:
            if( isspace( c ) )
            {
               bufr[end] = ' ';
               i = end + 1;
               c = EatWhitespace( InFile );
            }
            else
            {
               bufr[i++] = c;
               end = i;
               c = mygetc( InFile );
            }
        }
    }
    return( False );
}


static BOOL Parameter( myFILE *InFile, BOOL (*pfunc)(char *, char *), int c )
{
   int   i       = 0;
   int   end     = 0;
   int   vstart  = 0;
   while( 0 == vstart )
   {
      if( i > (bSize - 2) )
      {
         bSize += BUFR_INC;
         bufr   = (char *)Realloc( bufr, bSize );
         if( NULL == bufr )
         {
            return( False );
         }
      }

      switch( c )
      {
         case '=':  if( 0 == end )  return( False );
                    bufr[end++] = '\0';
                    i           = end;
                    vstart      = end;
                    bufr[i]     = '\0';
                    break;

         case '\n': i = Continuation( bufr, i );
                    if( i < 0 )
                    {
                       bufr[end] = '\0';
                       return( True );
                    }
                    end = ( (i > 0) && (' ' == bufr[i - 1]) ) ? (i - 1) : (i);
                    c = mygetc( InFile );
                    break;

        case '\0':
        case EOF :  bufr[i] = '\0'; return( True );

        default  :  if (isspace(c))
                    {
                       bufr[end] = ' ';
                       i = end + 1;
                       c = EatWhitespace( InFile );
                    }
                    else
                    {
                       bufr[i++] = c;
                       end = i;
                       c = mygetc( InFile );
                    }
      }
   }
   c = EatWhitespace( InFile );
   while( (EOF !=c) && (c > 0) )
   {
      if( i > (bSize - 2) )
      {
         bSize += BUFR_INC;
         bufr   = (char *)Realloc(bufr, bSize);
         if (NULL == bufr)
         {
            return( False );
         }
   }

   switch( c )
   {
      case '\r':
        c = mygetc( InFile );
        break;

      case '\n':
        i = Continuation( bufr, i );
        if( i < 0 )
          c = 0;
        else
        {
           for( end = i; (end >= 0) && isspace(bufr[end]); end-- );
           c = mygetc( InFile );
        }
        break;

      default:
        bufr[i++] = c;
        if( !isspace( c ) ) end = i;
        c = mygetc( InFile );
        break;
      }
   }
   bufr[end] = '\0';
   return( pfunc( bufr, &bufr[vstart] ) );

}


static BOOL Parse( myFILE *InFile, BOOL (*sfunc)(char *), BOOL (*pfunc)(char *, char *) )
{
  int    c;

  c = EatWhitespace( InFile );
  while( (EOF != c) && (c > 0) )
  {
    switch( c )
    {
      case '\n':
        c = EatWhitespace( InFile );
        break;

      case ';':
      case '#':
        c = EatComment( InFile );
        break;

      case '[':
        if( !Section( InFile, sfunc ) )
          return( False );
        c = EatWhitespace( InFile );
        break;

      case '\\':
        c = EatWhitespace( InFile );
        break;

      default:
        if( !Parameter( InFile, pfunc, c ) )
          return( False );
        c = EatWhitespace( InFile );
        break;
    }
  }
  return( True );
}


static myFILE *OpenConfFile( char *FileName )
{
   myFILE *ret;

   ret = (myFILE *)malloc(sizeof(*ret));
   if (!ret) return NULL;

   ret->buf = file_load(FileName, &ret->size);
   if( NULL == ret->buf )
   {
      free(ret);
      return NULL;
   }

   ret->p = ret->buf;
   return(ret);
}


BOOL pm_process( char *FileName, BOOL (*sfunc)(char *),
                 BOOL (*pfunc)(char *, char *) )
{
   int   result;
   myFILE *InFile;

   InFile = OpenConfFile( FileName );
   if(NULL == InFile) return(False);

   if (NULL != bufr) result = Parse(InFile, sfunc, pfunc);
   else
   {
      bSize = BUFR_INC;
      bufr = (char *)malloc( bSize );
      if( NULL == bufr )
      {
         myfile_close(InFile);
         return( False );
      }
      result = Parse( InFile, sfunc, pfunc );
      free( bufr );
      bufr  = NULL;
      bSize = 0;
   }
   myfile_close(InFile);
   if (!result)
   {
      return( False );
   }

   return( True );
}

