CROSS = $(PWD)/cross/bin/i586-elf-
ARCH  = x86
BUILD = $(PWD)/.buildtmp

.PHONY: all
all: kernel sigma0

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/c4-$(ARCH): $(BUILD)
	cd kernel; \
		make CROSS=$(CROSS) ARCH=$(ARCH); \
		cp c4-$(ARCH) $@

$(BUILD)/initfs.tar: $(BUILD)
	cp initfs.tar $@

$(BUILD)/c4-$(ARCH)-sigma0: $(BUILD)/initfs.tar 
	cd sigma0; \
		make CROSS=$(CROSS) ARCH=$(ARCH) CROSS=$(CROSS) \
			 KERNEL_ROOT=$(PWD)/kernel INITFS_TARBALL=$(BUILD)/initfs.tar; \
		cp c4-$(ARCH)-sigma0 $@

.PHONY: kernel
kernel: $(BUILD)/c4-$(ARCH)

.PHONY: sigma0
sigma0: $(BUILD)/c4-$(ARCH)-sigma0

.PHONY: clean
clean:
	rm -rf $(BUILD)
	cd sigma0; make clean ARCH=$(ARCH)
	cd kernel; make clean ARCH=$(ARCH)

.PHONY: test
test:
	qemu-system-i386 \
		-kernel $(BUILD)/c4-$(ARCH) \
		-initrd $(BUILD)/c4-$(ARCH)-sigma0 \
		-serial stdio -m 32
