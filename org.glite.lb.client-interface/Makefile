include Makefile.inc

SUFFIXES = .T

VPATH=${interface}
AT3=perl -I${lbconfig} ${lbproject}/at3

STAGETO=${stageinc}/${globalprefix}/${lbprefix}
STATIC_H=consumer.h context.h dump.h load.h notification.h notifid.h purge.h
GEN_H=events.h jobstat.h producer.h

all stage: stage-gen stage-static
compile: generate

stage-static:
	-mkdir -p ${STAGETO}
	cd ${interface} && install -m 644 ${STATIC_H} ${STAGETO}

stage-gen: generate
	-mkdir -p ${STAGETO}
	install -m 644 ${GEN_H} ${STAGETO}

generate: ${GEN_H}

check: 
	@echo No unit test required for interface-only module.

%.h: %.h.T
	rm -f $@
	${AT3} $< >$@ || rm -f $@
	chmod -w $@ >/dev/null


