ALL_PROGRAMS  =
ALL_LIBRARIES =

include buildconf.mk

.PHONY: do_all
do_all: all

# include recipes provided by each program
-include $(wildcard $(PROGRAM_ROOT)/*/objs.mk)
# and for each library
-include $(wildcard $(LIBRARY_ROOT)/*/objs.mk)
# and for sigma0 (this should be moved to the programs folder)
-include sigma0/objs.mk

ALL_TARGETS  += kernel sigma0 $(ALL_PROGRAMS)
#ALL_CLEAN    += kernel-clean sigma0-clean
ALL_CLEAN    += kernel-clean

.PHONY: all
all: $(ALL_TARGETS)

$(BUILD):
	mkdir -p $(BUILD)
	mkdir -p $(BUILD)/bin
	mkdir -p $(BUILD)/src
	mkdir -p $(BUILD)/libs

$(BUILD)/c4-$(ARCH): $(BUILD)
	@cd kernel; \
		make CROSS=$(CROSS) ARCH=$(ARCH); \
		cp c4-$(ARCH) $@

$(BUILD)/initfs: $(BUILD) $(INITFS_PROGRAMS)
	mkdir -p $(BUILD)/initfs/bin
	cp $(INITFS_PROGRAMS) $(BUILD)/initfs/bin

$(BUILD)/initfs.tar: $(BUILD)/initfs
	cd $(BUILD)/initfs; tar c ./bin > $@

.PHONY: kernel
kernel: $(BUILD)/c4-$(ARCH)

.PHONY: kernel-clean
kernel-clean:
	cd kernel; make clean ARCH=$(ARCH)
	rm -f $(BUILD)/c4-$(ARCH)

.PHONY: sigma0
sigma0: $(BUILD)/c4-$(ARCH)-sigma0

.PHONY: clean
clean: $(ALL_CLEAN)
	rm -rf $(BUILD)

.PHONY: test
test:
	qemu-system-i386 \
		-kernel $(BUILD)/c4-$(ARCH) \
		-initrd $(BUILD)/c4-$(ARCH)-sigma0 \
		-serial stdio -m 32 -s
