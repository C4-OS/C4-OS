MAKE=make
PREFIX=$(shell pwd)
SYSROOT=CHANGEME
TARGET=i686-elf-c4os
BINUTILS=binutils-2.28
GCC=gcc-6.4.0

CLEAN_DIRS=binbuild-$(TARGET) gccbuild-$(TARGET)
CLEAN_SOURCE=$(BINUTILS) $(GCC)

all: bin/$(TARGET)-ld bin/$(TARGET)-gcc

$(BINUTILS).tar.gz:
	wget "http://ftp.gnu.org/gnu/binutils/$(BINUTILS).tar.gz"

$(GCC).tar.gz:
	wget "http://ftp.gnu.org/gnu/gcc/$(GCC)/$(GCC).tar.gz"

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
	-rm -rf *.tar.*

clean-bin:
	-rm -rf ./bin ./$(TARGET) ./include ./lib ./share ./libexec

clean-all: clean-build clean-archives clean-bin clean-source

.PHONY: all
