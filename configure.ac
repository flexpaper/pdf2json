#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_CONFIG_HEADERS([aconf.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

dnl ##### Checks for header files.
AC_PATH_XTRA
AC_HEADER_DIRENT

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lGoo':
AC_CHECK_LIB([Goo], [main])
# FIXME: Replace `main' with a function in `-lX11':
AC_CHECK_LIB([X11], [main])
# FIXME: Replace `main' with a function in `-lXpdf':
AC_CHECK_LIB([Xpdf], [main])
# FIXME: Replace `main' with a function in `-lfofi':
AC_CHECK_LIB([fofi], [main])
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])
# FIXME: Replace `main' with a function in `-lsplash':
AC_CHECK_LIB([splash], [main])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

dnl ##### Path to xpdfrc.
dnl This ugly kludge to get the sysconfdir path is needed because
dnl autoconf doesn't actually set the prefix variable until later.
if test "$sysconfdir" = '${prefix}/etc'; then
  if test "x$prefix" = xNONE; then
    system_xpdfrc="$ac_default_prefix/etc/xpdfrc"
  else
    system_xpdfrc="$prefix/etc/xpdfrc"
  fi
else
  system_xpdfrc="$sysconfdir/xpdfrc"
fi
AC_DEFINE_UNQUOTED(SYSTEM_XPDFRC, "$system_xpdfrc")

dnl ##### Checks for library functions.
AC_CHECK_FUNCS(popen)
dnl # This should use 'AC_CHECK_FUNCS(mkstemp)' but that fails if
dnl # the mkstemp exists in the library but isn't declared in the
dnl # include file (e.g., in cygwin 1.1.2).
AC_CACHE_CHECK([for mkstemp],
xpdf_cv_func_mkstemp,
[AC_TRY_LINK([#include <stdlib.h>
#include <unistd.h>],
[mkstemp("foo");],
xpdf_cv_func_mkstemp=yes, xpdf_cv_func_mkstemp=no)])
if test "$xpdf_cv_func_mkstemp" = yes; then
  AC_DEFINE(HAVE_MKSTEMP)
fi
dnl # Check for mkstemps, just like mkstemp.
AC_CACHE_CHECK([for mkstemps],
xpdf_cv_func_mkstemps,
[AC_TRY_LINK([#include <stdlib.h>
#include <unistd.h>],
[mkstemps("foo", 0);],
xpdf_cv_func_mkstemps=yes, xpdf_cv_func_mkstemps=no)])
if test "$xpdf_cv_func_mkstemps" = yes; then
  AC_DEFINE(HAVE_MKSTEMPS)
fi

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FSEEKO
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_DEFINE(HAVE_SPLASH)
AC_CHECK_FUNCS([floor ftime getcwd gettimeofday memmove memset pow sqrt strcasecmp strchr strcspn strrchr strstr])

AC_CONFIG_FILES([Makefile
                 fofi/Makefile
                 goo/Makefile
                 splash/Makefile
                 src/Makefile
                 xpdf/Makefile])
AC_OUTPUT
