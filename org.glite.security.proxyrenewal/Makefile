#
# Copyright (c) 2004 on behalf of the EU EGEE Project:
# The European Organization for Nuclear Research (CERN),
# Istituto Nazionale di Fisica Nucleare (INFN), Italy
# Datamat Spa, Italy
# Centre National de la Recherche Scientifique (CNRS), France
# CS Systeme d'Information (CSSI), France
# Royal Institute of Technology, Center for Parallel Computers (KTH-PDC), Sweden
# Universiteit van Amsterdam (UvA), Netherlands
# University of Helsinki (UH.HIP), Finland
# University of Bergen (UiB), Norway
# Council for the Central Laboratory of the Research Councils (CCLRC), United Kingdom
#
# Top Makefile file for the GLite Security Proxyrenewal module
#
# Authors: Ales Krenek <ljocha@ics.muni.cz>
# Version info: $Id$
# Release: $Name$
#
# Revision history:
# $Log
#

# defaults
top_srcdir=.
builddir=build
top_builddir=${top_srcdir}/${builddir}
stagedir=.
distdir=.
globalprefix=glite
lbprefix=lb
package=glite-security-proxyrenewal
version=1.0.0
PREFIX=/opt/glite

glite_location=$PREFIX
globus_prefix=/opt/globus
nothrflavour=gcc32
thrflavour=gcc32pthr
myproxy_prefix=$globus_prefix

-include Makefile.inc

VPATH:=${top_srcdir}/src

GLOBUSINC:= -I${globus_prefix}/include/${nothrflavour} \
	-I${globus_prefix}/include/${nothrflavour}/openssl

GLOBUSTHRINC:= -I${globus_prefix}/include/${thrflavour} \
	-I${globus_prefix}/include/${thrflavour}/openssl

MYPROXYINC:= -I${myproxy_prefix}/include/${nothrflavour}
MYPROXYTHRINC:= -I${myproxy_prefix}/include/${thrflavour}

DEBUG:=-g -O0

CFLAGS:= ${DEBUG} \
	${MYPROXYINC} \
	-I${top_srcdir}/src -I${top_srcdir}/interface \
	-I${glite_location}/include

GLOBUS_LIBS:=-L${globus_prefix}/lib \
	-lglobus_common_${nothrflavour} \
	-lssl_${nothrflavour}

MYPROXY_LIB:=-L${myproxy_prefix}/lib -lmyproxy_${nothrflavour}

JOBIDLIB:=-L${glite_location}/lib -lglite_wmsutils_cjobid

COMPILE:=libtool --mode=compile ${CC} ${CFLAGS}
LINK:=libtool --mode=link ${CC} ${LDFLAGS}
INSTALL:=libtool --mode=install install

DAEMONOBJ:=renewd.o renew.o common.o commands.o api.o voms.o
LIBOBJ:=api.o common.o
LIB_CORE_OBJS := renewal_core.o voms.o renewal_core_compat.o
CLIENTOBJ:=client.o

THRLIBOBJ:=${LIBOBJ:.o=.thr.o}
LIBLOBJ:=${LIBOBJ:.o=.lo}
THRLIBLOBJ:=${LIBOBJ:.o=.thr.lo}

LIB_CORE_NOTHR_OBJS := ${LIB_CORE_OBJS}
LIB_CORE_NOTHR_LOBJS := ${LIB_CORE_OBJS:.o=.lo}
LIB_CORE_THR_OBJS := ${LIB_CORE_OBJS:.o=.thr.o}
LIB_CORE_THR_LOBJS := ${LIB_CORE_OBJS:.o=.thr.lo}

LIB:=libglite_security_proxyrenewal_${nothrflavour}.la
THRLIB:=libglite_security_proxyrenewal_${thrflavour}.la
LIB_CORE_NOTHR := libglite_security_proxyrenewal_core_${nothrflavour}.la
LIB_CORE_THR := libglite_security_proxyrenewal_core_${thrflavour}.la

DAEMON:=glite-proxy-renewd
CLIENT:=glite-proxy-renew

default: all
compile all: ${LIB} ${THRLIB} ${LIB_CORE_NOTHR} ${DAEMON} ${CLIENT}

${LIB}: ${LIBOBJ}
	${LINK} -o $@ ${LIBLOBJ} -rpath ${glite_location}/lib ${JOBIDLIB}


${THRLIB}: ${THRLIBOBJ}
	${LINK} -o $@ ${THRLIBLOBJ} -rpath ${glite_location}/lib

${LIB_CORE_NOTHR}: ${LIB_CORE_NOTHR_OBJS}
	${LINK} -o $@ ${LIB_CORE_NOTHR_OBJS} -rpath ${glite_location}/lib ${MYPROXY_LIB} -L${glite_location}/lib -lvomsc_${nothrflavour}

${DAEMON}: ${DAEMONOBJ}
	${LINK} -o $@ ${DAEMONOBJ} ${JOBIDLIB} ${MYPROXY_LIB} -lvomsc -lglobus_gss_assist_${nothrflavour} ${GLOBUS_LIBS} 

${CLIENT}: ${CLIENTOBJ} ${LIB}
	${LINK} -o $@ ${CLIENTOBJ} ${LIB} ${GLOBUS_LIBS}

${THRLIBOBJ} ${LIB_CORE_THR_OBJS}: %.thr.o: %.c
	${COMPILE} ${GLOBUSTHRINC} -o $@ -c $<

%.o: %.c
	${COMPILE} ${GLOBUSINC} -c $<

stage: compile
	$(MAKE) install PREFIX=${stagedir}


check:
	echo No unit tests

dist: distsrc distbin

distsrc:
	mkdir -p ${top_srcdir}/${package}-${version}
	cd ${top_srcdir} && GLOBIGNORE="${package}-${version}" && cp -Rf * ${package}-${version}
	cd ${top_srcdir} && tar -czf ${distdir}/${package}-${version}_src.tar.gz --exclude-from=project/tar_exclude ${package}-${version}
	rm -rf ${top_srcdir}/${package}-${version}

distbin:
	$(MAKE) install PREFIX=`pwd`/tmpbuilddir
	cd tmpbuilddir && tar -czf ../${top_srcdir}/${distdir}/${package}-${version}_bin.tar.gz *
	rm -rf tmpbuilddir

install:
	-mkdir -p ${PREFIX}/bin ${PREFIX}/lib ${PREFIX}/include/glite/security/proxyrenewal ${PREFIX}/share/doc/${package}-${version} ${PREFIX}/etc/init.d
	${INSTALL} -m 644 ${LIB} ${THRLIB} ${LIB_CORE_NOTHR} ${PREFIX}/lib
	${INSTALL} -m 755 ${DAEMON} ${CLIENT} ${PREFIX}/bin
	${INSTALL} -m 644 ${top_srcdir}/LICENSE ${PREFIX}/share/doc/${package}-${version}
	cd ${top_srcdir}/interface && ${INSTALL} -m 644 renewal.h renewal_core.h ${PREFIX}/include/glite/security/proxyrenewal

	${INSTALL} -m 755 ${top_srcdir}/config/startup ${PREFIX}/etc/init.d/glite-proxy-renewald


clean:
	$(RM) $(LIB) ${LIB_CORE_NOTHR} $(DAEMON) $(CLIENT) *.o core 
