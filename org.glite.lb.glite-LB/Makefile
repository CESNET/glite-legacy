top_srcdir=..
stagedir=.
package=glite-LB
version=0.0.0
PREFIX=/opt/glite
prefix=

default: all

-include Makefile.inc
-include ${top_srcdir}/project/version.properties

version=${module.version}

SOURCES=Makefile

all:

install:

stage:
	$(MAKE) install PREFIX=${stagedir}

check:

clean:
	rm -rvf dist ${package}-*.tar.gz

distclean:
	rm -rvf project/changelog Makefile.inc *.spec debian/

.PHONY: default all check install stage clean dist distclean distcheck
