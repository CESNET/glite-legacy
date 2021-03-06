# defaults
top_srcdir=.
stagedir=.
globalprefix=glite
lbprefix=lb
package=glite-lb-server
version=0.2.0
PREFIX=/opt/glite

nothrflavour=gcc32
thrflavour=gcc32pthr
expat_prefix=/opt/expat

-include Makefile.inc
-include ../project/version.properties

version=${module.version}

CC=gcc
YACC=bison -y

VPATH=${top_srcdir}/src:${top_srcdir}/test:${top_srcdir}/examples:${top_srcdir}/project

TEST_LIBS:=-L${cppunit_prefix}/lib -lcppunit
TEST_INC:=-I${cppunit_prefix}/include

ifdef LB_PERF
        STATIC_LIB_BK:=${stagedir}/lib/libglite_lb_bkserver_perf.a
        LB_PERF_FLAGS:=-DLB_PERF
else
        STATIC_LIB_BK:=${stagedir}/lib/libglite_lb_bkserver.a
endif

SUFFIXES = .T 

DEBUG:=-g -O0 -Wall

ifdef LB_STANDALONE
	LB_STANDALONE_FLAGS:=-DLB_STANDALONE
endif

CFLAGS:= ${DEBUG} \
	-DVERSION=\"${version}\" \
	-I${stagedir}/include -I${top_srcdir}/src -I. \
	-I${expat_prefix}/include \
	${COVERAGE_FLAGS} \
	-I${gridsite_prefix}/include \
	-D_GNU_SOURCE ${LB_STANDALONE_FLAGS} ${LB_PERF_FLAGS}


LINK:=libtool --mode=link ${CC} ${LDFLAGS} 
LINKXX:=libtool --mode=link ${CXX} ${LDFLAGS} 
INSTALL:=libtool --mode=install install
LINKXX:=libtool --mode=link  ${CXX} -rpath ${stagedir}/lib ${LDFLAGS}

archlib:=lib
host_cpu:=${shell uname -m}
ifeq (${host_cpu},x86_64)
    archlib:=lib64
endif

ifneq (${expat_prefix},/usr)
	expatlib := -L${expat_prefix}/lib
endif


SRVBONES_LIB:= -L${stagedir}/lib -lglite_lbu_server_bones
GRIDSITE_LIBS = -lgridsite_globus `xml2-config --libs`

vomsflavour := _${nothrflavour}
ifeq (${nothrflavour},gcc32)
        vomsflavour :=
endif
ifeq (${nothrflavour},gcc32dbg)
        vomsflavour :=
endif

VOMS_LIBS:=-L${voms_prefix}/lib -lvomsc${vomsflavour}
EXT_LIBS:= \
	${expatlib} -lexpat \
	${GRIDSITE_LIBS} \
	${VOMS_LIBS}



LB_PROXY_LIBS:= \
	${STATIC_LIB_BK} \
	${SRVBONES_LIB} \
	-lglite_lb_common_${nothrflavour} \
	-lglite_lbu_db \
	-lglite_security_gss_${nothrflavour} \
	${EXT_LIBS}



glite-lb-proxy: lbproxy.o fake_write2rgma.o ${STATIC_LIB_BK}
	@echo DEBUG: mysql_version=${mysql_version} mysql_prefix=${mysql_prefix}
	@echo DEBUG: shell: x$(shell echo ${mysql_version} | cut -d. -f1,2)x
	${LINK} -o $@ lbproxy.o fake_write2rgma.o ${LB_PROXY_LIBS}

glite-lb-proxy-perf: lbproxy.o fake_write2rgma.o ${STATIC_LIB_BK}
	${LINK} -o $@ lbproxy.o fake_write2rgma.o ${LB_PROXY_LIBS}

default all: compile

compile: glite-lb-proxy

check: compile
	-echo No test so far

examples: 

doc:

stage: compile
	$(MAKE) install PREFIX=${stagedir} DOSTAGE=yes

install:
	-mkdir -p ${PREFIX}/bin ${PREFIX}/etc ${PREFIX}/etc/init.d
	-mkdir -p ${PREFIX}/share/doc/${package}-${version}
	${INSTALL} -m 644 ${top_srcdir}/LICENSE ${PREFIX}/share/doc/${package}-${version}
	${INSTALL} -m 644 ${top_srcdir}/doc/README ${top_srcdir}/doc/README.deploy ${PREFIX}/share/doc/${package}-${version}
	${INSTALL} -m 755 glite-lb-proxy ${PREFIX}/bin/glite-lb-proxy
	${INSTALL} -m 644 ${top_srcdir}/config/glite-lb-dbsetup-proxy.sql ${PREFIX}/etc
	${INSTALL} -m 755 ${top_srcdir}/config/startup ${PREFIX}/etc/init.d/glite-lb-proxy

# don't do it due to RPM conflict
#	if [ x${DOSTAGE} != xyes ]; then \
#		${INSTALL} -m 755 ${stagedir}/bin/glite-lb-interlogd ${PREFIX}/bin; \
#	fi


clean:
	rm -rvf *.o .libs glite-lb-proxy
	rm -rvf log.xml project/ rpmbuild/ RPMS/ tgz/

%.o: %.y
	${YACC} -d ${YFLAGS} $<
	mv y.tab.c $*.c
	mv y.tab.h $*.h
	${CC} -c ${CFLAGS} $*.c
	rm $*.c
