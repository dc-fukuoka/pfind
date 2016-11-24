#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([pfind], [0.1], [dc-fukuoka@sgi.com])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O
AC_PROG_MAKE_SET
AC_OPENMP

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h stdio.h sys/types.h dirent.h linux/limits.h fnmatch.h errno.h omp.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strdup strerror strrchr])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
