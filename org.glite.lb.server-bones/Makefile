# defaults
top_srcdir=.
builddir=build
top_builddir=${top_srcdir}/${builddir}
stagedir=.
distdir=.
globalprefix=glite
lbprefix=lb
package=glite-lb-server-bones
version=0.0.1
PREFIX=/opt/glite

glite_location=/opt/glite
globus_prefix=/opt/globus
nothrflavour=gcc32
thrflavour=gcc32pthr
expat_prefix=/opt/expat
ares_prefix=/opt/ares

-include Makefile.inc

VPATH=${top_srcdir}/src:${top_srcdir}/test

DEBUG:=-g -O0 -Wall
CFLAGS:= ${DEBUG} -I${top_srcdir}/interface -I${top_srcdir}/src -I.

COMPILE:=libtool --mode=compile ${CC} ${CFLAGS}
LINK:=libtool --mode=link ${CC} -rpath ${stagedir}/lib ${LDFLAGS} 
INSTALL:=libtool --mode=install install

STATICLIB:=libglite_lb_server_bones.a
LTLIB:=libglite_lb_server_bones.la

OBJS:=srvbones.o
LOBJS:=${OBJS:.o=.lo}

HDRS:=srvbones.h

default all: compile

compile: ${STATICLIB} ${LTLIB}

${STATICLIB}: ${OBJS}
	ar crv $@ ${OBJS}
	ranlib $@

${LTLIB}: ${LOBJS}
	${LINK} -o $@ ${LOBJS}

stage: compile
	$(MAKE) install PREFIX=${stagedir}

check: example_test
	-echo "No unit tests so far."

example_test: srv_example client_example

srv_example: srv_example.o
	${LINK} -o $@ ${LTLIB} srv_example.o

client_example: client.o
	${LINK} -o $@ client.o

doc:

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
	mkdir -p ${PREFIX}/include/${globalprefix}/${lbprefix}
	mkdir -p ${PREFIX}/lib
	${INSTALL} -m 644 ${STATICLIB} ${PREFIX}/lib
	${INSTALL} -m 644 ${LTLIB} ${PREFIX}/lib
	cd ${top_srcdir}/interface && install -m 644 ${HDRS} ${PREFIX}/include/${globalprefix}/${lbprefix}

clean:

%.o: %.c
	${COMPILE} -c $<
