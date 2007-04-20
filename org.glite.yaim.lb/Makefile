PACKAGE_NAME=glite-yaim-lb
####################################################################
# Distribution Makefile
####################################################################

.PHONY: configure install clean

all: configure

####################################################################
# Prepare
####################################################################

prepare:
	rm -f *~ functions/*~ scripts/*~ examples/*~ 
	@mkdir -p $(prefix)/doc/html/web
	@mkdir -p $(prefix)/doc/man

####################################################################
# Configure
####################################################################

configure: 
	@echo "No configuration required, use either 'make install' or 'make rpm'."

####################################################################
# Compile
####################################################################

compile: 
	@echo "No compiling required, use either 'make install' or 'make rpm'."

####################################################################
# Install
####################################################################

install:
	@echo installing ...
	@mkdir -p $(prefix)/yaim/functions
	@mkdir -p $(prefix)/yaim/node-info.d
	@install -m 0644 config/functions/* $(prefix)/yaim/functions
	@install -m 0644 config/node-info.d/* $(prefix)/yaim/node-info.d

####################################################################
# Documentation
####################################################################

doc: man html

man: prepare

html: prepare

web: html

####################################################################
# Install Doc
####################################################################

install-doc: doc
	@echo installing  docs...

####################################################################
# Build Distribution
####################################################################

dist: prepare 
	@tar --gzip --exclude='*CVS*' -cf build/$(PACKAGE_NAME).src.tgz config doc examples src

rpm: dist
	@rpmbuild -ta build/$(PACKAGE_NAME).src.tgz 

clean::
	rm -f *~ test/*~ etc/*~ doc/*~ src/*~ $(PACKAGE_NAME).src.tgz 
	rm -rf build ${prefix}

