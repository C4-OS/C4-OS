CROSS        = $(PWD)/cross/bin/i586-elf-
ARCH         = x86
BUILD        = $(PWD)/.buildtmp
KERNEL_ROOT  = $(PWD)/kernel
PROGRAM_ROOT = $(PWD)/programs
LIBRARY_ROOT = $(PWD)/libraries
TOOL_ROOT    = $(PWD)/tools

KERNEL_INCLUDE    = -I$(KERNEL_ROOT)/include/ \
                    -I$(KERNEL_ROOT)/arch/$(ARCH)/include/
LIBRARY_INCLUDE   = $(patsubst %,-I%, $(wildcard $(LIBRARY_ROOT)/*/include/))
PROGRAM_INCLUDE   = $(patsubst %,-I%, $(wildcard $(PROGRAM_ROOT)/*/include/))
INTERFACE_INCLUDE = -I$(PWD)

C4_CC     = $(CROSS)gcc
C4_LD     = $(CROSS)ld
C4_CXX    = $(CROSS)g++
C4_CFLAGS = -Wall -g -O2 -ffreestanding -nostdlib -nodefaultlibs \
            -nostartfiles -fno-builtin \
            $(KERNEL_INCLUDE) $(LIBRARY_INCLUDE) $(PROGRAM_INCLUDE) \
			$(INTERFACE_INCLUDE)

%.o: %.c
	@echo CC $< -c -o $@
	@$(C4_CC) $(C4_CFLAGS) $< -c -o $@

INITFS_PROGRAMS  = $(BUILD)/bin/nameserver
INITFS_PROGRAMS += $(BUILD)/bin/ata
INITFS_PROGRAMS += $(BUILD)/bin/ext2fs
INITFS_PROGRAMS += $(BUILD)/bin/initsys

INITSYS_PROGRAMS += $(BUILD)/bin/keyboard
INITSYS_PROGRAMS += $(BUILD)/bin/display
INITSYS_PROGRAMS += $(BUILD)/bin/forth
