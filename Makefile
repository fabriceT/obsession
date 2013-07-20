-include makefile.mk

PREFIX ?=/usr/local
LOCALE_DIR =$(PREFIX)/share/locale


# Building flags.
CFLAGS ?=-march=i686 -mtune=generic -O2 -Wall
CFLAGS +=`pkg-config --cflags gtk+-2.0 dbus-1 x11 gio-unix-2.0` -I.
LIBS=`pkg-config --libs gtk+-2.0 glib-2.0 dbus-1 x11 gio-unix-2.0`

# PO and MO files
LINGUAS= $(shell ls po/*.po)
I18N_MO= $(LINGUAS:.po=.mo)


all: obsession-exit obsession-logout xdg-autostart $(I18N_MO)

.SUFFIXES: .c

.c.o:
	@echo "Compiling $<"
	@gcc -o $@ -c $< $(CFLAGS)

obsession-exit: obsession-exit.o dbus-interface.o obsession-common.o config.h
	@echo "Building $@"
	@$(CC) -o $@ $^ $(LIBS)
	@strip -s $@

obsession-logout: obsession-logout.o dbus-interface.o obsession-common.o config.h
	@echo "Building $@"
	@$(CC) -o $@ $^ $(LIBS)
	@strip -s $@

xdg-autostart:  autostart/xdg-autostart.vala
	@echo "Building $@"
	@valac -o $@ $^
	@strip -s $@

po/%.mo: po/%.po
	msgfmt -o $@ $<

PHONY: clean install configure mrproper

mrproper: clean
	rm makefile.mk

clean:
	rm obsession-exit obsession-logout xdg-autostart *.o $(I18N_MO)

configure:
	sed -i 's#define PREFIX.*#define PREFIX "$(PREFIX)"#' config.h
	echo "PREFIX ?= $(PREFIX)" > makefile.mk

install: all
	install -D -m0755 obsession-exit   $(PREFIX)/bin/obsession-exit
	install -D -m0755 obsession-logout $(PREFIX)/bin/obsession-logout
	install -D -m0755 xdg-autostart    $(PREFIX)/bin/xdg-autostart
	# mo files.
	for f in $(I18N_MO) ; do \
		F=`basename $$f | sed 's/\.[^\.]*$$//'`;\
		install -D -m0644 $$f $(LOCALE_DIR)/$$F/LC_MESSAGES/obsession-logout.mo;\
	done

	for f in `ls images/*.png`; do \
		install -D -m0644 $$f $(PREFIX)/share/obsession/$$f;\
	done
