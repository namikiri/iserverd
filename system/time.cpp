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
/**************************************************************************/

#include "includes.h"

int serverzone=0;
int extra_time_offset = 0;

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

#ifndef TIME_T_MIN
#define TIME_T_MIN ((time_t)0 < (time_t) -1 ? (time_t) 0 \
		    : ~ (time_t) 0 << (sizeof (time_t) * CHAR_BIT - 1))
#endif
#ifndef TIME_T_MAX
#define TIME_T_MAX (~ (time_t) 0 - TIME_T_MIN)
#endif

/**************************************************************************/
/* A gettimeofday wrapper						  */
/**************************************************************************/
void GetTimeOfDay(struct timeval *tval)
{
   gettimeofday(tval,NULL);
}


#define TM_YEAR_BASE 1900


/**************************************************************************/
/* Yield the diff between *A and *B, in seconds, ignoring leap seconds	  */
/**************************************************************************/
int tm_diff(struct tm *a, struct tm *b)
{
  int ay = a->tm_year + (TM_YEAR_BASE - 1);
  int by = b->tm_year + (TM_YEAR_BASE - 1);
  int intervening_leap_days =
    (ay/4 - by/4) - (ay/100 - by/100) + (ay/400 - by/400);
  int years = ay - by;
  int days = 365*years + intervening_leap_days + (a->tm_yday - b->tm_yday);
  int hours = 24*days + (a->tm_hour - b->tm_hour);
  int minutes = 60*hours + (a->tm_min - b->tm_min);
  int seconds = 60*minutes + (a->tm_sec - b->tm_sec);

  return seconds;
}


/**************************************************************************/
/* Return the UTC offs in sec west of UTC, or 0 if it can't be determined */
/**************************************************************************/
int TimeZone(time_t t)
{
  struct tm *tm = gmtime(&t);
  struct tm tm_utc;
  if (!tm) return 0;
  tm_utc = *tm;
  tm = localtime(&t);
  if (!tm)  return 0;

  return tm_diff(&tm_utc,tm);
}


/**************************************************************************/
/* init the time differences						  */
/**************************************************************************/
void TimeInit(void)
{
  serverzone = TimeZone(time(NULL));

}


/**************************************************************************/
/* Return the same value as TimeZone, but it should be more efficient.	  */
/*									  */
/* We keep a table of DST offsets to prevent calling localtime() on each  */
/* call of this function. This saves a LOT of time on many unixes.	  */
/*									  */
/* Updated by Paul Eggert <eggert@twinsun.com>				  */
/**************************************************************************/
int TimeZoneFaster(time_t t)
{
  static struct dst_table {time_t start,end; int zone;} *dst_table = NULL;
  static int table_size = 0;
  int i;
  int zone = 0;

  if (t == 0) t = time(NULL);

  /* Tunis has a 8 day DST region, we need to be careful ... */
#define MAX_DST_WIDTH (365*24*60*60)
#define MAX_DST_SKIP (7*24*60*60)

  for (i=0;i<table_size;i++)
    if (t >= dst_table[i].start && t <= dst_table[i].end) break;

  if (i<table_size)
  {
    zone = dst_table[i].zone;
  }
  else
  {
    time_t low,high;

    zone = TimeZone(t);
    dst_table = (struct dst_table *)Realloc(dst_table,
		 sizeof(dst_table[0])*(i+1));
    if (!dst_table)
    {
      table_size = 0;
    }
    else
    {
      table_size++;

      dst_table[i].zone = zone;
      dst_table[i].start = dst_table[i].end = t;

      /* no entry will cover more than 6 months */
      low = t - MAX_DST_WIDTH/2;
      if (t < low)   low = TIME_T_MIN;

      high = t + MAX_DST_WIDTH/2;
      if (high < t)  high = TIME_T_MAX;

      /* widen the new entry using two bisection searches */
      while (low+60*60 < dst_table[i].start)
      {
	if (dst_table[i].start - low > MAX_DST_SKIP*2)
	  t = dst_table[i].start - MAX_DST_SKIP;
	else
	  t = low + (dst_table[i].start-low)/2;

	if (TimeZone(t) == zone)
	  dst_table[i].start = t;
	else
	  low = t;
      }

      while (high-60*60 > dst_table[i].end)
      {
	if (high - dst_table[i].end > MAX_DST_SKIP*2)
	  t = dst_table[i].end + MAX_DST_SKIP;
	else
	  t = high - (high-dst_table[i].end)/2;

	if (TimeZone(t) == zone)
	  dst_table[i].end = t;
	else
	  high = t;
      }
    }
  }
  return zone;
}


/**************************************************************************/
/* Return the UTC offs in sec west of UTC, adjusted for extra time offset */
/**************************************************************************/
int TimeDiff(time_t t)
{
  return TimeZoneFaster(t) + 60*extra_time_offset;
}


/**************************************************************************/
/* Return the UTC offset in seconds west of UTC, adjusted for extra time  */
/* offset, for a local time value.  If ut = lt + LocTimeDiff(lt), then	  */
/* lt = ut - TimeDiff(ut), but the converse doesn't necessarily hold near */
/* daylight savings transitions because some local times are ambiguous.	  */
/* LocTimeDiff(t) eq TimeDiff(t) except near daylight savings transitions */
/**************************************************************************/
int LocTimeDiff(time_t lte)
{
  time_t lt = lte - 60*extra_time_offset;
  int d = TimeZoneFaster(lt);
  time_t t = lt + d;

  /* if overflow occurred, ignore all the adjustments so far */
  if (((lte < lt) ^ (extra_time_offset < 0))  |  ((t < lt) ^ (d < 0)))
    t = lte;

  /* now t should be close enough to the true UTC to yield the right answer */
  return TimeDiff(t);
}


/**************************************************************************/
/* Try to opt the localtm call, it can be quite expensive on some OS	  */
/**************************************************************************/
struct tm *LocalTime(time_t *t)
{
  time_t t2 = *t;

  t2 -= TimeDiff(t2);

  return(gmtime(&t2));
}


#define TIME_FIXUP_CONSTANT (369.0*365.25*24*60*60-(3.0*24*60*60+6.0*60*60))
/**************************************************************************/
/* check if it's a null mtime						  */
/**************************************************************************/
BOOL null_mtime(time_t mtime)
{
  if ((mtime == 0) || (mtime == (time_t)-1) )  return(True);

  return(False);
}


/**************************************************************************/
/* Return the date & time as a str in short format [14.03.2000 11:38:56]  */
/**************************************************************************/
char *timestr()
{
   static fstring TimeBuf;
   struct tm *tim;
   time_t t;
   t = time(NULL);

   tim = LocalTime(&t);
   slprintf(TimeBuf, sizeof(TimeBuf)-1, "%02d.%02d.%04d %02d:%02d:%02d",
            tim->tm_mday, tim->tm_mon+1, (tim->tm_year+1900),
            tim->tm_hour, tim->tm_min, tim->tm_sec);

   return(TimeBuf);
}


/**************************************************************************/
/* Conv time from time_t to string in short format [14.03.2000 11:38:56]  */
/**************************************************************************/
char *time2str(time_t stime)
{
   static fstring TimeBuf;
   struct tm *tim;

   tim = LocalTime(&stime);
   slprintf(TimeBuf, sizeof(TimeBuf)-1, "%02d.%02d.%04d %02d:%02d:%02d",
            tim->tm_mday, tim->tm_mon+1, (tim->tm_year+1900),
            tim->tm_hour, tim->tm_min, tim->tm_sec);

   return(TimeBuf);
}


/**************************************************************************/
/* Conv time from time_t to string in short format [14.03.2000 11:38:56]  */
/**************************************************************************/
char *time2str1(time_t stime)
{
   static fstring TimeBuf;
   struct tm *tim;

   tim = LocalTime(&stime);
   slprintf(TimeBuf, sizeof(TimeBuf)-1, "%02d.%02d.%02d %02d:%02d",
            tim->tm_mday, tim->tm_mon+1,
          ((tim->tm_year+1900) % 100),
	    tim->tm_hour, tim->tm_min);

   return(TimeBuf);
}


/**************************************************************************/
/* Return the date and time as a string					  */
/**************************************************************************/
char *timestring(BOOL hires)
{
   static fstring TimeBuf;
   struct timeval tp;
   time_t t;
   struct tm *tm;

   if (hires)
   {
      GetTimeOfDay(&tp);
      t = (time_t)tp.tv_sec;
   }
   else
   {
      t = time(NULL);
   }

   tm = LocalTime(&t);

   if (!tm)
   {
      if (hires)
      {
         slprintf(TimeBuf, sizeof(TimeBuf)-1,
                  "%ld.%06ld seconds since the Epoch",
                  (long)tp.tv_sec, (long)tp.tv_usec);

      }
      else
      {
         slprintf(TimeBuf, sizeof(TimeBuf)-1,
                  "%ld seconds since the Epoch",
                  (long)t);
      }
   }
   else
   {
      if (hires)
      {
         slprintf(TimeBuf, sizeof(TimeBuf)-1,
                  "%s.%06ld", asctime(tm), (long)tp.tv_usec);
      }
      else
      {
         fstrcpy(TimeBuf, asctime(tm));
      }
   }

   string_truncate(TimeBuf, strlen(TimeBuf)-1);

   return(TimeBuf);
}


/**************************************************************************/
/* Return the best approx to a 'create time' under UNIX from a stat	  */
/* structure.								  */
/**************************************************************************/
time_t get_create_time(struct stat  *st,BOOL fake_dirs)
{
  time_t ret, ret1;

  if(S_ISDIR(st->st_mode) && fake_dirs)
    return (time_t)315493200L;          /* 1/1/1980 */

  ret = MIN(st->st_ctime, st->st_mtime);
  ret1 = MIN(ret, st->st_atime);

  if(ret1 != (time_t)0)
    return ret1;

  return ret;
}

