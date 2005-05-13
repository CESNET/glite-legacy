# defaults
top_srcdir=.
builddir=build
top_builddir=${top_srcdir}/${builddir}
stagedir=.
distdir=.
globalprefix=glite
package=gsoap-plugin
version=1.0.0
PREFIX=/opt/glite

glite_location=/opt/glite
globus_prefix=/opt/globus
nothrflavour=gcc32
thrflavour=gcc32pthr
gsoap_prefix=/opt/gsoap
gsplugin_version_checking=yes

CC=gcc

-include Makefile.inc
-include ../Makefile.inc

GSPLUGIN_DEBUG?=no

version_info=-version-info `echo ${version} | cut -d. -f1,2 | tr . :`

VPATH=${top_srcdir}/src:${top_srcdir}/test:${top_srcdir}/examples

TEST_LIBS:=-L${cppunit}/lib -lcppunit
TEST_INC:=-I${cppunit}/include

ifeq ($(gsplugin_version_checking),yes)
	DEBUG:=-g -O0 -Wall -DGSPLUGIN_DEBUG
else
	DEBUG:=-g -O0 -Wall
endif

ifeq ($(GSPLUGIN_VERSION_CHECKING),yes)
	DEBUG:=${DEBUG} -DCHECK_GSOAP_VERSION
endif

CFLAGS:= ${DEBUG} \
	-DVERSION=\"${version}\" \
	-I${top_srcdir}/src -I${top_srcdir}/interface -I. \
	-I${stagedir}/include \
	-I${gsoap_prefix}/include \
	-I${ares_prefix}/include \
	-I${globus_prefix}/include/${nothrflavour} \
	-I${globus_prefix}/include/${nothrflavour}/openssl \
	${COVERAGE_FLAGS} -D_GNU_SOURCE -DDATAGRID_EXTENSION

LDFLAGS:=${COVERAGE_FLAGS}

COMPILE:=libtool --mode=compile ${CC} ${CFLAGS}
LINK:=libtool --mode=link ${CC} -rpath ${stagedir}/lib ${LDFLAGS} 
LINKXX:=libtool --mode=link ${CXX} ${LDFLAGS} 
INSTALL:=libtool --mode=install install
LINKXX:=libtool --mode=link  ${CXX} -rpath ${stagedir}/lib ${LDFLAGS}

GLOBUS_INC:= -I${globus_prefix}/include/${nothrflavour}
GLOBUS_LIBS:= -L${globus_prefix}/lib \
	-lglobus_common_${nothrflavour} \
	-lglobus_gssapi_gsi_${nothrflavour} \

GSOAP_LIBS:= -L${gsoap_prefix}/lib -lgsoap

EX_LIBS:= ${GLOBUS_LIBS} -L${ares_prefix}/lib -lares

PATCHED_GSOAP_SRC:=stdsoap2_2.6.2.c

HDRS:=glite_gss.h glite_gsplugin.h

GSS_OBJS:=glite_gss.o
GSS_LOBJS:=${GSS_OBJS:.o=.lo}
GSS_THROBJS:=${GSS_OBJS:.o=.thr.o}
GSS_THRLOBJS:=${GSS_OBJS:.o=.thr.lo}

GSS_STATICLIB:=libglite_security_gss_${nothrflavour}.a
GSS_THRSTATICLIB:=libglite_security_gss_${thrflavour}.a
GSS_LTLIB:=libglite_security_gss_${nothrflavour}.la
GSS_THRLTLIB:=libglite_security_gss_${thrflavour}.la

OBJS:=${GSS_OBJS} glite_gsplugin.o
LOBJS:=${GSS_OBJS:.o=.lo}
THROBJS:=${GSS_OBJS:.o=.thr.o}
THRLOBJS:=${GSS_OBJS:.o=.thr.lo}

STATICLIB:=libglite_security_gsoap_plugin_${nothrflavour}.a
THRSTATICLIB:=libglite_security_gsoap_plugin_${thrflavour}.a
LTLIB:=libglite_security_gsoap_plugin_${nothrflavour}.la
THRLTLIB:=libglite_security_gsoap_plugin_${thrflavour}.la

${GSS_STATICLIB}: ${GSS_OBJS}
	ar crv $@ ${GSS_OBJS}
	ranlib $@

${GSS_THRSTATICLIB}: ${GSS_THROBJS}
	ar crv $@ ${GSS_THROBJS}
	ranlib $@

${GSS_LTLIB}: ${GSS_OBJS}
	${LINK} ${version_info} -o $@ ${GSS_LOBJS} 

${GSS_THRLTLIB}: ${GSS_THROBJS}
	${LINK} ${version_info} -o $@ ${GSS_THRLOBJS} 

${STATICLIB}: ${OBJS}
	ar crv $@ ${OBJS}
	ranlib $@

${THRSTATICLIB}: ${THROBJS}
	ar crv $@ ${THROBJS}
	ranlib $@

${LTLIB}: ${OBJS}
	${LINK} ${version_info} -o $@ ${LOBJS}

${THRLTLIB}: ${THROBJS}
	${LINK} ${version_info} -o $@ ${THRLOBJS}

default: all

all compile: ${GSS_STATICLIB} ${GSS_LTLIB} ${GSS_THRSTATICLIB} ${GSS_THRLTLIB} ${STATICLIB} ${LTLIB} ${THRSTATICLIB} ${THRLTLIB} examples

v: 
	echo "check: ${CHECK_GSOAP_VERSION}"
	echo "DEBUG: ${DEBUG}"

check: compile check.gss

check.gss: test_gss
	# ./test_gss
	echo test_gss not run automatically util we have got some credentials

test_gss: test_gss.o
	${LINKXX} -o $@ test_gss.o ${GSS_LTLIB} ${TEST_LIBS} ${GLOBUS_LIBS} ${EX_LIBS}

test_coverage:
	-mkdir coverage
	cd coverage && $(MAKE) -f ../Makefile top_srcdir=../../ COVERAGE_FLAGS="-fprofile-arcs -ftest-coverage" check
	cd coverage && for i in ${OBJS}; do gcov -o .libs/ $$i ; done


examples: soap_gen wscalc_clt_ex wscalc_srv_ex wscalc_srv_ex2

GSOAP_FPREFIX:= GSOAP_

ifeq ($(GSPLUGIN_DEBUG),yes)
	WSCALC_CLT_OBJS = wscalc_clt_ex.o ${GSOAP_FPREFIX}C.o ${GSOAP_FPREFIX}Client.o \
					  stdsoap2.o
	WSCALC_SRV_OBJS = wscalc_srv_ex.o ${GSOAP_FPREFIX}C.o ${GSOAP_FPREFIX}Server.o \
					  stdsoap2.o
	WSCALC_SRV2_OBJS = wscalc_srv_ex2.o ${GSOAP_FPREFIX}C.o ${GSOAP_FPREFIX}Server.o \
					  stdsoap2.o
else
	WSCALC_CLT_OBJS = wscalc_clt_ex.o ${GSOAP_FPREFIX}C.o ${GSOAP_FPREFIX}Client.o
	WSCALC_SRV_OBJS = wscalc_srv_ex.o ${GSOAP_FPREFIX}C.o ${GSOAP_FPREFIX}Server.o
	WSCALC_SRV2_OBJS = wscalc_srv_ex2.o ${GSOAP_FPREFIX}C.o ${GSOAP_FPREFIX}Server.o
endif


wscalc_clt_ex: ${WSCALC_CLT_OBJS} ${STATICLIB}
	${LINK} -o $@ ${STATICLIB} ${GSOAP_LIBS} ${EX_LIBS} ${WSCALC_CLT_OBJS}

wscalc_srv_ex: ${WSCALC_SRV_OBJS} ${STATICLIB}
	${LINK} -o $@ ${STATICLIB} ${GSOAP_LIBS} ${EX_LIBS} ${WSCALC_SRV_OBJS}

wscalc_srv_ex2: ${WSCALC_SRV2_OBJS} ${STATICLIB}
	${LINK} -o $@ ${STATICLIB} ${GSOAP_LIBS} ${EX_LIBS} ${WSCALC_SRV2_OBJS}

${GSOAP_FPREFIX}H.h ${GSOAP_FPREFIX}C.c ${GSOAP_FPREFIX}Server.c ${GSOAP_FPREFIX}Client.c ${GSOAP_FPREFIX}ServerLib.c ${GSOAP_FPREFIX}ClientLib.c soap_gen:  calc.h.S
	${gsoap_prefix}/bin/soapcpp2 -c -p ${GSOAP_FPREFIX} ${top_srcdir}/examples/calc.h.S

wscalc_clt_ex.o: wscalc_clt_ex.c
	${CC} -c ${CFLAGS} ${GLOBUS_INC} -o $@ $<
	
stdsoap2.o: ${gsoap_prefix}/devel/stdsoap2.c
	${CC} -c ${CFLAGS} -o $@ $<

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
	-mkdir -p ${PREFIX}/lib
	-mkdir -p ${PREFIX}/share/doc/${package}-${version}
	-mkdir -p ${PREFIX}/include/glite/security/${package}
	${INSTALL} -m 644 ${GSS_LTLIB} ${GSS_THRLTLIB} ${PREFIX}/lib
	${INSTALL} -m 644 ${top_srcdir}/LICENSE ${PREFIX}/share/doc/${package}-${version}
	cd ${top_srcdir}/interface && ${INSTALL} -m 644 ${HDRS} ${PREFIX}/include/glite/security/
	if [ x${DOSTAGE} = xyes ]; then \
		install -m 644 ${GSS_STATICLIB} ${GSS_THRSTATICLIB} ${STATICLIB} ${THRSTATICLIB} ${PREFIX}/lib; \
		install -m 644 ${top_srcdir}/src/${PATCHED_GSOAP_SRC} ${PREFIX}/lib; \
	fi

clean:

%.o: %.c
	${COMPILE} ${GLOBUS_INC} -o $@ -c $<

%.thr.o: %.c
	${COMPILE} ${GLOBUS_THR_INC} -o $@ -c $<

test_gss.o: %.o: %.cpp
	${CXX} -c ${CFLAGS} ${GLOBUSINC} ${TEST_INC} $<

soap_version.h:
	${gsoap_prefix}/bin/soapcpp2 /dev/null
	perl -ne '$$. == 2 && /.*([0-9])\.([0-9])\.(\S+)\s.*/ && printf "#define GSOAP_VERSION %d%02d%02d\n#define GSOAP_MIN_VERSION \"$$3\"\n",$$1,$$2,$$3' soapH.h >$@
	-rm soapC.cpp soapH.h soapStub.h soapClient.cpp soapServer.cpp soapClientLib.cpp soapServerLib.cpp

glite_gsplugin.o: soap_version.h

