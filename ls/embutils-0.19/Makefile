INSTALL=install
prefix=/opt/diet
# Set the following to install to a different root
#DESTDIR=/tmp/fefix
# Set the following to use the diet libc
DIET=diet -Os

FLAGS=

PRGS=sync echo chroot mkdir rmdir mknod pwd true false clear chown chgrp \
chmod sleep tty tar basename df tee uname arch hostname env time dirname \
domainname id ln mv cp yes which cat rm wc ls whoami mkfifo head install \
sosrm soscp sosmv sosln soslns md5sum sleep2 allinone uniq tr mesg du \
uuencode uudecode nohup nice cmp mktemp truncate strings test date \
printenv chrootuid renice

OS:=$(shell uname)
ifeq ($(OS),Linux)
PRGS+=chvt dmesg kill dd write tail mount pivot_root insmod rmmod \
lsmod touch umount
endif

ARCH:=$(shell uname -m | sed 's/i[4-9]86/i386/')

OBJDIR:=bin-$(ARCH)
TARGETS=$(patsubst %,$(OBJDIR)/%,$(PRGS))

all: $(OBJDIR) $(TARGETS)

CFLAGS=-pipe -Os -fomit-frame-pointer

CROSS=

#CFLAGS=-pipe -g

$(OBJDIR)/%.o: %.c
	@test -d $(OBJDIR) || mkdir $(OBJDIR)
	$(DIET) $(CROSS)gcc $(CFLAGS) $(FLAGS) -c $< -o $@
	$(CROSS)strip -x -R .comment -R .note $@

$(OBJDIR)/%.o: %.S $(OBJDIR)
	@test -d $(OBJDIR) || mkdir $(OBJDIR)
	$(DIET) $(CROSS)gcc -pipe -c $< -o $@

$(OBJDIR)/%: $(OBJDIR)/%.o
	$(DIET) $(CROSS)gcc -o $@ $^
	$(CROSS)strip -R .note -R .comment $@

$(OBJDIR)/uniq: $(OBJDIR)/uniq.o $(OBJDIR)/fmt_ulongpadright.o $(OBJDIR)/fmt_ulong.o

$(OBJDIR)/renice: $(OBJDIR)/renice.o $(OBJDIR)/fmt_long.o $(OBJDIR)/fmt_ulong.o

$(OBJDIR)/cp: $(OBJDIR)/mv
	ln -s mv $@

$(OBJDIR)/chgrp: $(OBJDIR)/chown
	-ln -s chown $@

$(OBJDIR)/mkfifo: $(OBJDIR)/mknod
	ln -s mknod $@

$(OBJDIR)/mknod.o $(OBJDIR)/mkdir.o: error.c parsemode.c

$(OBJDIR)/chown.o: openreadclose.c split.c mygetpwnam.c mygetgrnam.c

$(OBJDIR)/tar.o $(OBJDIR)/id.o $(OBJDIR)/chown.o: split.c mygetgrnam.c mygetpwnam.c

$(OBJDIR)/ls.o: embfeatures.h

$(OBJDIR)/touch.o: error.c

$(OBJDIR):
	@test -d $(OBJDIR) || mkdir $(OBJDIR)

$(OBJDIR)/test.o: fmt_ulong.c scan_uint.c scan_int.c

clean:
	rm -rf *.o bin-* $(PRGS)

VERSION=embutils-$(shell head -n 1 CHANGES|sed 's/://')
CURNAME=$(notdir $(shell pwd))

rename:
	if test $(CURNAME) != $(VERSION); then cd .. && mv $(CURNAME) $(VERSION); fi

tarball: rename
	cd .. && tar cvvf $(VERSION).tar.bz2 --use=bzip2 --exclude CVS --exclude bin-* $(VERSION)

alpha sparc ppc arm i386:
	$(MAKE) ARCH=$@ CROSS=$@-linux-

mips:
	$(MAKE) ARCH=$@ CROSS=$@-linux-gnu-

cross:
	$(MAKE) arm sparc ppc alpha i386 mips

.PHONY: install

install: $(TARGETS)
	test -d $(DESTDIR)$(prefix)/bin || mkdir -p $(DESTDIR)$(prefix)/bin
	$(INSTALL) $(TARGETS) $(DESTDIR)$(prefix)/bin
	-ln -f $(DESTDIR)$(prefix)/bin/chown $(DESTDIR)$(prefix)/bin/chgrp
	-ln -f $(DESTDIR)$(prefix)/bin/mv $(DESTDIR)$(prefix)/bin/cp
	-ln -f $(DESTDIR)$(prefix)/bin/mknod $(DESTDIR)$(prefix)/bin/mkfifo

uninstall:
	rm -f $(patsubst %,$(DESTDIR)$(prefix)/bin/%,$(PRGS))
