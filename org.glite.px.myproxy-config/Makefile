top_srcdir=..
stagedir=.
PREFIX=/opt/glite

-include Makefile.inc
-include ../project/version.properties

all:

install: 
	@echo installing ...
	@install -D -m 0755 ${top_srcdir}/myproxy-initd $(PREFIX)/etc/init.d/myproxy

check:

clean:
	rm -f *~ test/*~ etc/*~ doc/*~ src/*~
	rm -rf rpmbuild

.PHONY: default all check install clean
