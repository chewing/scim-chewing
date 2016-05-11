#! /bin/sh
AM_VERSION=
AC_VERSION=

set -x

if [ "x${ACLOCAL_DIR}" != "x" ]; then
  ACLOCAL_ARG=-I ${ACLOCAL_DIR}
fi

# ChangeLog is deprecated, but autotool expects one.
touch ChangeLog

AUTOMAKE=${AUTOMAKE:-automake$AM_VERSION} libtoolize -c --automake 
AUTOMAKE=${AUTOMAKE:-automake$AM_VERSION} intltoolize -c --automake
${ACLOCAL:-aclocal$AM_VERSION} ${ACLOCAL_ARG}
${AUTOHEADER:-autoheader$AC_VERSION}
${AUTOMAKE:-automake$AM_VERSION} --add-missing --copy --include-deps
${AUTOCONF:-autoconf$AC_VERSION}

if [ -d autom4te.cache ]; then
	rm -rf autom4te.cache
fi	
