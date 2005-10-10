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

CC=gcc

gsoap_versions?=2.6.2 2.7.0

-include Makefile.inc
-include ../Makefile.inc

GSPLUGIN_DEBUG?=no
GSPLUGIN_VERSION_CHECKING?=yes

version_info=-version-info `echo ${version} | cut -d. -f1,2 | tr . :`

VPATH=${top_srcdir}/src:${top_srcdir}/test:${top_srcdir}/examples

TEST_LIBS:=-L${cppunit}/lib -lcppunit
TEST_INC:=-I${cppunit}/include

default: all

#ifeq ($(GSPLUGIN_DEBUG),yes)
#	DEBUG:=-g -O0 -Wall -DGSPLUGIN_DEBUG
#else
#	DEBUG:=-g -O0 -Wall
#endif

#ifeq ($(GSPLUGIN_VERSION_CHECKING),yes)
#	DEBUG:=${DEBUG} -DCHECK_GSOAP_VERSION
#endif

CFLAGS:= ${DEBUG} \
	-DVERSION=\"${version}\" \
	-I. -I${top_srcdir}/interface \
	-I${stagedir}/include \
	-I${globus_prefix}/include/${nothrflavour} \
	-I${globus_prefix}/include/${nothrflavour}/openssl \
	-I${ares_prefix}/include \
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

HDRS:=glite_gss.h glite_gsplugin.h

GSS_OBJS:=glite_gss.o
GSS_LOBJS:=${GSS_OBJS:.o=.lo}
GSS_THROBJS:=${GSS_OBJS:.o=.thr.o}
GSS_THRLOBJS:=${GSS_OBJS:.o=.thr.lo}

GSS_STATICLIB:=libglite_security_gss_${nothrflavour}.a
GSS_THRSTATICLIB:=libglite_security_gss_${thrflavour}.a
GSS_LTLIB:=libglite_security_gss_${nothrflavour}.la
GSS_THRLTLIB:=libglite_security_gss_${thrflavour}.la

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
	${LINK} ${version_info} -o $@ ${GSS_LOBJS} ${EX_LIBS}

${GSS_THRLTLIB}: ${GSS_THROBJS}
	${LINK} ${version_info} -o $@ ${GSS_THRLOBJS} ${EX_LIBS}


OBJS:=${GSS_OBJS} glite_gsplugin.o
LOBJS:=${OBJS:.o=.lo}
THROBJS:=${OBJS:.o=.thr.o}
THRLOBJS:=${OBJS:.o=.thr.lo}

dotless_ver:=${shell echo ${gsoap_version} | tr -d '.'}
STATICLIB_S:=libglite_security_gsoap_plugin_${dotless_ver}_${nothrflavour}.a
THRSTATICLIB_S:=libglite_security_gsoap_plugin_${dotless_ver}_${thrflavour}.a
LTLIB_S:=libglite_security_gsoap_plugin_${dotless_ver}_${nothrflavour}.la
THRLTLIB_S:=libglite_security_gsoap_plugin_${dotless_ver}_${thrflavour}.la

OBJS_S:=${GSS_OBJS} glite_gsplugin.o stdsoap2.o
LOBJS_S:=${OBJS_S:.o=.lo}
THROBJS_S:=${OBJS_S:.o=.thr.o}
THRLOBJS_S:=${OBJS_S:.o=.thr.lo}

${STATICLIB}: ${OBJS}
	ar crv $@ ${OBJS}
	ranlib $@

${THRSTATICLIB}: ${THROBJS}
	ar crv $@ ${THROBJS}
	ranlib $@

${LTLIB}: ${OBJS} 
	${LINK} ${version_info} -o $@ ${LOBJS} ${EX_LIBS}

${THRLTLIB}: ${THROBJS}
	${LINK} ${version_info} -o $@ ${THRLOBJS} ${EX_LIBS}

${STATICLIB_S}: ${OBJS_S}
	ar crv $@ ${OBJS_S}
	ranlib $@

${THRSTATICLIB_S}: ${THROBJS_S}
	ar crv $@ ${THROBJS_S}
	ranlib $@

${LTLIB_S}: ${OBJS_S} 
	${LINK} ${version_info} -o $@ ${LOBJS_S} ${EX_LIBS}

${THRLTLIB_S}: ${THROBJS_S}
	${LINK} ${version_info} -o $@ ${THRLOBJS_S} ${EX_LIBS}

all compile: \
	link-gsoap-default \
	${GSS_STATICLIB} ${GSS_LTLIB} ${GSS_THRSTATICLIB} ${GSS_THRLTLIB} \
	${STATICLIB} ${LTLIB} ${THRSTATICLIB} ${THRLTLIB} \
	all-libs-with-soap \
	examples

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


examples: link-gsoap-default soap_gen wscalc_clt_ex wscalc_srv_ex wscalc_srv_ex2

all-libs-with-soap:
	for v in ${gsoap_versions}; do	\
		${MAKE} gsoap_version=$$v libs-with-soap; \
	done

libs-with-soap: clean-objs link-gsoap ${LTLIB_S} ${THRLTLIB_S} ${STATICLIB_S} ${THRSTATICLIB_S}

link-gsoap:
	if [ -f ${top_srcdir}/src/stdsoap2_${gsoap_version}.c ]; then \
		ln -sf ${top_srcdir}/src/stdsoap2_${gsoap_version}.c stdsoap2.c; \
		ln -sf ${top_srcdir}/src/stdsoap2_${gsoap_version}.h stdsoap2.h; \
	elif [ -f ${repository}/${gsoap_name}/${gsoap_version}/src/stdsoap2.c ]; then \
		ln -sf ${repository}/${gsoap_name}/${gsoap_version}/src/stdsoap2.[ch] .; \
	elif [ -f ${repository}/${gsoap_name}/${gsoap_version}/${gsoap_platform}/stdsoap2.c ]; then \
		ln -sf ${repository}/${gsoap_name}/${gsoap_version}/${gsoap_platform}/stdsoap2.[ch] .; \
	else false ; \
	fi

link-gsoap-default:
	${MAKE} link-gsoap gsoap_version=${gsoap_default_version}

clean-objs:
	rm -f *.o *.lo soap_version.h

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


dotless_def_ver:=${shell echo ${gsoap_default_version} | tr -d .}
EXALIB:=libglite_security_gsoap_plugin_${dotless_def_ver}_${nothrflavour}.la

wscalc_clt_ex: ${WSCALC_CLT_OBJS} ${STATICLIB}
	${LINK} -o $@ ${WSCALC_CLT_OBJS} ${EXALIB}

wscalc_srv_ex: ${WSCALC_SRV_OBJS} ${STATICLIB}
	${LINK} -o $@ ${WSCALC_SRV_OBJS} ${EXALIB}

wscalc_srv_ex2: ${WSCALC_SRV2_OBJS} ${STATICLIB}
	${LINK} -o $@ ${WSCALC_SRV2_OBJS} ${EXALIB}


soapcpp:=${shell if [ -x ${gsoap_prefix}/bin/soapcpp2 ]; then \
	echo ${gsoap_prefix}/bin/soapcpp2; \
	else echo ${gsoap_prefix}/soapcpp2; \
	fi}

${GSOAP_FPREFIX}H.h ${GSOAP_FPREFIX}C.c ${GSOAP_FPREFIX}Server.c ${GSOAP_FPREFIX}Client.c ${GSOAP_FPREFIX}ServerLib.c ${GSOAP_FPREFIX}ClientLib.c soap_gen:  calc.h.S
	${soapcpp} -c -p ${GSOAP_FPREFIX} ${top_srcdir}/examples/calc.h.S

wscalc_clt_ex.o: wscalc_clt_ex.c
	${CC} -c ${CFLAGS} ${GLOBUS_INC} -o $@ $<
	
#stdsoap2_2.6.2.o: stdsoap2_2.6.2.c stdsoap2_2.6.2.h
#	ln -sf ${top_srcdir}/src/stdsoap2_2.6.2.h stdsoap2.h
#	libtool --mode=compile ${CC} -c ${DEBUG} -I${top_srcdir}/src -I${top_srcdir}/interface -I. -o $@ $<
#	rm -f stdsoap2.h
#
#stdsoap2.o: ${gsoap_prefix}/devel/stdsoap2.c
#	${CC} -c ${CFLAGS} -o $@ $<

doc:

stage:
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
	${INSTALL} -m 755 ${GSS_LTLIB} ${GSS_THRLTLIB} ${PREFIX}/lib
	${INSTALL} -m 644 ${top_srcdir}/LICENSE ${PREFIX}/share/doc/${package}-${version}
	cd ${top_srcdir}/interface && ${INSTALL} -m 644 ${HDRS} ${PREFIX}/include/glite/security/
	if [ x${DOSTAGE} = xyes ]; then \
		install -m 644 ${GSS_STATICLIB} ${GSS_THRSTATICLIB} ${STATICLIB} ${THRSTATICLIB} ${PREFIX}/lib; \
	fi
	for v in ${gsoap_versions}; do \
		${MAKE} install-soaplib gsoap_version=$$v PREFIX=${PREFIX}; \
	done

install-soaplib:
	${INSTALL} -m 755 ${LTLIB_S} ${THRLTLIB_S} ${PREFIX}/lib

clean:

%.o: %.c
	${COMPILE} ${GLOBUS_INC} -o $@ -c $<

%.thr.o: %.c
	${COMPILE} ${GLOBUS_THR_INC} -o $@ -c $<

test_gss.o: %.o: %.cpp
	${CXX} -c ${CFLAGS} ${GLOBUSINC} ${TEST_INC} $<

soap_version.h:
	head stdsoap2.h | \
	perl -ne '/^stdsoap2.h\s+([0-9])\.([0-9])\.(\S+)\s.*/ && printf "#define GSOAP_VERSION %d%02d%02d\n#define GSOAP_MIN_VERSION \"$$3\"\n#ident \"soap_version.h $$1.$$2.$$3\"\n",$$1,$$2,$$3' >$@

glite_gsplugin.o: soap_version.h

