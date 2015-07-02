#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.69)
AC_INIT(user-manage-client, 1.1.0, NULL)
AC_CONFIG_SRCDIR(src/main.c)
AC_CONFIG_HEADERS(config.h)
AM_INIT_AUTOMAKE
#AM_INIT_AUTOMAKE(user-manage-client, 1.0)

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB(m, pow)
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB(pthread, pthread_create)
AC_CHECK_LIB(gtk+-3.0, gtk_init)
AM_PATH_GTK_3_0(,,AC_MSG_ERROR(need GTK+-3))

# Checks for header files.
AC_CHECK_HEADERS(arpa/inet.h fcntl.h float.h limits.h stdlib.h string.h sys/socket.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS(bzero floor memset pow socket strchr)

AC_OUTPUT(Makefile
		src/Makefile)
