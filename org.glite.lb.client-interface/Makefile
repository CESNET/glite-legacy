include Makefile.inc

SUFFIXES = .T

VPATH=${top_srcdir}/interface
AT3=perl -I${top_srcdir}/project ${top_srcdir}/project/at3

STAGETO=${stageinc}/${globalprefix}/${lbprefix}
STATIC_H=consumer.h context.h dump.h load.h notification.h notifid.h purge.h
GEN_H=events.h jobstat.h producer.h

stage: stage-gen stage-static
all compile: generate

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

clean:
