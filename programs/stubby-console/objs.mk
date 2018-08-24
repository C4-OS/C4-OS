stubby-console-libs =
stubby-console-src  = $(wildcard $(PROGRAM_ROOT)/stubby-console/src/*.c)
stubby-console-obj  = $(stubby-console-src:.c=.o)

$(BUILD)/bin/stubby-console: $(stubby-console-libs) $(stubby-console-obj)
	@echo CC $^ -o $@
	@$(C4_CC) $^ -o $@ $(stubby-console-libs) $(C4_CFLAGS) 

.PHONY: stubby-console-clean
stubby-console-clean:
	rm -f $(BUILD)/bin/stubby-console
	rm -f $(stubby-console-obj)

ALL_PROGRAMS += $(BUILD)/bin/stubby-console
ALL_CLEAN    += stubby-console-clean
