cross     =
buildroot = /not/a/path
version   = 5.3.5

.PHONY: all
all: build install

clean:
	rm -rf lua-$(version)

build: lua-$(version)
	cd lua-$(version); \
		export CC=$(cross)gcc; \
		make CC=$(cross)gcc LD=$(cross)ld INSTALL_TOP="$(buildroot)" generic

install:
	cd lua-$(version); \
		INSTALL_TOP="$(buildroot)" make c89 install
	touch $(buildroot)/ports/lua

.PHONY: checksum
checksum: CHECKSUMS
	sha256sum -c CHECKSUMS

lua-$(version): lua-$(version).tar.gz checksum
	tar xzf $<

lua-$(version).tar.gz:
	wget "https://www.lua.org/ftp/lua-$(version).tar.gz"
