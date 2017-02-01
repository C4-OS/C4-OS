initprog-libs = $(BUILD)/libs/c4rt.a $(BUILD)/libs/miniforth.a \
                $(BUILD)/libs/c4alloc.a

$(BUILD)/bin/keyboard: $(PROGRAM_ROOT)/initprogs/src/keyboard.o $(initprog-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $(C4_CFLAGS) $^ -o $@ $(initprog-libs)

$(BUILD)/bin/test: $(PROGRAM_ROOT)/initprogs/src/test.o $(initprog-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $(C4_CFLAGS) $^ -o $@ $(initprog-libs)

$(BUILD)/bin/pci: $(PROGRAM_ROOT)/initprogs/src/pci.o $(initprog-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $(C4_CFLAGS) $^ -o $@ $(initprog-libs)

$(BUILD)/bin/faulter: $(PROGRAM_ROOT)/initprogs/src/faulter.o $(initprog-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $(C4_CFLAGS) $^ -o $@ $(initprog-libs)

$(BUILD)/bin/forth: $(PROGRAM_ROOT)/initprogs/src/forth.o $(initprog-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $(C4_CFLAGS) $^ -o $@ $(initprog-libs)

$(BUILD)/bin/display: $(PROGRAM_ROOT)/initprogs/src/display.o $(initprog-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $(C4_CFLAGS) $^ -o $@ $(initprog-libs)

$(BUILD)/bin/ipcbench: $(PROGRAM_ROOT)/initprogs/src/ipcbench.o $(initprog-libs)
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

.PHONY: initprogs-pci-clean
initprogs-pci-clean:
	rm -f $(BUILD)/bin/pci
	rm -f $(PROGRAM_ROOT)/initprogs/src/pci.o

.PHONY: initprogs-faulter-clean
initprogs-faulter-clean:
	rm -f $(BUILD)/bin/faulter
	rm -f $(PROGRAM_ROOT)/initprogs/src/faulter.o

.PHONY: initprogs-forth-clean
initprogs-forth-clean:
	rm -f $(BUILD)/bin/forth
	rm -f $(PROGRAM_ROOT)/initprogs/src/forth.o

.PHONY: initprogs-display-clean
initprogs-display-clean:
	rm -f $(BUILD)/bin/display
	rm -f $(PROGRAM_ROOT)/initprogs/src/display.o

.PHONY: initprogs-ipcbench-clean
initprogs-ipcbench-clean:
	rm -f $(BUILD)/bin/ipcbench
	rm -f $(PROGRAM_ROOT)/initprogs/src/ipcbench.o

ALL_PROGRAMS += $(BUILD)/bin/keyboard
ALL_PROGRAMS += $(BUILD)/bin/test
ALL_PROGRAMS += $(BUILD)/bin/pci
ALL_PROGRAMS += $(BUILD)/bin/faulter
ALL_PROGRAMS += $(BUILD)/bin/forth
ALL_PROGRAMS += $(BUILD)/bin/display
ALL_PROGRAMS += $(BUILD)/bin/ipcbench
ALL_CLEAN    += initprogs-keyboard-clean
ALL_CLEAN    += initprogs-test-clean
ALL_CLEAN    += initprogs-pci-clean
ALL_CLEAN    += initprogs-faulter-clean
ALL_CLEAN    += initprogs-forth-clean
ALL_CLEAN    += initprogs-display-clean
ALL_CLEAN    += initprogs-ipcbench-clean
