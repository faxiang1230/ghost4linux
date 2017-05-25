#!/usr/bin/make

TARGET = jetcat-mod
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/man/man1

all:	$(TARGET)

install: $(TARGET)
	install -d $(BINDIR)
	install -c -s -m 755 $(TARGET) $(BINDIR)
	install -c -m 644 jetcat.1 $(MANDIR)

uninstall:
	rm -f $(BINDIR)/$(TARGET) $(MANDIR)/jetcat.1

clean:
	rm -f $(TARGET)
