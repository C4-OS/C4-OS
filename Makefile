include buildconf.mk

.PHONY: do_all
do_all: all

ALL_PROGRAMS  = 
ALL_LIBRARIES = 

# include recipes provided by each program
-include $(wildcard $(PROGRAM_ROOT)/*/objs.mk)
# and for each libraries
-include $(wildcard $(LIBRARY_ROOT)/*/objs.mk)

ALL_TARGETS   = kernel sigma0 $(ALL_PROGRAMS) $(ALL_LIBRARIES)
ALL_CLEAN     = kernel-clean sigma0-clean

.PHONY: all
all: $(ALL_TARGETS)

$(BUILD):
	mkdir -p $(BUILD)
	mkdir -p $(BUILD)/bin
	mkdir -p $(BUILD)/src

$(BUILD)/c4-$(ARCH): $(BUILD)
	@cd kernel; \
		make CROSS=$(CROSS) ARCH=$(ARCH); \
		cp c4-$(ARCH) $@

$(BUILD)/initfs.tar: $(BUILD) $(ALL_PROGRAMS)
	cd $(BUILD); tar c . > $@

$(BUILD)/c4-$(ARCH)-sigma0: $(BUILD)/initfs.tar
	@cd sigma0; \
		make CROSS=$(CROSS) ARCH=$(ARCH) CROSS=$(CROSS) \
			 KERNEL_ROOT=$(PWD)/kernel INITFS_TARBALL=$(BUILD)/initfs.tar; \
		cp c4-$(ARCH)-sigma0 $@

.PHONY: kernel
kernel: $(BUILD)/c4-$(ARCH)

.PHONY: kernel-clean
kernel-clean:
	cd kernel; make clean ARCH=$(ARCH)

.PHONY: sigma0
sigma0: $(BUILD)/c4-$(ARCH)-sigma0

.PHONY: sigma0-clean
sigma0-clean:
	cd sigma0; make clean ARCH=$(ARCH)

.PHONY: clean
clean: $(ALL_CLEAN)
	rm -rf $(BUILD)

.PHONY: test
test:
	qemu-system-i386 \
		-kernel $(BUILD)/c4-$(ARCH) \
		-initrd $(BUILD)/c4-$(ARCH)-sigma0 \
		-serial stdio -m 32
