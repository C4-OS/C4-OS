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
-include $(PORTS_ROOT)/objs.mk

LIBC_STUFF    = $(BUILD)/lib/crt0.o $(BUILD)/lib/libc.a
ALL_TARGETS  += kernel $(LIBC_STUFF) sigma0 $(ALL_PROGRAMS) $(PORTS)
ALL_CLEAN    += kernel-clean ports-clean
ALL_INCLUDES  = $(patsubst -I%,%,$(KERNEL_INCLUDE) $(LIBRARY_INCLUDE) $(PROGRAM_INCLUDE))

.PHONY: all
all: $(ALL_TARGETS)

$(BUILD)/usr/include: $(ALL_INCLUDES)
	mkdir -p $(BUILD)/usr
	cp -rv $(ALL_INCLUDES) $(BUILD)/usr

$(BUILD): $(BUILD)/usr/include
	mkdir -p $(BUILD)
	mkdir -p $(BUILD)/bin
	mkdir -p $(BUILD)/src
	mkdir -p $(BUILD)/lib
	mkdir -p $(BUILD)/tree
	mkdir -p $(BUILD)/ports

$(BUILD)/c4-$(ARCH): $(BUILD)
	@cd kernel; \
		make CROSS=$(CROSS) ARCH=$(ARCH); \
		cp c4-$(ARCH) $@

$(BUILD)/initfs: $(BUILD) $(INITFS_PROGRAMS)
	mkdir -p $(BUILD)/initfs/bin
	cp $(INITFS_PROGRAMS) $(BUILD)/initfs/bin

$(BUILD)/initfs.tar: $(BUILD)/initfs
	cd $(BUILD)/initfs; tar c ./bin > $@

$(BUILD)/include/usr/include: $(BUILD)
	mkdir -p "$@"
	for dir in $(ALL_INCLUDES); do \
		cp -RT "$$dir" "$@"; \
	done;

$(BUILD)/test.img: $(ALL_TARGETS)
	sudo $(TOOL_ROOT)/buildimg-$(ARCH) $@ \
		$(TOOL_ROOT)/bootconf-$(ARCH) \
		$(BUILD)/c4-$(ARCH) \
		$(BUILD)/c4-$(ARCH)-sigma0 \
		$(BUILD) \
		$(INITSYS_PROGRAMS)

.PHONY: toolchain
toolchain: sysroot $(BUILD)
	cd cross; make SYSROOT=$(BUILD) MAKEARGS=-j4

.PHONY: kernel
kernel: sysroot $(BUILD)/c4-$(ARCH)

.PHONY: kernel-clean
kernel-clean:
	cd kernel; make clean ARCH=$(ARCH)
	rm -f $(BUILD)/c4-$(ARCH)

.PHONY: sigma0
sigma0: sysroot $(BUILD)/c4-$(ARCH)-sigma0

.PHONY: libc
libc: sysroot $(LIBC_STUFF)

.PHONY: ports
ports: $(PORTS)

.PHONY: initfs-clean
initfs-clean:
	rm -rf $(BUILD)/initfs
	rm -f  $(BUILD)/initfs.tar

.PHONY: sysroot
sysroot: $(BUILD)

.PHONY: sysincludes
sysincludes: sysroot $(BUILD)/include/usr/include

.PHONY: clean-sysroot
clean-sysroot:
	rm -rf $(BUILD)

.PHONY: clean
clean: $(ALL_CLEAN)
	rm -rf $(BUILD)

.PHONY: test
test: image
	qemu-system-i386 \
		-hda $(BUILD)/test.img \
		-serial stdio -m 32 -s -enable-kvm -smp 4 -cpu max,migratable=false

.PHONY: image
image: $(BUILD)/test.img
