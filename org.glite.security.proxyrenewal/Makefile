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
version=0.0.0
PREFIX=/opt/glite

glite_location=$PREFIX
globus_prefix=/opt/globus
voms_prefix=/opt/glite
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

# XXX: until VOMS is ready in SCM
CFLAGS:= -DNOVOMS \
	${DEBUG} \
	-DVOMS_INSTALL_PATH=\"${voms_prefix}\"\
	${MYPROXYINC} \
	-I${top_srcdir}/src -I${top_srcdir}/interface \
	-I${glite_location}/include

GLOBUS_LIBS:=-L${globus_prefix}/lib \
	-lglobus_common_${nothrflavour} \
	-lssl_${nothrflavour}

SSL_UTILS_LIB:=-L${glite_location}/lib -lglobus_ssl_utils
MYPROXY_LIB:=-L${myproxy_prefix}/lib -lmyproxy_${nothrflavour}

JOBIDLIB:=-L${glite_location}/lib -lglite_wms_cjobid

COMPILE:=libtool --mode=compile ${CC} ${CFLAGS}
LINK:=libtool --mode=link ${CC} ${LDFLAGS}
INSTALL:=libtool --mode=install install

DAEMONOBJ:=renewd.o renew.o common.o commands.o api.o
LIBOBJ:=api.o common.o
CLIENTOBJ:=client.o

THRLIBOBJ:=${LIBOBJ:.o=.thr.o}
LIBLOBJ:=${LIBOBJ:.o=.lo}
THRLIBLOBJ:=${LIBOBJ:.o=.thr.lo}

LIB:=libglite_security_proxyrenewal_${nothrflavour}.la
THRLIB:=libglite_security_proxyrenewal_${thrflavour}.la

DAEMON:=glite-proxy-renewd
CLIENT:=glite-proxy-renew

default: all
compile all: ${LIB} ${THRLIB} ${DAEMON} ${CLIENT}

${LIB}: ${LIBOBJ}
	${LINK} -o $@ ${LIBLOBJ} -rpath ${glite_location}/lib ${JOBIDLIB} ${SSL_UTILS_LIB}


${THRLIB}: ${THRLIBOBJ}
	${LINK} -o $@ ${THRLIBLOBJ} -rpath ${glite_location}/lib ${SSL_UTILS_LIB}

${DAEMON}: ${DAEMONOBJ}
	${LINK} -o $@ ${DAEMONOBJ} ${JOBIDLIB} ${SSL_UTILS_LIB} ${MYPROXY_LIB} -lglobus_gss_assist_${nothrflavour} ${GLOBUS_LIBS} 

${CLIENT}: ${CLIENTOBJ} ${LIB}
	${LINK} -o $@ ${CLIENTOBJ} ${LIB} ${GLOBUS_LIBS}

${THRLIBOBJ}: %.thr.o: %.c
	${COMPILE} ${GLOBUSTHRINC} -o $@ -c $<

%.o: %.c
	${COMPILE} ${GLOBUSINC} -c $<

install:
	-mkdir -p ${PREFIX}/bin ${PREFIX}/lib ${PREFIX}/include/glite/security
	${INSTALL} -m 644 ${LIB} ${THRLIB} ${PREFIX}/lib
	${INSTALL} -m 755 ${DAEMON} ${CLIENT} ${PREFIX}/bin
	cd ${top_srcdir}/interface && ${INSTALL} -m 644 renewal.h ${PREFIX}/include/glite/security


stage: compile
	$(MAKE) install PREFIX=${stagedir}


check:
	echo No unit tests
