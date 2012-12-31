# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)

AC_INIT(scim-chewing, 0.3.5, chewing-devel@googlegroups.com)
AC_CONFIG_AUX_DIR([autotools])
AC_CONFIG_SRCDIR([src/scim_chewing_imengine.h])

AC_SUBST(PACKAGE_VERSION)

# Save this value here, since automake will set cflags and cxxflags later
cflags_set=${CFLAGS+set}
cppflags_set=${CPPFLAGS+set}

AC_DEFINE_UNQUOTED(SCIM_CHEWING_VERSION, "$PACKAGE_VERSION", [The release version of scim-chewing.])

# Init automake stuff
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

# Init gettext
#ALL_LINGUAS="zh_TW"
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.14.1)

GETTEXT_PACKAGE=scim-chewing
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [The gettext domain])
AM_GLIB_GNU_GETTEXT

# Init libtool
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
#AC_LIB_LTDL
AC_SUBST(LIBTOOL_DEPS)

# Init intltool
IT_PROG_INTLTOOL([0.34.0])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_LANG(C++)

AC_C_CONST
AC_TYPE_SIZE_T

# Checks for libraries.

SCIM_VERSION=1.0.0
CHEWING_VERSION=0.3.4

PKG_CHECK_MODULES(SCIM, [scim >= $SCIM_VERSION])

# Check if we should build setup module
PKG_CHECK_MODULES(SCIM_GTKUTILS,[scim-gtkutils >= $SCIM_VERSION],
		  		[SCIM_HAS_GTKUTILS=yes],
				[SCIM_HAS_GTKUTILS=no])

AC_SUBST(SCIM_VERSION)

PKG_CHECK_MODULES(CHEWING, [chewing = $CHEWING_VERSION])
AC_SUBST(CHEWING_VERSION)

AM_CONDITIONAL(SCIM_BUILD_SETUP, [test "$SCIM_HAS_GTKUTILS" = "yes"])

if test "$SCIM_HAS_GTKUTILS" = "yes"; then
  SCIM_BUILD_SETUP=1
else
  SCIM_BUILD_SETUP=0
fi

AC_SUBST(SCIM_BUILD_SETUP)

# Checks for header files.
AC_HEADER_STDC

# Checks for library functions.
AC_SUBST(ac_aux_dir)

# libtool option to control which symbols are exported
# right now, symbols starting with _ are not exported
LIBTOOL_EXPORT_OPTIONS='-export-symbols-regex "^[[^_]].*"'

# Extra args.
AC_ARG_ENABLE(debug,
	      [  --enable-debug     Turn on debugging],
	      [case "${enableval}" in
	       yes) debug=yes ;;
	       no)  debug=no ;;
	       *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
       	      esac],[debug=no])

if test "x$debug" = "xyes"; then
  test "$cflags_set" = set || CFLAGS="$CFLAGS -g"
  test "$cppflags_set" = set || CPPFLAGS="$CXXFLAGS -g"
  SCIM_DEBUG_FLAGS="-DENABLE_DEBUG=1"
else
  SCIM_DEBUG_FLAGS=""
fi
AC_SUBST(SCIM_DEBUG_FLAGS)

SCIM_ICONDIR=`$PKG_CONFIG --variable=icondir scim`
SCIM_MODULEDIR=`$PKG_CONFIG --variable=moduledir scim`
SCIM_DATADIR=`$PKG_CONFIG --variable=scimdatadir scim`

if test "x$SCIM_ICONDIR" = "x"; then
  SCIM_ICONDIR=%{prefix}/share/scim/icons
fi

if test "x$SCIM_MODULEDIR" = "x"; then
  SCIM_MODULEDIR=${libdir}/scim-1.0
fi

if test "x$SCIM_DATADIR" = "x"; then
  SCIM_DATADIR=${datadir}/scim
fi

AC_SUBST(SCIM_ICONDIR)
AC_SUBST(SCIM_MODULEDIR)
AC_SUBST(SCIM_DATADIR)

CHEWING_DATADIR=`$PKG_CONFIG --variable=datadir chewing`

if test "x$CHEWING_DATADIR" = "x"; then
  CHEWING_DATADIR=%{prefix}/share/chewing
fi

AC_SUBST(CHEWING_DATADIR)
AC_DEFINE_UNQUOTED(CHEWING_DATADIR,"$CHEWING_DATADIR", [Chewing Runtime data dir])

AC_CONFIG_FILES([
		Makefile
		po/Makefile.in 
		src/Makefile
		data/Makefile
		scim-chewing.spec
])
AC_OUTPUT
