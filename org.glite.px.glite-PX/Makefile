top_srcdir=..
stagedir=.
package=glite-PX
version=0.0.0
PREFIX=/opt/glite

-include Makefile.inc
-include ../project/version.properties

default all:
	echo -e "${gLiteCopyrightText}\n\n${gLiteLicenseText}" > LICENSE
	cp LICENSE COPYRIGHT
	echo ${module.version}-${module.age} > node-version
	echo ${platformArch} > arch
	echo "NA" > service
	echo "NA" > update

install:
	mkdir -p ${PREFIX}/release/glite-PX
	cp -fp LICENSE COPYRIGHT node-version arch service update ${PREFIX}/release/glite-PX/

stage:
	$(MAKE) install PREFIX=${stagedir}

check:

clean:
	rm -rvf log.xml project/ rpmbuild/ RPMS/ tgz/ debian/

.PHONY: default all check install stage clean
