# defaults
top_srcdir=.
builddir=build
stagedir=.
distdir=.
globalprefix=glite
lbutilsprefix=lb-utils
package=glite-lb-utils-db
version=0.2.0
PREFIX=/opt/glite

glite_location=/opt/glite
mysql_prefix=/opt/mysql
mysql_version=4.1.11
cppunit_prefix=/opt/cppunit
thrflavour=gcc32pthr
nothrflavour=gcc32

-include Makefile.inc
-include ../project/version.properties

version=${module.version}

CC=gcc

VPATH=${top_srcdir}/interface:${top_srcdir}/src:${top_srcdir}/examples

DEBUG:=-g -O0 -Wall

CFLAGS:= \
	${DEBUG} \
	-DVERSION=\"${version}\" \
	-I${stagedir}/include -I${top_srcdir}/src -I. \
	-I${top_srcdir}/interface \
	${COVERAGE_FLAGS} \
	-I${mysql_prefix}/include -I${mysql_prefix}/include/mysql \
	-D_GNU_SOURCE

ifdef LBS_DB_PROFILE
	CFLAGS:=${CFLAGS} -DLBS_DB_PROFILE
endif

TEST_LIBS:=-L${cppunit_prefix}/lib -lcppunit
TEST_INC:=-I${cppunit_prefix}/include

COMPILE:=libtool --mode=compile ${CC} ${CFLAGS}
LINK:=libtool --mode=link ${CC} -rpath ${stagedir}/lib ${LDFLAGS} 
INSTALL:=libtool --mode=install install

ifneq (${mysql_prefix},/usr)
	ifeq ($(shell echo ${mysql_version} | cut -d. -f1,2),4.1)
		mysqlib := -L${mysql_prefix}/lib/mysql
	else
		mysqlib := -L${mysql_prefix}/lib
	endif
endif

EXT_LIBS:=${mysqlib} -lmysqlclient
OBJS:=db.o
HDRS:=db.h
LOBJS:=${OBJS:.o=.lo}

libglite_lbutils_db.la: ${LOBJS}
	${LINK} -o $@ $< ${EXT_LIBS}

db_test: db_test.lo
	${LINK} -o $@ $< -lglite_lbutils_db

default all: compile

compile: libglite_lbutils_db.la

check:
	-echo No checks here yet.

test_coverage:
	-mkdir coverage
	cd coverage && $(MAKE) -f ../Makefile top_srcdir=../../ COVERAGE_FLAGS="-fprofile-arcs -ftest-coverage" check
	cd coverage && for i in `echo ${OBJS} | tr ' ' '\012' | sort -u`; do gcov $$i ; done

examples: db_test

doc:
	doxygen C.dox

stage: compile
	$(MAKE) install PREFIX=${stagedir} DOSTAGE=yes

dist: distsrc distbin

distsrc:
	mkdir -p ${top_srcdir}/${package}-${version}
	cd ${top_srcdir} && GLOBIGNORE="${package}-${version}" && cp -Rf * ${package}-${version}
	cd ${top_srcdir} && tar -czf ${distdir}/${package}-${version}_src.tar.gz --exclude-from=project/tar_exclude ${package}-${version}
	rm -rf ${top_srcdir}/${package}-${version}

distbin:
	$(MAKE) install PREFIX=`pwd`/tmpbuilddir${stagedir}
	save_dir=`pwd`; cd tmpbuilddir${stagedir} && tar -czf $$save_dir/${top_srcdir}/${distdir}/${package}-${version}_bin.tar.gz *; cd $$save_dir
	rm -rf tmpbuilddir
        
install:
	-mkdir -p ${PREFIX}/lib
	-mkdir -p ${PREFIX}/include/${globalprefix}/${lbutilsprefix}
#	${INSTALL} -m 644 ${top_srcdir}/LICENSE ${PREFIX}/share/doc/${package}-${version}
	${INSTALL} -m 755 "libglite_lbutils_db.la" "${PREFIX}/lib/glite-lbutils-db.la"; \
	${INSTALL} -m 644 ${HDRS} ${PREFIX}/include/${globalprefix}/${lbutilsprefix}

clean:

%.o %.lo: %.c
	${COMPILE} -c $<

db.o db.lo: db.c db.h
db_test: libglite_lbutils_db.la db.h db_test.c

.PHONY: default all compile check examples doc stage dist distsrc distbin install clean test_coverage
