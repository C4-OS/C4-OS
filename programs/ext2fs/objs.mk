ext2fs-libs = $(BUILD)/libs/c4rt.a
ext2fs-src  = $(wildcard $(PROGRAM_ROOT)/ext2fs/src/*.c)
ext2fs-obj  = $(ext2fs-src:.c=.o)

$(BUILD)/bin/ext2fs: $(ext2fs-libs) $(ext2fs-obj)
	@echo CC $^ -o $@
	@$(C4_CC) $(C4_CFLAGS) $^ -o $@ $(ext2fs-libs)

.PHONY: ext2fs-clean
ext2fs-clean:
	rm -f $(BUILD)/bin/ext2fs
	rm -f $(ext2fs-obj)

ALL_PROGRAMS += $(BUILD)/bin/ext2fs
ALL_CLEAN    += ext2fs-clean
