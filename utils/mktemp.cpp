#include "config.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

int main(int argc, char **argv)
{
   char tempst[1024];
   int fd;

   if (argc < 2)
   {
      printf("usage: %s filenameXXXXXX\n", argv[0]);
      exit(-1);
   }

   strncpy(tempst, argv[1], 1023);

#ifdef HAVE_MKSTEMP
   fd = mkstemp(tempst);
#else
#ifdef HAVE_MKTEMP
   mktemp(tempst);
   fd = open(tempst, O_CREAT);
#else
#define close "ERROR: I need mktemp utility or mktemp/mkstemp functions"
#endif
#endif
   close(fd);
   printf("%s\n", tempst);
   return(0);
}
