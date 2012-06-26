top_srcdir=..
stagedir=.
package=glite-PX
version=0.0.0
PREFIX=/opt/glite
prefix=

-include Makefile.inc
-include ../project/version.properties

version=${module.version}

SOURCES=Makefile

default all:

install:

stage:
	$(MAKE) install PREFIX=${stagedir}

check:

clean:
	rm -rvf dist ${package}-*.tar.gz

distclean:
	rm -rvf Makefile.inc *.spec debian/

.PHONY: default all check install stage clean distclean dist distcheck
