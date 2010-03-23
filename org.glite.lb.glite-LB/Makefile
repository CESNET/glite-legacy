top_srcdir=..
stagedir=.
package=glite-lb-yaim
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
	mkdir -p ${PREFIX}/release/glite-LB
	cp -fp LICENSE COPYRIGHT node-version arch service update ${PREFIX}/release/glite-LB/

stage:
	$(MAKE) install PREFIX=${stagedir}

check:

clean:

.PHONY: default all check install stage clean
