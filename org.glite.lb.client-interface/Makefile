# Default values
top_srcdir=.
builddir=build
top_builddir=${top_srcdir}/${builddir}
stagedir=.
distdir=.
globalprefix=glite
lbprefix=lb
package=glite-lb-client-interface
version=0.0.0
PREFIX=/opt/glite

-include Makefile.inc
-include ../project/version.properties

version=${module.version}

SUFFIXES = .T

VPATH=${top_srcdir}/interface
AT3=perl -I${top_srcdir}/project ${top_srcdir}/project/at3

STAGETO=include/${globalprefix}/${lbprefix}
STATIC_H=consumer.h context.h dump.h load.h notification.h notifid.h purge.h \
	statistics.h \
	Notification.h CountRef.h Job.h LoggingExceptions.h ServerConnection.h \

FAKE_H=consumer_fake.h producer_fake.h

GEN_H=events.h jobstat.h producer.h interface_version.h Event.h JobStatus.h


generate: ${GEN_H}

all compile: generate

check: 
	@echo No unit test required for interface-only module.

%.h: %.h.T
	rm -f $@
	${AT3} $< >$@ || rm -f $@
	chmod -w $@ >/dev/null

interface_version.h: ${top_srcdir}/project/version.properties
	echo "#define GLITE_LB_CLIENT_INTERFACE \"${version}\"" >$@ 

ifdef LB_STANDALONE
stage: generate
	$(MAKE) install PREFIX=${stagedir} DOSTAGE=yes
else
stage: generate doc
	$(MAKE) install PREFIX=${top_srcdir}/${stagedir} DOSTAGE=yes
endif

dist: distsrc distbin

distsrc:
	mkdir -p ${top_srcdir}/${package}-${version}
	cd ${top_srcdir} && GLOBIGNORE="${package}-${version}" && cp -Rf * ${package}-${version}
	cd ${top_srcdir} && tar -czf ${distdir}/${package}-${version}_src.tar.gz --exclude-from=project/tar_exclude ${package}-${version}
	rm -rf ${top_srcdir}/${package}-${version}

distbin:
	$(MAKE) install PREFIX=${top_srcdir}/tmpbuilddir
	cd ${top_srcdir}/tmpbuilddir && tar -czf ${top_srcdir}/${distdir}/${package}-${version}_bin.tar.gz *
	rm -rf ${top_srcdir}/tmpbuilddir
	
doc: generate 
	doxygen C.dox
# FIXME: cistka
#	doxygen CPP.dox

install: generate 
	-mkdir -p ${PREFIX}/${STAGETO}
	-mkdir -p ${PREFIX}/share/doc/${package}-${version}
	install -m 644 ${GEN_H} ${PREFIX}/${STAGETO}
	install -m 644 ${top_srcdir}/LICENSE ${PREFIX}/share/doc/${package}-${version}
	cd ${top_srcdir}/interface && install -m 644 ${STATIC_H} ${PREFIX}/${STAGETO}
ifndef LB_STANDALONE
# FIXME: cistka
#	cd ${top_srcdir}/doc && cp -r C CPP ${PREFIX}/share/doc/${package}-${version}
	cd ${top_srcdir}/doc && cp -r C ${PREFIX}/share/doc/${package}-${version}
endif
	if [ x${DOSTAGE} = xyes ]; then \
		cd ${top_srcdir}/interface && install -m 644 ${FAKE_H} ${PREFIX}/${STAGETO} ; \
	fi

clean:
	rm -f *.h
	
