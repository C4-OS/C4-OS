forth-libs = $(BUILD)/libs/c4rt.a $(BUILD)/libs/miniforth.a \
	         $(BUILD)/libs/c4alloc.a
forth-src  = $(wildcard $(PROGRAM_ROOT)/forth/src/*.c)
forth-obj  = $(forth-src:.c=.o)

$(BUILD)/bin/forth: $(PROGRAM_ROOT)/forth/src/forth.o $(forth-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $(C4_CFLAGS) $^ -o $@ $(forth-libs)

.PHONY: forth-clean
forth-clean:
	rm -f $(BUILD)/bin/forth
	rm -f $(PROGRAM_ROOT)/forth/src/forth.o

ALL_PROGRAMS += $(BUILD)/bin/forth
ALL_CLEAN    += forth-clean
