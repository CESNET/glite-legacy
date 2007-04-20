# defaults
top_srcdir=..
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

gsoap_versions?=2.6.2 2.7.0f 2.7.6b 2.7.9b 2.7.9d
gsoap_versions:=${shell if ! echo $gsoap_versions | grep "\<${gsoap_default_version}\>" >/dev/null; then echo "${gsoap_default_version} "; else echo ne; fi} ${gsoap_versions}

-include Makefile.inc
-include ../Makefile.inc

GSPLUGIN_DEBUG?=no
GSPLUGIN_VERSION_CHECKING?=yes


offset=1
version_info:=-version-info ${shell \
	perl -e '$$,=":"; @F=split "\\.","${version}"; print $$F[0]+$$F[1]+${offset},$$F[2],$$F[1]' }

# version_info=-version-info `echo ${version} | cut -d. -f1,2 | tr . :`

ext_repository:=${shell if [ -d "${repository}/externals" ]; then \
	echo "${repository}/externals"; \
	else echo "${repository}"; \
	fi}

gsoap_version=${gsoap_default_version}
gsoap_prefix?=${ext_repository}/${gsoap_name}/${gsoap_version}/${gsoap_platform}

VPATH=${top_srcdir}/src:${top_srcdir}/test:${top_srcdir}/examples

TEST_LIBS:=-L${cppunit}/lib -lcppunit
TEST_INC:=-I${cppunit}/include

default: all

#ifeq ($(GSPLUGIN_DEBUG),yes)
#	DEBUG:=-g -O0 -Wall -DGSPLUGIN_DEBUG
#else
#	DEBUG:=-g -O0 -Wall
#endif

DEBUG:=-g -O0 -Wall

#ifeq ($(GSPLUGIN_VERSION_CHECKING),yes)
#	DEBUG:=${DEBUG} -DCHECK_GSOAP_VERSION
#endif

CFLAGS:= ${DEBUG} \
	-DVERSION=\"${version}\" \
	-DWITH_NONAMESPACES \
	-I. -I${top_srcdir}/interface \
	-I${stagedir}/include \
	-I${cares_prefix}/include \
	-I${gsoap_prefix}/include \
	${COVERAGE_FLAGS} -D_GNU_SOURCE -DDATAGRID_EXTENSION

LDFLAGS:=${COVERAGE_FLAGS}

COMPILE:=libtool --mode=compile ${CC} ${CFLAGS}
LINK:=libtool --mode=link ${CC} -rpath ${stagedir}/lib ${LDFLAGS} 
LINKXX:=libtool --mode=link ${CXX} ${LDFLAGS} 
INSTALL:=libtool --mode=install install
LINKXX:=libtool --mode=link  ${CXX} -rpath ${stagedir}/lib ${LDFLAGS}

GLOBUS_INC:= -I${globus_prefix}/include/${nothrflavour}
GLOBUS_THR_INC:= -I${globus_prefix}/include/${thrflavour}

GLOBUS_LIBS:= -L${globus_prefix}/lib \
	-lglobus_common_${nothrflavour} \
	-lglobus_gssapi_gsi_${nothrflavour} \

GLOBUS_THR_LIBS:= -L${globus_prefix}/lib \
	-lglobus_common_${thrflavour} \
	-lglobus_gssapi_gsi_${thrflavour}

GSOAP_LIBS:= -L${gsoap_prefix}/lib -lgsoap
ARES_LIBS:=-L${cares_prefix}/lib -lcares

EX_LIBS:= ${GLOBUS_LIBS} ${ARES_LIBS}
EX_THRLIBS := ${GLOBUS_THR_LIBS} ${ARES_LIBS}

HDRS:=glite_gss.h glite_gsplugin.h glite_gscompat.h

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
	${LINK} ${version_info} -o $@ ${GSS_THRLOBJS} ${EX_THRLIBS}


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
	${LINK} ${version_info} -o $@ ${THRLOBJS} ${EX_THRLIBS}

${STATICLIB_S}: ${OBJS_S}
	ar crv $@ ${OBJS_S}
	ranlib $@

${THRSTATICLIB_S}: ${THROBJS_S}
	ar crv $@ ${THROBJS_S}
	ranlib $@

${LTLIB_S}: ${OBJS_S} 
	${LINK} ${version_info} -o $@ ${LOBJS_S} ${EX_LIBS}

${THRLTLIB_S}: ${THROBJS_S}
	${LINK} ${version_info} -o $@ ${THRLOBJS_S} ${EX_THRLIBS}

all compile: \
	${GSS_STATICLIB} ${GSS_LTLIB} ${GSS_THRSTATICLIB} ${GSS_THRLTLIB} \
	${STATICLIB} ${LTLIB} ${THRSTATICLIB} ${THRLTLIB} \
	all-libs-with-soap examples

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


examples: wscalc_clt_ex wscalc_srv_ex wscalc_srv_ex2

all-libs-with-soap:
	for v in ${gsoap_versions}; do	\
		dir=`echo $$v | tr -d .`; \
		mkdir $$dir; \
		( cd $$dir && ${MAKE} -f ../Makefile gsoap_version=$$v top_srcdir=../.. libs-with-soap) ; \
	done

libs-with-soap: ${LTLIB_S} ${THRLTLIB_S} ${STATICLIB_S} ${THRSTATICLIB_S}

gsoap_srcname=gsoap-`echo ${gsoap_version} | cut -d. -f1,2`

link-gsoap:
	if [ -f ${top_srcdir}/src/stdsoap2_${gsoap_version}.c ]; then \
		ln -sf ${top_srcdir}/src/stdsoap2_${gsoap_version}.c stdsoap2.c; \
		ln -sf ${top_srcdir}/src/stdsoap2_${gsoap_version}.h stdsoap2.h; \
	elif [ -f ${ext_repository}/${gsoap_name}/${gsoap_version}/src/stdsoap2.c ]; then \
		ln -sf ${ext_repository}/${gsoap_name}/${gsoap_version}/src/stdsoap2.[ch] .; \
	elif [ -f ${ext_repository}/${gsoap_name}/${gsoap_version}/${gsoap_platform}/stdsoap2.c ]; then \
		ln -sf ${ext_repository}/${gsoap_name}/${gsoap_version}/${gsoap_platform}/stdsoap2.[ch] .; \
	elif [ -f ${ext_repository}/${gsoap_name}/${gsoap_version}/src/stdsoap2.c ]; then \
		ln -sf ${ext_repository}/${gsoap_name}/${gsoap_version}/src/stdsoap2.[ch] .; \
	elif [ -f ${ext_repository}/${gsoap_name}/${gsoap_version}/src/${gsoap_srcname}/soapcpp2/stdsoap2.c ]; then \
		ln -sf ${ext_repository}/${gsoap_name}/${gsoap_version}/src/${gsoap_srcname}/soapcpp2/stdsoap2.[ch] .; \
	else false ; \
	fi

stdsoap2.c stdsoap2.h: link-gsoap

GSOAP_FPREFIX:= GSOAP_

WSCALC_CLT_OBJS = wscalc_clt_ex.o ${GSOAP_FPREFIX}C.o ${GSOAP_FPREFIX}Client.o
WSCALC_SRV_OBJS = wscalc_srv_ex.o ${GSOAP_FPREFIX}C.o ${GSOAP_FPREFIX}Server.o
WSCALC_SRV2_OBJS = wscalc_srv_ex2.o ${GSOAP_FPREFIX}C.o ${GSOAP_FPREFIX}Server.o

wscalc_clt_ex.o wscalc_srv_ex.o wscalc_srv_ex2.o: ${GSOAP_FPREFIX}H.h

wscalc_clt_ex: ${WSCALC_CLT_OBJS} ${LTLIB_S}
	${LINK} -o $@ ${WSCALC_CLT_OBJS} ${LTLIB_S}

wscalc_srv_ex: ${WSCALC_SRV_OBJS} ${LTLIB_S}
	${LINK} -o $@ ${WSCALC_SRV_OBJS} ${LTLIB_S}

wscalc_srv_ex2: ${WSCALC_SRV2_OBJS} ${LTLIB_S}
	${LINK} -o $@ ${WSCALC_SRV2_OBJS} ${LTLIB_S}


soapcpp:=${shell if [ -x ${gsoap_prefix}/bin/soapcpp2 ]; then \
	echo ${gsoap_prefix}/bin/soapcpp2; \
	else echo ${gsoap_prefix}/soapcpp2; \
	fi}

${GSOAP_FPREFIX}H.h ${GSOAP_FPREFIX}C.c ${GSOAP_FPREFIX}Server.c ${GSOAP_FPREFIX}Client.c ${GSOAP_FPREFIX}ServerLib.c ${GSOAP_FPREFIX}ClientLib.c:  calc.h.S
	${soapcpp} -c -p ${GSOAP_FPREFIX} ${top_srcdir}/examples/calc.h.S

wscalc_clt_ex.o: wscalc_clt_ex.c
	${CC} -c ${CFLAGS} ${GLOBUS_INC} -o $@ $<
	
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
		install -m 644 ${GSS_STATICLIB} ${GSS_THRSTATICLIB} ${PREFIX}/lib; \
	fi
	for v in ${gsoap_versions}; do \
		(cd `echo $$v | tr -d .` && ${MAKE} -f ../Makefile install-soaplib gsoap_version=$$v PREFIX=${PREFIX} ); \
	done

install-soaplib:
	${INSTALL} -m 755 ${LTLIB_S} ${THRLTLIB_S} ${PREFIX}/lib

clean:

%.o: %.c
	${COMPILE} ${GLOBUS_INC} -o $@ -c $<

%.thr.o: %.c
	${COMPILE} ${GLOBUS_THR_INC} -o $@ -c $<

test_gss.o: %.o: %.cpp
	${CXX} -c ${CFLAGS} ${GLOBUS_INC} ${TEST_INC} $<

glite_gsplugin.o, glite_gsplugin.thr.o: soap_version.h

soap_version.h: stdsoap2.h
	head stdsoap2.h | \
	perl -ne '/^stdsoap2.h\s+([0-9])\.([0-9])\.(\S+)\s.*/ && printf "#define GSOAP_VERSION %d%02d%02d\n#define GSOAP_MIN_VERSION \"$$3\"\n#ident \"soap_version.h $$1.$$2.$$3\"\n",$$1,$$2,$$3' >$@

glite_gsplugin.o: soap_version.h

