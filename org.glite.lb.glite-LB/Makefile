top_srcdir=..
stagedir=.
package=glite-LB
version=0.0.0
PREFIX=/opt/glite
prefix=

-include Makefile.inc
-include ${top_srcdir}/project/version.properties

version=${module.version}

SOURCES=Makefile

default all:

install:

stage:
	$(MAKE) install PREFIX=${stagedir}

check:

clean:
	rm -rvf log.xml rpmbuild/ RPMS/ tgz/ debian/
	rm -rvf dist ${package}-*.tar.gz

.PHONY: default all check install stage clean dist distcheck
