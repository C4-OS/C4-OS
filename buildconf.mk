CROSS        = $(PWD)/cross/bin/i686-elf-c4os-
ARCH         = x86
BUILD        = $(PWD)/.buildtmp
KERNEL_ROOT  = $(PWD)/kernel
PROGRAM_ROOT = $(PWD)/programs
LIBRARY_ROOT = $(PWD)/libraries
PORTS_ROOT   = $(PWD)/ports
TOOL_ROOT    = $(PWD)/tools

KERNEL_INCLUDE    = -I$(KERNEL_ROOT)/include/ \
                    -I$(KERNEL_ROOT)/arch/$(ARCH)/include/
LIBRARY_INCLUDE   = $(patsubst %,-I%, $(wildcard $(LIBRARY_ROOT)/*/include/))
PROGRAM_INCLUDE   = $(patsubst %,-I%, $(wildcard $(PROGRAM_ROOT)/*/include/))
INTERFACE_INCLUDE = -I$(PWD)

C4_CC     = $(CROSS)gcc
C4_LD     = $(CROSS)ld
C4_CXX    = $(CROSS)g++
C4_CFLAGS = -Wall -Os

%.o: %.c
	@echo CC $< -c -o $@
	@$(C4_CC) $(C4_CFLAGS) $< -c -o $@

PORTS += $(BUILD)/ports/freetype
PORTS += $(BUILD)/ports/lua
PORTS += $(BUILD)/ports/zlib

INITFS_PROGRAMS  = $(BUILD)/bin/nameserver
INITFS_PROGRAMS += $(BUILD)/bin/ata
INITFS_PROGRAMS += $(BUILD)/bin/ext2fs
INITFS_PROGRAMS += $(BUILD)/bin/initsys

INITSYS_PROGRAMS += $(BUILD)/bin/keyboard
INITSYS_PROGRAMS += $(BUILD)/bin/ps2mouse
INITSYS_PROGRAMS += $(BUILD)/bin/display
#INITSYS_PROGRAMS += $(BUILD)/bin/forth
#INITSYS_PROGRAMS += $(BUILD)/bin/clibtest
INITSYS_PROGRAMS += $(BUILD)/bin/stubbywm
INITSYS_PROGRAMS += $(BUILD)/bin/stubby-console
