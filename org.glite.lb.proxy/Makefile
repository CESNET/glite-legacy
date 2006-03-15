# defaults
top_srcdir=.
builddir=build
top_builddir=${top_srcdir}/${builddir}
stagedir=.
distdir=.
globalprefix=glite
lbprefix=lb
package=glite-lb-server
version=0.2.0
PREFIX=/opt/glite

glite_location=/opt/glite
globus_prefix=/opt/globus
nothrflavour=gcc32
thrflavour=gcc32pthr
expat_prefix=/opt/expat
ares_prefix=/opt/ares
gsoap_prefix=/opt/gsoap

-include Makefile.inc
-include ../Makefile.inc

CC=gcc
YACC=bison -y

VPATH=${top_srcdir}/src:${top_srcdir}/test:${top_srcdir}/examples:${top_srcdir}/project
AT3=perl -I${top_srcdir}/project ${top_srcdir}/project/at3

TEST_LIBS:=-L${cppunit}/lib -lcppunit
TEST_INC:=-I${cppunit}/include

SUFFIXES = .T 

DEBUG:=-g -O0 -Wall

CFLAGS:= ${DEBUG} \
	-DVERSION=\"${version}\" \
	-I${stagedir}/include -I${top_srcdir}/src -I. \
	-I${expat_prefix}/include \
	-I${ares_prefix}/include \
	-I${gsoap_prefix}/include \
	${COVERAGE_FLAGS} \
	-I${mysql_prefix}/include -I${mysql_prefix}/include/mysql \
	-I${globus_prefix}/include/${nothrflavour} \
	-I${gridsite_prefix}/include -I${globus_prefix}/include/${nothrflavour}/openssl \
	-D_GNU_SOURCE


LINK:=libtool --mode=link ${CC} ${LDFLAGS} 
LINKXX:=libtool --mode=link ${CXX} ${LDFLAGS} 
INSTALL:=libtool --mode=install install
LINKXX:=libtool --mode=link  ${CXX} -rpath ${stagedir}/lib ${LDFLAGS}

GLOBUS_LIBS:= -L${globus_prefix}/lib \
	-lglobus_common_${nothrflavour} \
	-lglobus_gssapi_gsi_${nothrflavour} \

ifneq (${mysql_prefix},/usr)
	ifeq ($(shell echo ${mysql_version} | cut -d. -f1,2),4.1)
		mysqlib := -L${mysql_prefix}/lib/mysql
	else
		mysqlib := -L${mysql_prefix}/lib
	endif
endif

ifneq (${expat_prefix},/usr)
	expatlib := -L${expat_prefix}/lib
endif


SRVBONES_LIB:= -L${stagedir}/lib -lglite_lb_server_bones
LB_SERVER_SHARE_LIB:=${stagedir}/lib/libglite_lb_bkserver.a
LB_PROXY_COMMON_LIB:= -L${stagedir}/lib -lglite_lb_common_${nothrflavour}
COMMON_LIBS:= -L${stagedir}/lib -lglite_lb_common_${nothrflavour} -lglite_security_gss_${nothrflavour}
GRIDSITE_LIBS = -lgridsite_globus `xml2-config --libs`

vomsflavour := _${nothrflavour}
ifeq (${nothrflavour},gcc32)
        vomsflavour :=
endif
ifeq (${nothrflavour},gcc32dbg)
        vomsflavour :=
endif

EXT_LIBS:= -L${ares_prefix}/lib -lares \
	${mysqlib} -lmysqlclient -lz\
	${expatlib} -lexpat \
	${GRIDSITE_LIBS} \
	-lvomsc${vomsflavour} \
	${GLOBUS_LIBS}



LB_PROXY_LIBS:= \
	${LB_SERVER_SHARE_LIB} \
	${SRVBONES_LIB} \
	-lglite_lb_common_${nothrflavour} \
	-lglite_security_gss_${nothrflavour} \
	${EXT_LIBS}



glite_lb_proxy: lbproxy.o ${LB_SERVER_SHARE_LIB}
	@echo DEBUG: mysql_version=${mysql_version} mysql_prefix=${mysql_prefix}
	@echo DEBUG: shell: x$(shell echo ${mysql_version} | cut -d. -f1,2)x
	${LINK} -o $@ lbproxy.o ${LB_PROXY_LIBS}

default all: compile

compile: glite_lb_proxy

check: compile
	-echo No test so far

examples: 

doc:

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
	-mkdir -p ${PREFIX}/bin ${PREFIX}/etc ${PREFIX}/etc/init.d
	-mkdir -p ${PREFIX}/share/doc/${package}-${version}
	${INSTALL} -m 644 ${top_srcdir}/LICENSE ${PREFIX}/share/doc/${package}-${version}
	${INSTALL} -m 644 ${top_srcdir}/doc/README ${top_srcdir}/doc/README.deploy ${PREFIX}/share/doc/${package}-${version}
	${INSTALL} -m 755 glite_lb_proxy ${PREFIX}/bin/glite-lb-proxy
	${INSTALL} -m 644 ${top_srcdir}/config/glite-lb-dbsetup-proxy.sql ${PREFIX}/etc
	${INSTALL} -m 755 ${top_srcdir}/config/startup ${PREFIX}/etc/init.d/glite-lb-proxy

# don't do it due to RPM conflict
#	if [ x${DOSTAGE} != xyes ]; then \
#		${INSTALL} -m 755 ${stagedir}/bin/glite-lb-interlogd ${PREFIX}/bin; \
#	fi


clean:

%.c: %.c.T
	rm -f $@
	${AT3} $< >$@ || rm -f $@
	chmod -w $@ >/dev/null

%.o: %.y
	${YACC} -d ${YFLAGS} $<
	mv y.tab.c $*.c
	mv y.tab.h $*.h
	${CC} -c ${CFLAGS} $*.c
	rm $*.c

%.cpp: %.cpp.T
	rm -f $@
	${AT3} $< >$@ || rm -f $@
	chmod -w $@ >/dev/null
