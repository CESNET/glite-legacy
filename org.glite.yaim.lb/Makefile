prefix=/opt/glite
package=glite-yaim-lb
name=$Name$
tag:=$(shell echo $(name) | sed 's/^[^:]*: //' )
version:=$(shell echo "$(tag)" | sed 's/^.*R_//' | sed 's/_/\./g')
release:=$(shell echo "$(version)" | sed 's/.*\.//')
version:=$(shell echo "$(version)" | sed 's/\(.*\)\.[0-9]*/\1/')

.PHONY: configure install clean rpm

all: configure

install: 
	@echo installing ...
	@mkdir -p $(prefix)/yaim/functions/
	@mkdir -p $(prefix)/yaim/functions/local
	@mkdir -p $(prefix)/yaim/node-info.d
	@install -m 0644 config/functions/config* $(prefix)/yaim/functions
	@install -m 0644 config/functions/local/config* $(prefix)/yaim/functions/local
	@install -m 0644 config/node-info.d/glite* $(prefix)/yaim/node-info.d

clean::
	rm -f *~ test/*~ etc/*~ doc/*~ src/*~  
	rm -rf rpmbuild 

rpm:
	@mkdir -p  RPMS
	@mkdir -p  rpmbuild/RPMS/noarch
	@mkdir -p  rpmbuild/SRPMS/
	@mkdir -p  rpmbuild/SPECS/
	@mkdir -p  rpmbuild/SOURCES/
	@mkdir -p  rpmbuild/BUILD/
	echo "|$(tag)|"
ifneq ("$(tag)","ame:")
	@sed -i 's/^Version:.*/Version: $(version)/' $(package).spec
	@sed -i 's/^Release:.*/Release: $(release)/' $(package).spec
endif
	@tar --gzip --exclude='*CVS*' -cf rpmbuild/SOURCES/${package}.src.tgz *
	rpmbuild -ba ${package}.spec
	cp rpmbuild/RPMS/noarch/*.rpm rpmbuild/SRPMS/*.rpm RPMS/.




