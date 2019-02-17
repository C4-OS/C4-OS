cross     =
buildroot = /not/a/path
version   = 1.2.11

.PHONY: all
all: build install

clean:
	rm -rf zlib-$(version)

build: zlib-$(version)
	cd zlib-$(version); \
		CC=$(cross)gcc LD=$(cross)ld \
		./configure --prefix=$(buildroot) && \
		make

install:
	cd zlib-$(version); \
		make install prefix=$(buildroot)
	touch $(buildroot)/ports/lua

.PHONY: checksum
checksum: CHECKSUMS
	sha256sum -c CHECKSUMS

zlib-$(version): zlib-$(version).tar.gz checksum
	tar xzf $<

zlib-$(version).tar.gz:
	wget "https://www.zlib.net/zlib-1.2.11.tar.gz"
