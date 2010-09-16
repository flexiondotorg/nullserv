# Available CFLAGS:
# -DLOGGING    - log activity

VERSION=0.2
PREFIX=/usr/local/bin
CC=gcc
CFLAGS=-Os -s
       
all:
	$(CC) $(CFLAGS) -o nullserv nullserv.c
	strip nullserv

clean:
	rm nullserv

install:
	cp -f nullserv $(PREFIX)
	@echo ""
	@echo "Make sure you update your /etc/inetd.conf."

uninstall:
	rm -rf $(PREFIX)/nullserv

dist: clean
	cd ..; tar --exclude .bzr -czvf nullserv-v$(VERSION).tar.gz nullserv
