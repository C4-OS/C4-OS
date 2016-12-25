CROSS        = $(PWD)/cross/bin/i586-elf-
ARCH         = x86
BUILD        = $(PWD)/.buildtmp
KERNEL_ROOT  = $(PWD)/kernel
PROGRAM_ROOT = $(PWD)/programs
LIBRARY_ROOT = $(PWD)/libraries

KERNEL_INCLUDE = -I$(KERNEL_ROOT)/include/ \
				 -I$(KERNEL_ROOT)/arch/$(ARCH)/include/

C4_CC     = $(CROSS)gcc
C4_CXX    = $(CROSS)g++
C4_CFLAGS = -Wall -g -O2 -ffreestanding -nostdlib -nodefaultlibs \
			-nostartfiles -fno-builtin $(KERNEL_INCLUDE)

%.o: %.c
	@echo CC $< -c -o $@
	@$(C4_CC) $(C4_CFLAGS) $< -c -o $@
