-include makefile.mk

PREFIX ?=/usr/local
LOCALE_DIR=$(PREFIX)/share/locale


# Building flags.
CFLAGS ?=-march=native -mtune=generic -O2 -Wall
VALAFLAGS:=$(foreach w,$(LDFLAGS) $(CFLAGS) $(CPPFLAGS),-X $(w))
CFLAGS +=$(shell pkg-config --cflags gtk+-2.0 dbus-1 x11 gio-unix-2.0) -I.
LDFLAGS+=$(shell pkg-config --libs gtk+-2.0 glib-2.0 dbus-1 x11 gio-unix-2.0)

# PO and MO files
LINGUAS= $(shell ls po/*.po)
I18N_MO= $(LINGUAS:.po=.mo)


all: obsession-exit obsession-logout $(I18N_MO)

.SUFFIXES: .c

.c.o:
	@echo "Compiling $<"
	@gcc -o $@ -c $< $(CFLAGS) $(CPPFLAGS)

obsession-exit: obsession-exit.o dbus-interface.o obsession-common.o config.h
	@echo "Building $@"
	@$(CC) -o $@ $(filter-out %.h,$^) $(LDFLAGS)
	@strip -s $@

obsession-logout: obsession-logout.o dbus-interface.o obsession-common.o config.h
	@echo "Building $@"
	@$(CC) -o $@ $(filter-out %.h,$^) $(LDFLAGS)
	@strip -s $@

po/%.mo: po/%.po
	msgfmt -o $@ $<

PHONY: clean install configure mrproper

mrproper: clean
	rm -f makefile.mk

clean:
	rm -f obsession-exit obsession-logout *.o $(I18N_MO)

configure:
	sed -i 's#define PREFIX.*#define PREFIX "$(PREFIX)"#' config.h
	echo "PREFIX ?= $(PREFIX)" > makefile.mk

install: all
	install -D -m0755 obsession-exit   $(DESTDIR)$(PREFIX)/bin/obsession-exit
	install -D -m0755 obsession-logout $(DESTDIR)$(PREFIX)/bin/obsession-logout
	# mo files.
	for f in $(I18N_MO) ; do \
		F=`basename $$f | sed 's/\.[^\.]*$$//'`;\
		install -D -m0644 $$f $(DESTDIR)$(LOCALE_DIR)/$$F/LC_MESSAGES/obsession-logout.mo;\
	done
	for f in `ls images/*.png`; do \
		install -D -m0644 $$f $(DESTDIR)$(PREFIX)/share/obsession/$$f;\
	done
