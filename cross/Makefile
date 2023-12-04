MAKE=make
PREFIX=$(shell pwd)
SYSROOT=CHANGEME
TARGET=i686-elf-c4os
BINUTILS=binutils-2.28
GCC=gcc-6.4.0

CLEAN_DIRS=binbuild-$(TARGET) gccbuild-$(TARGET)
CLEAN_SOURCE=$(BINUTILS) $(GCC)

.PHONY: all
all: bin/$(TARGET)-ld bin/$(TARGET)-gcc

.PHONY: get-archives
get-archives: $(BINUTILS).tar.gz $(GCC).tar.gz

$(BINUTILS).tar.gz.sig:
	wget "https://ftp.gnu.org/gnu/binutils/$(BINUTILS).tar.gz.sig" -O "$(BINUTILS).tar.gz.sig"

$(GCC).tar.gz.sig:
	wget "https://ftp.gnu.org/gnu/gcc/$(GCC)/$(GCC).tar.gz.sig" -O "$(GCC).tar.gz.sig"

$(BINUTILS).tar.gz: $(BINUTILS).tar.gz.sig
	wget "https://ftp.gnu.org/gnu/binutils/$(BINUTILS).tar.gz" -O "$(BINUTILS).tar.gz.dat"
	gpgv --keyring ./gnu-keyring.gpg "$(BINUTILS).tar.gz.sig" "$(BINUTILS).tar.gz.dat"
	mv "$(BINUTILS).tar.gz.dat" "$(BINUTILS).tar.gz"
	@# update modification time, in case the signature file dep is newer
	touch "$(BINUTILS).tar.gz"

$(GCC).tar.gz: $(GCC).tar.gz.sig
	wget "https://ftp.gnu.org/gnu/gcc/$(GCC)/$(GCC).tar.gz" -O "$(GCC).tar.gz.dat"
	gpgv --keyring ./gnu-keyring.gpg "$(GCC).tar.gz.sig" "$(GCC).tar.gz.dat"
	mv "$(GCC).tar.gz.dat" "$(GCC).tar.gz"
	touch "$(GCC).tar.gz"

$(BINUTILS): $(BINUTILS).tar.gz
	tar xzf $(BINUTILS).tar.gz
	cd $(BINUTILS); patch -p1 < ../$(BINUTILS).patch

$(GCC): $(GCC).tar.gz
	tar xzf $(GCC).tar.gz
	cd $(GCC); patch -p1 < ../$(GCC).patch

bin/$(TARGET)-ld: $(BINUTILS)
	mkdir -p binbuild-$(TARGET)
	cd binbuild-$(TARGET); \
		../$(BINUTILS)/configure \
			--target=$(TARGET) \
			--prefix=$(PREFIX) \
			--with-sysroot=$(SYSROOT) \
			--disable-werror \
			--disable-nls \
			; \
		$(MAKE) $(MAKEARGS); $(MAKE) install;

bin/$(TARGET)-gcc: $(GCC) bin/$(TARGET)-ld
	mkdir -p gccbuild-$(TARGET)
	cd gccbuild-$(TARGET); \
		export PATH="$$PATH:$(PREFIX)";\
		../$(GCC)/configure \
			--target=$(TARGET) \
			--prefix=$(PREFIX) \
			--with-sysroot=$(SYSROOT) \
			--disable-nls \
			--enable-languages=c,c++ \
			; \
		$(MAKE) $(MAKEARGS) all-gcc all-target-libgcc; \
		$(MAKE) install-gcc install-target-libgcc;

clean-build:
	-for dir in $(CLEAN_DIRS); do cd $$dir && make clean && cd ..; done
	-rm -rf $(CLEAN_DIRS)

clean-source:
	-rm -rf $(CLEAN_SOURCE)

clean-archives:
	@# don't clear signature files
	-rm -f *.tar.{gz,xz}
	-rm -f *.dat

clean-bin:
	-rm -rf ./bin ./$(TARGET) ./include ./lib ./share ./libexec

clean-all: clean-build clean-archives clean-bin clean-source
