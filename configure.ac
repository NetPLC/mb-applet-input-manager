# Process this file with autoconf to produce a configure script.
AC_INIT(mbinputmgr.c)
AM_INIT_AUTOMAKE(mbinputmgr, 0.6)
AM_CONFIG_HEADER(config.h)

MB_LIBS=""

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# Checks for header files.
AC_PATH_X
AC_PATH_XTRA
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h sys/param.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_MALLOC
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([strchr strdup])

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)		

if test x$PKG_CONFIG != xno && $PKG_CONFIG --exists libmb; then
      CFLAGS="$CFLAGS `pkg-config --cflags libmb`"
      MB_LIBS="`pkg-config --libs libmb`"
  else
    AC_MSG_ERROR([Could not find pkg-config entry for libMB])
fi

AC_ARG_ENABLE(gnomeapplet,
  [  --enable-gnomeapplet    Buld GNOME2 Applet (BROKEN).],
  enable_gnomeapplet=$enableval, enable_gnomeapplet=no)

if test x$enable_gnomeapplet = xyes; then

PKG_CHECK_MODULES(GRANDR_APPLET, libgnomeui-2.0 libpanelapplet-2.0)
AC_SUBST(GRANDR_APPLET_CFLAGS)
AC_SUBST(GRANDR_APPLET_LIBS)

fi

AM_CONDITIONAL(ENABLE_GNOME, test x$enable_gnomeapplet = xyes)

dnl check for debug build
AC_ARG_ENABLE(debug,
  [  --enable-debug         enable debug build],
  [if test x$enable_debug = xyes; then
      CFLAGS="$CFLAGS -g -DDEBUG"
  fi])

AC_SUBST(MB_LIBS)

AC_OUTPUT(Makefile)
