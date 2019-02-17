cross     =
buildroot = /not/a/path
version   = 2.9.1

.PHONY: all
all: build install

clean:
	rm -rf freetype-$(version)

build: freetype-$(version)
	cd freetype-$(version); \
		export PATH="$(dir $(cross)):$$PATH"; \
		echo $$PATH; \
		./configure \
			--prefix=$(buildroot)/usr \
			--exec-prefix=$(buildroot)/usr \
			--host i686-elf-c4os; \
		make

install:
	cd freetype-$(version); \
		make install

.PHONY: checksum
checksum: CHECKSUMS
	sha256sum -c CHECKSUMS

freetype-$(version): freetype-$(version).tar.gz checksum
	tar xzf $<

freetype-$(version).tar.gz:
	wget "https://download.savannah.gnu.org/releases/freetype/$@"
