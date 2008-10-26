#!/bin/sh

if [ -f Makefile ]; then
	if [ -f test/Makefile ]; then
		make -C test distclean
	fi
	make distclean
fi

rm -rf autom4te.cache

find -name Makefile | xargs rm -f 
find -name Makefile.in | xargs rm -f
find -name .deps | xargs rm -rf
find -name '*.gmo' | xargs rm -f

rm -rf \
	aclocal.m4 \
	autotools/config.sub \
	autotools/config.guess \
	config.status.lineno \
	configure \
	autotools/install-sh \
	autom4te.cache \
	config.log \
	autotools/depcomp \
	libtool \
	config.status \
	autotools/ltmain.sh \
	autotools/missing \
	config.h.in \
	intltool-* \
	po/boldquot.sed \
	po/insert-header.sin \
	po/Makevars.template \
	po/remove-potcdate.sin \
	po/stamp-po \
	po/Makefile.in.in  \
	po/Rules-quot \
	po/Makevars \
	po/quot.sed           

LINK_KEY=src/tools/key2pho.c
if [ -f $LINK_KEY ]; then
	rm -f $LINK_KEY
fi
