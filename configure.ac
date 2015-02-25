#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT(OpenStratos, Alpha-1-dev, https://openstratos.org/bugtracker)
AC_CONFIG_SRCDIR([openstratos.cc])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_CHECK_PROG(RASPIVID, raspivid, yes)

# Checks for libraries.
AC_CHECK_LIB([wiringPi], [wiringPiSetupSys])

# Checks for header files.
AC_CHECK_HEADERS([string.h])

AC_LANG([C++])
AX_CXX_COMPILE_STDCXX_11()

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([stpcpy strstr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
