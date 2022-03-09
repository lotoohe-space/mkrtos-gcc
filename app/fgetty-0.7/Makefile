# Uncomment the following if you are a distribution maker and want to
# install to somewhere else than /
#DESTDIR=/tmp/fefix

all: fgetty login login2 checkpassword

DIET=diet -Os
#CROSS=arm-linux-
CROSS=
LDFLAGS=-s

%.o: %.c
#	gcc -march=i386 -mcpu=i386 -pipe -Os -fomit-frame-pointer -I../dietlibc/include -c $^ -DTEST
	$(DIET) $(CROSS)$(CC) -pipe -Os -fomit-frame-pointer -I../dietlibc/include -c $^ -DTEST
#	gcc -march=i386 -mcpu=i386 -pipe -g -I../dietlibc/include -DTEST -c $^
	$(CROSS)strip -x -R .comment -R .note $@

%: %.o
	$(DIET) $(CROSS)$(CC) -o $@ $^ $(LDFLAGS)

fgetty: fgetty.o fmt_ulong.o

login: login.o
login2: login2.o
checkpassword: checkpassword.o
checkpassword-pam: checkpassword-pam.o checkpassword-pam2.o
	$(CROSS)$(CC) -o $@ $^ -lmisc $(LDFLAGS)

debug: fgetty.c fmt_ulong.o
	gcc -g -o debug fgetty.c fmt_ulong.o -DDEBUG

install:
	install -d $(DESTDIR)/bin $(DESTDIR)/sbin $(DESTDIR)/usr/man/man8
	install login $(DESTDIR)/bin/login1
	install login2 $(DESTDIR)/bin
	install fgetty $(DESTDIR)/sbin
	install checkpassword $(DESTDIR)/bin/checkpassword.login
	install -m 644 fgetty.8 $(DESTDIR)/usr/man/man8/fgetty.8
	@echo "now change your /etc/inittab to do something like"
	@echo "  1:123:respawn:/sbin/fgetty /dev/vc/1 --noclear"

clean:
	rm -f *.o fgetty debug dietgetty login login2 checkpassword core

sigs: fgetty.sig login.sig login2.sig checkpassword.sig

.SUFFIXES: .sig
%.sig: %
	gpg --detach-sign $<

VERSION=fgetty-$(shell head -n 1 CHANGES|sed 's/://')
CURNAME=$(notdir $(shell pwd))

tar: clean rename
	cd ..; tar cvvf $(VERSION).tar.bz2 $(VERSION) --use=bzip2 --exclude CVS

rename:
	if test $(CURNAME) != $(VERSION); then cd .. && mv $(CURNAME) $(VERSION); fi

