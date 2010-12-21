top_srcdir=..
stagedir=.
PREFIX=/opt/glite
package=glite-px-myproxy-config
ifndef MYRPMDIR
export MYRPMDIR=$(shell pwd)/../RPMTMP
endif

-include Makefile.inc
-include ../project/version.properties

all:

install: 
	@echo installing ...
	@mkdir -p $(PREFIX)/etc/init.d
	@install -m 0755 ${top_srcdir}/myproxy-initd $(PREFIX)/etc/init.d/myproxy

check:

rpm:
	mkdir -p dist dist/${package}
	rm -rf $(MYRPMDIR) dist
	mkdir -p $(MYRPMDIR)/SOURCES $(MYRPMDIR)/SPECS $(MYRPMDIR)/BUILD \
		$(MYRPMDIR)/SRPMS $(MYRPMDIR)/RPMS/noarch $(MYRPMDIR)/BUILDROOT 
	$(MAKE) install PREFIX=dist/${package}
	(cd dist; tar -cf - .) | gzip > $(MYRPMDIR)/SOURCES/${package}.tgz
	cat ../project/myproxy-config.spec | \
		sed "s/^Name:.*/Name: $(package)/" | \
		sed "s/^Version:.*/Version: $(module.version)/" | \
		sed "s/^Release:.*/Release: $(module.age)/"  > ${package}.spec
	rpmbuild -ba ${package}.spec --buildroot $(MYRPMDIR)/BUILDROOT --define "_topdir $(MYRPMDIR)"

clean:
	rm -f *~ test/*~ etc/*~ doc/*~ src/*~
	rm -rf rpmbuild dist

.PHONY: default all check install clean
