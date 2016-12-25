initprog-libs = $(BUILD)/libs/c4rt.a

$(BUILD)/bin/keyboard: $(PROGRAM_ROOT)/initprogs/src/keyboard.o $(initprog-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $(C4_CFLAGS) $^ -o $@ $(initprog-libs)

$(BUILD)/bin/test: $(PROGRAM_ROOT)/initprogs/src/test.o $(initprog-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $(C4_CFLAGS) $^ -o $@ $(initprog-libs)

.PHONY: initprogs-keyboard-clean
initprogs-keyboard-clean:
	rm -f $(BUILD)/bin/keyboard
	rm -f $(PROGRAM_ROOT)/initprogs/src/keyboard.o

.PHONY: initprogs-test-clean
initprogs-test-clean:
	rm -f $(BUILD)/bin/test
	rm -f $(PROGRAM_ROOT)/initprogs/src/test.o

ALL_PROGRAMS += $(BUILD)/bin/keyboard
ALL_PROGRAMS += $(BUILD)/bin/test
ALL_CLEAN    += initprogs-keyboard-clean
ALL_CLEAN    += initprogs-test-clean
