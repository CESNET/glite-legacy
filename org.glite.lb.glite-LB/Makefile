top_srcdir=..
stagedir=.
package=glite-lb-yaim
version=0.0.0
PREFIX=/opt/glite
prefix=

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
	mkdir -p ${DESTDIR}${PREFIX}${prefix}/release/glite-LB
	cp -fp LICENSE COPYRIGHT node-version arch service update ${DESTDIR}${PREFIX}${prefix}/release/glite-LB/

stage:
	$(MAKE) install PREFIX=${stagedir}

check:

clean:
	rm -rvf log.xml project/ rpmbuild/ RPMS/ tgz/ debian/

.PHONY: default all check install stage clean
