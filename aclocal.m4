dnl Checking for Postgres frontend library file

AC_DEFUN(AC_PATH_PG_LIB,
[
  AC_REQUIRE_CPP()
  AC_MSG_CHECKING(for Postgres libraries)
  
  AC_ARG_WITH(pg-libraries,
    [  --with-pg-libraries     where the Postgress libraries are located. ],
    [  ac_pg_libraries="$withval"; ac_cv_lib_pglib=$ac_pg_libraries ])

  AC_CACHE_VAL(ac_cv_lib_pglib, [
    
    ac_pg_libraries="no"
    dnl Did the user give --with-pg-libraries?
    
    if test "$ac_pg_libraries" = no; then

      dnl No they didn't, so lets look for them...
    
      dnl If you need to add extra directories to check, add them here.
      
      pg_library_dirs="\
        /usr/lib \
        /usr/local/lib \
        /usr/lib/pgsql \
        /usr/lib/pgsql/lib \
        /usr/local/lib/pgsql \
        /usr/local/pgsql/lib"
  
      if test "x$PGDIR" != x; then
        pg_library_dirs="$pg_library_dirs $PGDIR/lib"
      fi
  
      if test "x$PGLIB" != x; then
        pg_library_dirs="$pg_library_dirs $PGLIB"
      fi
    
      for pg_dir in $pg_library_dirs; do
        for pg_check_lib in $pg_dir/libpq.a; do
          if test -r $pg_check_lib; then
            ac_pg_libraries=$pg_dir
            break 2
          fi
        done
      done
    fi

    ac_cv_lib_pglib=$ac_pg_libraries
  ])

  dnl Define a shell variable for later checks

  if test "$ac_cv_lib_pglib" = no; then
    have_pg_lib="no"
  else
    have_pg_lib="yes"
  fi
  
  AC_MSG_RESULT([$ac_cv_lib_pglib])
  PG_LDFLAGS="-L$ac_cv_lib_pglib -lpq"
  PG_LIBDIR="$ac_cv_lib_pglib"
  AC_SUBST(PG_LDFLAGS)
  AC_SUBST(PG_LIBDIR)
  
])

dnl Checking for Postgres library header files
AC_DEFUN(AC_PATH_PG_INC,
[
  AC_REQUIRE_CPP()
  AC_MSG_CHECKING(for Postgres includes)
  
  AC_ARG_WITH(pg-includes,
    [  --with-pg-includes      where the Postgres headers are located. ],
    [  ac_pg_includes="$withval"; ac_cv_header_pglib="$withval" ])
  
  AC_CACHE_VAL(ac_cv_header_pglib, [
    
    ac_pg_includes="no"
    dnl Did the user give --with-pg-includes?
    
    if test "$ac_pg_includes" = no; then

      dnl No they didn't, so lets look for them...

      dnl If you need to add extra directories to check, add them here.
      
      pg_include_dirs="\
        /usr/include \
	/usr/local/include \
        /usr/lib/pgsql/include \
        /usr/include/pgsql \
        /usr/local/pgsql/include \
        /usr/local/include/pgsql \
	/usr/include/postgresql"

      if test "x$PGDIR" != x; then
        pg_include_dirs="$pg_include_dirs $PGDIR/include"
      fi

      if test "x$PGINC" != x; then
        pg_include_dirs="$pg_include_dirs $PGINC"
      fi

      for pg_dir in $pg_include_dirs; do
        if test -r "$pg_dir/libpq-fe.h"; then
          ac_pg_includes=$pg_dir
          break
        fi
      done
    fi

    ac_cv_header_pglib=$ac_pg_includes
  
  ])

  if test "$ac_pg_includes" = no; then
    have_pg_inc="no"
  else
    have_pg_inc="yes"
  fi

  AC_MSG_RESULT([$ac_cv_header_pglib])
  PG_INCLUDES="-I$ac_cv_header_pglib"
  PG_INCLUDES2="-I$ac_cv_header_pglib/libpq"
  PG_INCDIR="$ac_cv_header_pglib"
  PG_INCDIR2="$ac_cv_header_pglib/libpq"
  AC_SUBST(PG_INCLUDES)
  AC_SUBST(PG_INCLUDES2)
  AC_SUBST(PG_INCDIR)
  AC_SUBST(PG_INCDIR2)
])

dnl ===============================================================

# Do all the work for Automake.  This macro actually does too much --
# some checks are only needed if your package does certain things.
# But this isn't really a big deal.

# serial 1

dnl Usage:
dnl AM_INIT_AUTOMAKE(package,version, [no-define])

AC_DEFUN(AM_INIT_AUTOMAKE,
[AC_REQUIRE([AC_PROG_INSTALL])
PACKAGE=[$1]
AC_SUBST(PACKAGE)
VERSION=[$2]
AC_SUBST(VERSION)
dnl test to see if srcdir already configured
if test "`cd $srcdir && pwd`" != "`pwd`" && test -f $srcdir/config.status; then
  AC_MSG_ERROR([source directory already configured; run "make distclean" there first])
fi
ifelse([$3],,
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version of package]))
AC_REQUIRE([AM_SANITY_CHECK])
AC_REQUIRE([AC_ARG_PROGRAM])
dnl FIXME This is truly gross.
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_REQUIRE([AC_PROG_MAKE_SET])])


# serial 1

dnl AC_DEFUN(AC_PROG_INSTALL,
dnl [AC_REQUIRE([AC_PROG_INSTALL])
dnl test -z "$INSTALL_SCRIPT" && INSTALL_SCRIPT='${INSTALL_PROGRAM}'
dnl AC_SUBST(INSTALL_SCRIPT)dnl
dnl ])

#
# Check to make sure that the build environment is sane.
#

AC_DEFUN(AM_SANITY_CHECK,
[AC_MSG_CHECKING([whether build environment is sane])
# Just in case
sleep 1
echo timestamp > conftestfile
# Do `set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   set X `ls -Lt $srcdir/configure conftestfile 2> /dev/null`
   if test "[$]*" = "X"; then
      # -L didn't work.
      set X `ls -t $srcdir/configure conftestfile`
   fi
   if test "[$]*" != "X $srcdir/configure conftestfile" \
      && test "[$]*" != "X conftestfile $srcdir/configure"; then

      # If neither matched, then we have a broken ls.  This can happen
      # if, for instance, CONFIG_SHELL is bash and it inherits a
      # broken ls alias from the environment.  This has actually
      # happened.  Such a system could not be considered "sane".
      AC_MSG_ERROR([ls -t appears to fail.  Make sure there is not a broken
alias in your environment])
   fi

   test "[$]2" = conftestfile
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
rm -f conftest*
AC_MSG_RESULT(yes)])

dnl AM_MISSING_PROG(NAME, PROGRAM, DIRECTORY)
dnl The program must properly implement --version.
AC_DEFUN(AM_MISSING_PROG,
[AC_MSG_CHECKING(for working $2)
# Run test in a subshell; some versions of sh will print an error if
# an executable is not found, even if stderr is redirected.
# Redirect stdin to placate older versions of autoconf.  Sigh.
if ($2 --version) < /dev/null > /dev/null 2>&1; then
   $1=$2
   AC_MSG_RESULT(found)
else
   $1="$3/missing $2"
   AC_MSG_RESULT(missing)
fi
AC_SUBST($1)])

# Like AC_CONFIG_HEADER, but automatically create stamp file.

AC_DEFUN(AM_CONFIG_HEADER,
[AC_PREREQ([2.12])
AC_CONFIG_HEADER([$1])
dnl When config.status generates a header, we must update the stamp-h file.
dnl This file resides in the same directory as the config header
dnl that is generated.  We must strip everything past the first ":",
dnl and everything past the last "/".
AC_OUTPUT_COMMANDS(changequote(<<,>>)dnl
ifelse(patsubst(<<$1>>, <<[^ ]>>, <<>>), <<>>,
<<test -z "<<$>>CONFIG_HEADERS" || echo timestamp > patsubst(<<$1>>, <<^\([^:]*/\)?.*>>, <<\1>>)stamp-h<<>>dnl>>,
<<am_indx=1
for am_file in <<$1>>; do
  case " <<$>>CONFIG_HEADERS " in
  *" <<$>>am_file "*<<)>>
    echo timestamp > `echo <<$>>am_file | sed -e 's%:.*%%' -e 's%[^/]*$%%'`stamp-h$am_indx
    ;;
  esac
  am_indx=`expr "<<$>>am_indx" + 1`
done<<>>dnl>>)
changequote([,]))])


# serial 24 AM_PROG_LIBTOOL
AC_DEFUN(AM_PROG_LIBTOOL,
[AC_REQUIRE([AM_ENABLE_SHARED])dnl
AC_REQUIRE([AM_ENABLE_STATIC])dnl
AC_REQUIRE([AC_CANONICAL_HOST])dnl
AC_REQUIRE([AC_PROG_RANLIB])dnl
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AM_PROG_LD])dnl
AC_REQUIRE([AM_PROG_NM])dnl
AC_REQUIRE([AC_PROG_LN_S])dnl
dnl
# Always use our own libtool.
LIBTOOL='$(SHELL) $(top_builddir)/libtool'
AC_SUBST(LIBTOOL)dnl

# Check for any special flags to pass to ltconfig.
libtool_flags=
test "$enable_shared" = no && libtool_flags="$libtool_flags --disable-shared"
test "$enable_static" = no && libtool_flags="$libtool_flags --disable-static"
test "$silent" = yes && libtool_flags="$libtool_flags --silent"
test "$ac_cv_prog_gcc" = yes && libtool_flags="$libtool_flags --with-gcc"
test "$ac_cv_prog_gnu_ld" = yes && libtool_flags="$libtool_flags --with-gnu-ld"

# Some flags need to be propagated to the compiler or linker for good
# libtool support.
case "$host" in
*-*-irix6*)
  # Find out which ABI we are using.
  echo '[#]line __oline__ "configure"' > conftest.$ac_ext
  if AC_TRY_EVAL(ac_compile); then
    case "`/usr/bin/file conftest.o`" in
    *32-bit*)
      LD="${LD-ld} -32"
      ;;
    *N32*)
      LD="${LD-ld} -n32"
      ;;
    *64-bit*)
      LD="${LD-ld} -64"
      ;;
    esac
  fi
  rm -rf conftest*
  ;;

*-*-sco3.2v5*)
  # On SCO OpenServer 5, we need -belf to get full-featured binaries.
  CFLAGS="$CFLAGS -belf"
  ;;
esac

# Actually configure libtool.  ac_aux_dir is where install-sh is found.
CC="$CC" CFLAGS="$CFLAGS" CPPFLAGS="$CPPFLAGS" \
LD="$LD" NM="$NM" RANLIB="$RANLIB" LN_S="$LN_S" \
${CONFIG_SHELL-/bin/sh} $ac_aux_dir/ltconfig \
$libtool_flags --no-verify $ac_aux_dir/ltmain.sh $host \
|| AC_MSG_ERROR([libtool configure failed])
])

# AM_ENABLE_SHARED - implement the --enable-shared flag
# Usage: AM_ENABLE_SHARED[(DEFAULT)]
#   Where DEFAULT is either `yes' or `no'.  If omitted, it defaults to
#   `yes'.
AC_DEFUN(AM_ENABLE_SHARED,
[define([AM_ENABLE_SHARED_DEFAULT], ifelse($1, no, no, yes))dnl
AC_ARG_ENABLE(shared,
changequote(<<, >>)dnl
<<  --enable-shared         build shared libraries [default=>>AM_ENABLE_SHARED_DEFAULT]
changequote([, ])dnl
[  --enable-shared=PKGS    only build shared libraries if the current package
                          appears as an element in the PKGS list],
[p=${PACKAGE-default}
case "$enableval" in
yes) enable_shared=yes ;;
no) enable_shared=no ;;
*)
  enable_shared=no
  # Look at the argument we got.  We use all the common list separators.
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:,"
  for pkg in $enableval; do
    if test "X$pkg" = "X$p"; then
      enable_shared=yes
    fi
  done
  IFS="$ac_save_ifs"
  ;;
esac],
enable_shared=AM_ENABLE_SHARED_DEFAULT)dnl
])

# AM_DISABLE_SHARED - set the default shared flag to --disable-shared
AC_DEFUN(AM_DISABLE_SHARED,
[AM_ENABLE_SHARED(no)])

# AM_DISABLE_STATIC - set the default static flag to --disable-static
AC_DEFUN(AM_DISABLE_STATIC,
[AM_ENABLE_STATIC(no)])

# AM_ENABLE_STATIC - implement the --enable-static flag
# Usage: AM_ENABLE_STATIC[(DEFAULT)]
#   Where DEFAULT is either `yes' or `no'.  If omitted, it defaults to
#   `yes'.
AC_DEFUN(AM_ENABLE_STATIC,
[define([AM_ENABLE_STATIC_DEFAULT], ifelse($1, no, no, yes))dnl
AC_ARG_ENABLE(static,
changequote(<<, >>)dnl
<<  --enable-static         build static libraries [default=>>AM_ENABLE_STATIC_DEFAULT]
changequote([, ])dnl
[  --enable-static=PKGS    only build shared libraries if the current package
                          appears as an element in the PKGS list],
[p=${PACKAGE-default}
case "$enableval" in
yes) enable_static=yes ;;
no) enable_static=no ;;
*)
  enable_static=no
  # Look at the argument we got.  We use all the common list separators.
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:,"
  for pkg in $enableval; do
    if test "X$pkg" = "X$p"; then
      enable_static=yes
    fi
  done
  IFS="$ac_save_ifs"
  ;;
esac],
enable_static=AM_ENABLE_STATIC_DEFAULT)dnl
])


# AM_PROG_LD - find the path to the GNU or non-GNU linker
AC_DEFUN(AM_PROG_LD,
[AC_ARG_WITH(gnu-ld,
[  --with-gnu-ld           assume the C compiler uses GNU ld [default=no]],
test "$withval" = no || with_gnu_ld=yes, with_gnu_ld=no)
AC_REQUIRE([AC_PROG_CC])
ac_prog=ld
if test "$ac_cv_prog_gcc" = yes; then
  # Check if gcc -print-prog-name=ld gives a path.
  AC_MSG_CHECKING([for ld used by GCC])
  ac_prog=`($CC -print-prog-name=ld) 2>&5`
  case "$ac_prog" in
  # Accept absolute paths.
  /* | [A-Za-z]:\\*)
    test -z "$LD" && LD="$ac_prog"
    ;;
  "")
    # If it fails, then pretend we aren't using GCC.
    ac_prog=ld
    ;;
  *)
    # If it is relative, then search for the first ld in PATH.
    with_gnu_ld=unknown
    ;;
  esac
elif test "$with_gnu_ld" = yes; then
  AC_MSG_CHECKING([for GNU ld])
else
  AC_MSG_CHECKING([for non-GNU ld])
fi
AC_CACHE_VAL(ac_cv_path_LD,
[if test -z "$LD"; then
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_dir in $PATH; do
    test -z "$ac_dir" && ac_dir=.
    if test -f "$ac_dir/$ac_prog"; then
      ac_cv_path_LD="$ac_dir/$ac_prog"
      # Check to see if the program is GNU ld.  I'd rather use --version,
      # but apparently some GNU ld's only accept -v.
      # Break only if it was the GNU/non-GNU ld that we prefer.
      if "$ac_cv_path_LD" -v 2>&1 < /dev/null | egrep '(GNU|with BFD)' > /dev/null; then
	test "$with_gnu_ld" != no && break
      else
        test "$with_gnu_ld" != yes && break
      fi
    fi
  done
  IFS="$ac_save_ifs"
else
  ac_cv_path_LD="$LD" # Let the user override the test with a path.
fi])
LD="$ac_cv_path_LD"
if test -n "$LD"; then
  AC_MSG_RESULT($LD)
else
  AC_MSG_RESULT(no)
fi
test -z "$LD" && AC_MSG_ERROR([no acceptable ld found in \$PATH])
AC_SUBST(LD)
AM_PROG_LD_GNU
])

AC_DEFUN(AM_PROG_LD_GNU,
[AC_CACHE_CHECK([if the linker ($LD) is GNU ld], ac_cv_prog_gnu_ld,
[# I'd rather use --version here, but apparently some GNU ld's only accept -v.
if $LD -v 2>&1 </dev/null | egrep '(GNU|with BFD)' 1>&5; then
  ac_cv_prog_gnu_ld=yes
else
  ac_cv_prog_gnu_ld=no
fi])
])

# AM_PROG_NM - find the path to a BSD-compatible name lister
AC_DEFUN(AM_PROG_NM,
[AC_MSG_CHECKING([for BSD-compatible nm])
AC_CACHE_VAL(ac_cv_path_NM,
[case "$NM" in
/* | [A-Za-z]:\\*)
  ac_cv_path_NM="$NM" # Let the user override the test with a path.
  ;;
*)
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_dir in /usr/ucb /usr/ccs/bin $PATH /bin; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/nm; then
      # Check to see if the nm accepts a BSD-compat flag.
      # Adding the `sed 1q' prevents false positives on HP-UX, which says:
      #   nm: unknown option "B" ignored
      if ($ac_dir/nm -B /dev/null 2>&1 | sed '1q'; exit 0) | egrep /dev/null >/dev/null; then
        ac_cv_path_NM="$ac_dir/nm -B"
      elif ($ac_dir/nm -p /dev/null 2>&1 | sed '1q'; exit 0) | egrep /dev/null >/dev/null; then
        ac_cv_path_NM="$ac_dir/nm -p"
      else
        ac_cv_path_NM="$ac_dir/nm"
      fi
      break
    fi
  done
  IFS="$ac_save_ifs"
  test -z "$ac_cv_path_NM" && ac_cv_path_NM=nm
  ;;
esac])
NM="$ac_cv_path_NM"
AC_MSG_RESULT([$NM])
AC_SUBST(NM)
])

dnl ==========================================================
dnl test for socklen_t (for shit recvfrom function)
dnl ==========================================================
AC_DEFUN(AC_TYPE_SOCKLEN_T,
[AC_CACHE_CHECK(for socklen_t in sys/socket.h, ac_cv_type_socklen_t,
[AC_TRY_RUN(
[
#include <sys/types.h>
#include <sys/socket.h>
main()
{
 socklen_t socklen;
}
],
  ac_cv_type_socklen_t=yes, ac_cv_type_socklen_t=no, 
  ac_cv_type_socklen_t=no)
])]

if test "$ac_cv_type_socklen_t" = yes; then
AC_DEFINE([HAVE_SOCKLEN_T], 1, [Define if we have socklen_t datatype])
fi

)

dnl ==========================================================
dnl test if recvfrom have signed 6th arg
dnl ==========================================================
AC_DEFUN(AC_RECVFROM_WITH_INT,
[AC_CACHE_CHECK(if recvfrom have signed 6th arg, ac_cv_recvfrom_with_int,
[AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE(
 changequote(<<, >>)dnl
 <<
 #include <sys/types.h>
 #include <sys/socket.h>
 >>,

 << 
  struct sockaddr client_addr;
  int socklen = sizeof(client_addr);
  int buffr;
     
  recvfrom(0, &buffr, 0, 0, &client_addr, &socklen);
 >>,
 changequote([, ])dnl
 ac_cv_recvfrom_with_int=yes, ac_cv_recvfrom_with_int=no)
 AC_LANG_C])]
 
if test "$ac_cv_recvfrom_with_int" = yes; then
 AC_DEFINE([RECVFROM_WITH_INT], 1, [Define if we can use signed int in recvfrom()])
fi

)

AC_DEFUN(AC_PATH_PG_BIN,
[
  AC_REQUIRE_CPP()
  AC_MSG_CHECKING(for Postgres binaries)
  
  ac_pg_binaries="no"
  
  AC_ARG_WITH(pg-binaries,
    [  --with-pg-binaries     where the Postgress binaries are located. ],
    [  ac_pg_binaries="$withval" ])

  AC_CACHE_VAL(ac_cv_bin_pgbin, [
    
    dnl Did the user give --with-pg-binaries?
    
    if test "$ac_pg_binaries" = no; then

      dnl No they didn't, so lets look for them...
    
      dnl If you need to add extra directories to check, add them here.
      
      pg_binaries_dirs="\
        /usr/bin \
        /usr/local/bin \
        /usr/local/pgsql/bin \
        /usr/pgsql/bin \
        /usr/sbin \
	/bin \
	/sbin \
	/usr/local/sbin"
  
      if test "x$PGDIR" != x; then
        pg_binaries_dirs="$pg_binaries_dirs $PGDIR/bin"
      fi
  
      if test "x$PGBIN" != x; then
        pg_binaries_dirs="$pg_binaries_dirs $PGBIN"
      fi
    
      for pg_dir in $pg_binaries_dirs; do
        for pg_check_bin in $pg_dir/psql; do
          if test -r $pg_check_bin; then
            ac_pg_binaries=$pg_dir
            break 2
          fi
        done
      done
    fi

    ac_cv_bin_pgbin=$ac_pg_binaries
  ])

  dnl Define a shell variable for later checks

  if test "$ac_cv_bin_pgbin" = no; then
    have_pg_bin="no"
  else
    have_pg_bin="yes"
  fi
  
  AC_MSG_RESULT([$ac_cv_bin_pgbin])
  PSQL_BIN="$ac_cv_bin_pgbin"
  AC_SUBST(PG_LDFLAGS)
  AC_SUBST(PSQL_BIN)
 
])

dnl @synopsis ETR_SYSV_IPC
dnl
dnl This macro checks for the SysV IPC header files.  It only checks
dnl that you can compile a program with them, not whether the system
dnl actually implements working SysV IPC.
dnl
dnl @author Warren Young <warren@etr-usa.com>
dnl
AC_DEFUN([ETR_SYSV_IPC],
[
AC_CACHE_CHECK([for System V IPC headers], ac_cv_sysv_ipc, [
        AC_TRY_COMPILE(
                [
                        #include <sys/types.h>
                        #include <sys/ipc.h>
                        #include <sys/msg.h>
                        #include <sys/sem.h>
                        #include <sys/shm.h>
                ],, ac_cv_sysv_ipc=yes, ac_cv_sysv_ipc=no)
])

        if test x"$ac_cv_sysv_ipc" = "xyes"
        then
                AC_DEFINE(HAVE_SYSV_IPC, 1, [ Define if you have System V IPC ])
        fi
]) dnl ETR_SYSV_IPC


dnl @synopsis CHECK_BACKTRACE
dnl
AC_DEFUN([CHECK_BACKTRACE],
[
AC_CACHE_CHECK([for backtrace], ac_cv_check_backtrace, [
        AC_TRY_COMPILE(
                [
                        #include <sys/types.h>
                        #include <execinfo.h>
			
			int chk()
			{
			   void *addr_array[32];
			   int addr_num = backtrace(addr_array, 32);
			}
			
                ],, ac_cv_check_backtrace=yes, ac_cv_check_backtrace=no)
])

        if test x"$ac_cv_check_backtrace" = "xyes"
        then
                AC_DEFINE(HAVE_BACKTRACE, 1, [ Define if you have backtrace function ])
        fi
]) dnl CHECK_BACKTRACE



dnl @synopsis ETR_STRUCT_SEMUN
dnl
dnl This macro checks to see if sys/sem.h defines struct semun.  Some
dnl systems do, some systems don't.  Your code must be able to deal
dnl with this possibility; if HAVE_STRUCT_SEMUM isn't defined for a
dnl given system, you have to define this structure before you can
dnl call functions like semctl().
dnl
dnl You should call ETR_SYSV_IPC before this macro, to separate the
dnl check for System V IPC headers from the check for struct semun.
dnl
dnl @author Warren Young <warren@etr-usa.com>
dnl
AC_DEFUN([ETR_STRUCT_SEMUN],
[
AC_CACHE_CHECK([for struct semun], ac_cv_struct_semun, [
        AC_TRY_COMPILE(
                [
                        #include <sys/types.h>
                        #include <sys/ipc.h>
                        #include <sys/sem.h>
                ],
                [ union semun s; ],
                ac_cv_struct_semun=yes,
                ac_cv_struct_semun=no)
])

        if test x"$ac_cv_struct_semun" = "xyes"
        then
                AC_DEFINE(HAVE_STRUCT_SEMUN, 1,
                        [ Define if your system's sys/sem.h file defines struct semun ])
        fi
]) dnl ETR_STRUCT_SEMUN

dnl @synopsis AC_COMPILE_CHECK_SIZEOF(TYPE [, HEADERS [, EXTRA_SIZES...]])
dnl
dnl This macro checks for the size of TYPE using compile checks, not
dnl run checks. You can supply extra HEADERS to look into. the check
dnl will cycle through 1 2 4 8 16 and any EXTRA_SIZES the user
dnl supplies. If a match is found, it will #define SIZEOF_`TYPE' to
dnl that value. Otherwise it will emit a configure time error
dnl indicating the size of the type could not be determined.
dnl
dnl The trick is that C will not allow duplicate case labels. While
dnl this is valid C code:
dnl
dnl      switch (0) case 0: case 1:;
dnl
dnl The following is not:
dnl
dnl      switch (0) case 0: case 0:;
dnl
dnl Thus, the AC_TRY_COMPILE will fail if the currently tried size
dnl does not match.
dnl
dnl Here is an example skeleton configure.in script, demonstrating the
dnl macro's usage:
dnl
dnl      AC_PROG_CC
dnl      AC_CHECK_HEADERS(stddef.h unistd.h)
dnl      AC_TYPE_SIZE_T
dnl      AC_CHECK_TYPE(ssize_t, int)
dnl
dnl      headers='#ifdef HAVE_STDDEF_H
dnl      #include <stddef.h>
dnl      #endif
dnl      #ifdef HAVE_UNISTD_H
dnl      #include <unistd.h>
dnl      #endif
dnl      '
dnl
dnl      AC_COMPILE_CHECK_SIZEOF(char)
dnl      AC_COMPILE_CHECK_SIZEOF(short)
dnl      AC_COMPILE_CHECK_SIZEOF(int)
dnl      AC_COMPILE_CHECK_SIZEOF(long)
dnl      AC_COMPILE_CHECK_SIZEOF(unsigned char *)
dnl      AC_COMPILE_CHECK_SIZEOF(void *)
dnl      AC_COMPILE_CHECK_SIZEOF(size_t, $headers)
dnl      AC_COMPILE_CHECK_SIZEOF(ssize_t, $headers)
dnl      AC_COMPILE_CHECK_SIZEOF(ptrdiff_t, $headers)
dnl      AC_COMPILE_CHECK_SIZEOF(off_t, $headers)
dnl
dnl @author Kaveh Ghazi <ghazi@caip.rutgers.edu>
dnl
AC_DEFUN([AC_COMPILE_CHECK_SIZEOF],
[changequote(<<, >>)dnl
dnl The name to #define.
define(<<AC_TYPE_NAME>>, translit(sizeof_$1, [a-z *], [A-Z_P]))dnl
dnl The cache variable name.
define(<<AC_CV_NAME>>, translit(ac_cv_sizeof_$1, [ *], [_p]))dnl
changequote([, ])dnl
AC_MSG_CHECKING(size of $1)
AC_CACHE_VAL(AC_CV_NAME,
[for ac_size in 4 8 1 2 16 $2 ; do # List sizes in rough order of prevalence.
  AC_TRY_COMPILE([#include "confdefs.h"
#include <sys/types.h>
$2
], [switch (0) case 0: case (sizeof ($1) == $ac_size):;], AC_CV_NAME=$ac_size)
  if test x$AC_CV_NAME != x ; then break; fi
done
])
if test x$AC_CV_NAME = x ; then
  AC_MSG_ERROR([cannot determine a size for $1])
fi
AC_MSG_RESULT($AC_CV_NAME)
AC_DEFINE_UNQUOTED(AC_TYPE_NAME, $AC_CV_NAME, [The number of bytes in type $1])
undefine([AC_TYPE_NAME])dnl
undefine([AC_CV_NAME])dnl
])

dnl @synopsis AC_NEED_STDINT_H [( HEADER-TO-GENERATE [, HEDERS-TO-CHECK])]
dnl
dnl the "ISO C9X: 7.18 Integer types <stdint.h>" section requires the
dnl existence of an include file <stdint.h> that defines a set of 
dnl typedefs, especially uint8_t,int32_t,uintptr_t.
dnl Many older installations will not provide this file, but some will
dnl have the very same definitions in <inttypes.h>. In other enviroments
dnl we can use the inet-types in <sys/types.h> which would define the
dnl typedefs int8_t and u_int8_t respectivly.
dnl
dnl This macros will create a local "stdint.h" if it cannot find the
dnl global <stdint.h> (or it will create the headerfile given as an argument).
dnl In many cases that file will just have a singular "#include <inttypes.h>"
dnl statement, while in other environments it will provide the set of basic
dnl stdint's defined: 
dnl int8_t,uint8_t,int16_t,uint16_t,int32_t,uint32_t,intptr_t,uintptr_t
dnl int_least32_t.. int_fast32_t.. intmax_t
dnl which may or may not rely on the definitions of other files,
dnl or using the AC_COMPILE_CHECK_SIZEOF macro to determine the actual
dnl sizeof each type.
dnl
dnl if your header files require the stdint-types you will want to create an
dnl installable file package-stdint.h that all your other installable header
dnl may include. So if you have a library package named "mylib", just use
dnl      AC_NEED_STDINT(zziplib-stdint.h) 
dnl in configure.in and go to install that very header file in Makefile.am
dnl along with the other headers (mylib.h) - and the mylib-specific headers
dnl can simply use "#include <mylib-stdint.h>" to obtain the stdint-types.
dnl
dnl Remember, if the system already had a valid <stdint.h>, the generated
dnl file will include it directly. No need for fuzzy HAVE_STDINT_H things...
dnl
dnl @author  Guido Draheim <guidod@gmx.de>       STATUS: used on new platforms

AC_DEFUN([AC_NEED_STDINT_H],
[AC_MSG_CHECKING([for stdint-types])
 ac_cv_header_stdint="no-file"
 ac_cv_header_stdint_u="no-file"
 for i in $1 inttypes.h sys/inttypes.h sys/int_types.h stdint.h ; do
   AC_CHECK_TYPEDEF_(uint32_t, $i, [ac_cv_header_stdint=$i])
 done
 for i in $1 sys/types.h inttypes.h sys/inttypes.h sys/int_types.h ; do
   AC_CHECK_TYPEDEF_(u_int32_t, $i, [ac_cv_header_stdint_u=$i])
 done
 dnl debugging: __AC_MSG( !$ac_cv_header_stdint!$ac_cv_header_stdint_u! ...)

 ac_stdint_h=`echo ifelse($1, , stdint.h, $1)`
 if test "$ac_cv_header_stdint" != "no-file" ; then
   if test "$ac_cv_header_stdint" != "$ac_stdint_h" ; then
     AC_MSG_RESULT(found in $ac_cv_header_stdint)
     echo "#include <$ac_cv_header_stdint>" >$ac_stdint_h
     AC_MSG_RESULT(creating $ac_stdint_h - (just to include  $ac_cv_header_stdint) )
   else
     AC_MSG_RESULT(found in $ac_stdint_h)
   fi
   ac_cv_header_stdint_generated=false
 elif test "$ac_cv_header_stdint_u" != "no-file" ; then
   AC_MSG_RESULT(found u_types in $ac_cv_header_stdint_u)
   if test $ac_cv_header_stdint = "$ac_stdint_h" ; then
     AC_MSG_RESULT(creating $ac_stdint_h - includes $ac_cv_header_stdint, expect problems!)
   else
     AC_MSG_RESULT(creating $ac_stdint_h - (include inet-types in $ac_cv_header_stdint_u and re-typedef))
   fi
   cat >$ac_stdint_h <<EOF
#ifndef __MY_STDINT_H
#define __MY_STDINT_H 1
#include <stddef.h>
#include <$ac_cv_header_stdint_u>
/* int8_t int16_t int32_t defined by inet code */
typedef u_int8_t uint8_t;
typedef u_int16_t uint16_t;
typedef u_int32_t uint32_t;
/* it's a networkable system, but without any stdint.h */
/* hence it's an older 32-bit system... (a wild guess that seems to work) */
typedef u_int32_t uintptr_t;
typedef   int32_t  intptr_t;
EOF
   ac_cv_header_stdint_generated=true
 else
   AC_MSG_RESULT(not found, need to guess the types now... )
   AC_COMPILE_CHECK_SIZEOF(long, 32)
   AC_COMPILE_CHECK_SIZEOF(void*, 32)
   AC_MSG_RESULT( creating $ac_stdint_h - using detected values for sizeof long and sizeof void* )
   cat >$ac_stdint_h <<EOF
   
#ifndef __MY_STDINT_H
#define __MY_STDINT_H 1
/* ISO C 9X: 7.18 Integer types <stdint.h> */

#define __int8_t_defined  
typedef   signed char    int8_t;
typedef unsigned char   uint8_t;
typedef   signed short  int16_t;
typedef unsigned short uint16_t;
EOF

   if test "$ac_cv_sizeof_long" = "64" ; then
     cat >>$ac_stdint_h <<EOF

typedef   signed int    int32_t;
typedef unsigned int   uint32_t;
typedef   signed long   int64_t;
typedef unsigned long  uint64_t;
#define  int64_t  int64_t
#define uint64_t uint64_t
EOF

   else
    cat >>$ac_stdint_h <<EOF

typedef   signed long   int32_t;
typedef unsigned long  uint32_t;
EOF

   fi
   if test "$ac_cv_sizeof_long" != "$ac_cv_sizeof_voidp" ; then
     cat >>$ac_stdint_h <<EOF

typedef   signed int   intptr_t;
typedef unsigned int  uintptr_t;
EOF
   else
     cat >>$ac_stdint_h <<EOF

typedef   signed long   intptr_t;
typedef unsigned long  uintptr_t;
EOF
     ac_cv_header_stdint_generated=true
   fi
 fi   

 if "$ac_cv_header_stdint_generated" ; then
     cat >>$ac_stdint_h <<EOF

typedef  int8_t    int_least8_t;
typedef  int16_t   int_least16_t;
typedef  int32_t   int_least32_t;

typedef uint8_t   uint_least8_t;
typedef uint16_t  uint_least16_t;
typedef uint32_t  uint_least32_t;

typedef  int8_t    int_fast8_t;	
typedef  int32_t   int_fast16_t;
typedef  int32_t   int_fast32_t;

typedef uint8_t   uint_fast8_t;	
typedef uint32_t  uint_fast16_t;
typedef uint32_t  uint_fast32_t;

typedef long int       intmax_t;
typedef unsigned long uintmax_t;
#endif
EOF
  fi dnl
])

dnl @synopsis PETI_ENABLED_DYNAMIC_LINKING
dnl
dnl This macro give the user a comfortable way to add "-static" to the
dnl linker flags, that is, to build statically linked binaries.
dnl Currently only the "-static" flags is used to achieve that, but on
dnl some operating systems, more sophisticated LDFLAGS might be
dnl necessary.
dnl
dnl @author Peter Simons <simons@computer.org>
dnl
AC_DEFUN([PETI_ENABLED_DYNAMIC_LINKING], [
AC_MSG_CHECKING(whether what binaries we shall create)
AC_ARG_ENABLE(dynamic-link,
[  --enable-dynamic-link   Create dynamically linked binaries (default)],
if test "$enableval" = "yes"; then
    AC_MSG_RESULT(dynamically linked)
else
    LDFLAGS="$LDFLAGS -static"
    AC_MSG_RESULT(statically linked)
fi,
AC_MSG_RESULT(dynamically linked))
])

dnl @synopsis AC_CHECK_TYPEDEF_(TYPEDEF, HEADER [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]])
dnl
dnl check if the given typedef-name is recognized as a type. The trick is
dnl to use a sizeof(TYPEDEF) and see if the compiler is happy with that.
dnl
dnl this can be thought of as a mixture of AC_CHECK_TYPE(TYPEDEF,DEFAULT)
dnl and AC_CHECK_LIB(LIBRARY,FUNCTION,ACTION-IF-FOUND,ACTION-IF-NOT-FOUND)
dnl
dnl a convenience macro AC_CHECK_TYPEDEF_ is provided that will not emit
dnl any message to the user - it just executes one of the actions.
dnl
dnl @author  Guido Draheim <guidod@gmx.de>

AC_DEFUN(AC_CHECK_TYPEDEF_,
[dnl
ac_lib_var=`echo $1['_']$2 | sed 'y%./+-%__p_%'`
AC_CACHE_VAL(ac_cv_lib_$ac_lib_var,
[ eval "ac_cv_type_$ac_lib_var='not-found'"
  ac_cv_check_typedef_header=`echo ifelse([$2], , stddef.h, $2)`
  AC_TRY_COMPILE( [#include <$ac_cv_check_typedef_header>], 
	[int x = sizeof($1); x = x;],
        eval "ac_cv_type_$ac_lib_var=yes" ,
        eval "ac_cv_type_$ac_lib_var=no" )
  if test `eval echo '$ac_cv_type_'$ac_lib_var` = "no" ; then 
     ifelse([$4], , :, $4)
  else 
     ifelse([$3], , :, $3) 
  fi
])])

dnl AC_CHECK_TYPEDEF(TYPEDEF, HEADER [, ACTION-IF-FOUND,
dnl    [, ACTION-IF-NOT-FOUND ]])
AC_DEFUN(AC_CHECK_TYPEDEF,
[dnl
 AC_MSG_CHECKING([for $1 in $2])
 AC_CHECK_TYPEDEF_($1,$2,AC_MSG_RESULT(yes),AC_MSG_RESULT(no))dnl
])


dnl Checking for full system name and version (uname -a)
AC_DEFUN(AC_UNAME_SYS,
[
  AC_MSG_CHECKING(for full system name)
  
  AC_CACHE_VAL(ac_cv_uname_sys, [
    
  ac_cv_uname_sys=`((uname -s -r) 2>/dev/null || echo unknown) | sed 's/\"//g'`
  
  ])

  AC_CACHE_VAL(ac_cv_name_sys, [
    
  ac_cv_name_sys=`((uname -n) 2>/dev/null || echo unknown) | sed 's/\"//g'`;
  
  ])

  if test "$ac_cv_uname_sys" = "unknown" ; then
    AC_MSG_RESULT(unknown)
  else
    AC_MSG_RESULT(uname -a)
  fi
  
  SYS_UNAME=$ac_cv_uname_sys
  SYS_NAME=$ac_cv_name_sys
  
  AC_SUBST(SYS_UNAME)
  AC_SUBST(SYS_NAME)
])

dnl @synopsis PETI_PATH_SENDMAIL
dnl
dnl This macro will find a sendmail binary in many obscure places and
dnl replace @SENDMAIL@ with the path in all output files.
dnl
dnl @author Peter Simons <address@bogus.example.com>
dnl
AC_DEFUN([PETI_PATH_SENDMAIL], [
    peti_path_backup=$PATH
    PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/lib:/usr/libexec:/usr/local/bin
    PATH=$PATH:/usr/local/sbin:/usr/local/lib:/usr/local/libexec:/usr/etc
    AC_PATH_PROG(SENDMAIL, sendmail, sendmail)
    PATH=$peti_path_backup
])

