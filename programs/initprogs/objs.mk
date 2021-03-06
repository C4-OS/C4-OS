initprog-libs = $(BUILD)/lib/miniforth.a


$(BUILD)/bin/keyboard: $(PROGRAM_ROOT)/initprogs/src/keyboard.o $(initprog-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $^ -o $@ $(initprog-libs) $(C4_CFLAGS) 

$(BUILD)/bin/ps2mouse: $(PROGRAM_ROOT)/initprogs/src/ps2mouse.o $(initprog-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $^ -o $@ $(initprog-libs) $(C4_CFLAGS) 

$(BUILD)/bin/test: $(PROGRAM_ROOT)/initprogs/src/test.o $(initprog-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $^ -o $@ $(initprog-libs) $(C4_CFLAGS) 

$(BUILD)/bin/pci: $(PROGRAM_ROOT)/initprogs/src/pci.o $(initprog-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $^ -o $@ $(initprog-libs) $(C4_CFLAGS) 

$(BUILD)/bin/faulter: $(PROGRAM_ROOT)/initprogs/src/faulter.o $(initprog-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $^ -o $@ $(initprog-libs) $(C4_CFLAGS) 

$(BUILD)/bin/ipcbench: $(PROGRAM_ROOT)/initprogs/src/ipcbench.o $(initprog-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $^ -o $@ $(initprog-libs) $(C4_CFLAGS) 

$(BUILD)/bin/alloctest: $(PROGRAM_ROOT)/initprogs/src/alloctest.o $(initprog-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $^ -o $@ $(initprog-libs) $(C4_CFLAGS) 

$(BUILD)/bin/atatest: $(PROGRAM_ROOT)/initprogs/src/atatest.o $(initprog-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $^ -o $@ $(initprog-libs) $(C4_CFLAGS) 

$(BUILD)/bin/clibtest: $(PROGRAM_ROOT)/initprogs/src/clibtest.o $(initprog-libs)
	@echo CC $^ -o $@
	@$(C4_CC) $^ -o $@ $(initprog-libs) $(C4_CFLAGS) 


.PHONY: initprogs-keyboard-clean
initprogs-keyboard-clean:
	rm -f $(BUILD)/bin/keyboard
	rm -f $(PROGRAM_ROOT)/initprogs/src/keyboard.o

.PHONY: initprogs-ps2mouse-clean
initprogs-ps2mouse-clean:
	rm -f $(BUILD)/bin/ps2mouse
	rm -f $(PROGRAM_ROOT)/initprogs/src/ps2mouse.o

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

.PHONY: initprogs-ipcbench-clean
initprogs-ipcbench-clean:
	rm -f $(BUILD)/bin/ipcbench
	rm -f $(PROGRAM_ROOT)/initprogs/src/ipcbench.o

.PHONY: initprogs-alloctest-clean
initprogs-alloctest-clean:
	rm -f $(BUILD)/bin/alloctest
	rm -f $(PROGRAM_ROOT)/initprogs/src/alloctest.o

.PHONY: initprogs-atatest-clean
initprogs-atatest-clean:
	rm -f $(BUILD)/bin/atatest
	rm -f $(PROGRAM_ROOT)/initprogs/src/atatest.o

.PHONY: initprogs-clibtest-clean
initprogs-clibtest-clean:
	rm -f $(BUILD)/bin/clibtest
	rm -f $(PROGRAM_ROOT)/initprogs/src/clibtest.o


ALL_PROGRAMS += $(BUILD)/bin/keyboard
ALL_PROGRAMS += $(BUILD)/bin/ps2mouse
ALL_PROGRAMS += $(BUILD)/bin/test
ALL_PROGRAMS += $(BUILD)/bin/pci
ALL_PROGRAMS += $(BUILD)/bin/faulter
ALL_PROGRAMS += $(BUILD)/bin/ipcbench
ALL_PROGRAMS += $(BUILD)/bin/alloctest
ALL_PROGRAMS += $(BUILD)/bin/atatest
ALL_PROGRAMS += $(BUILD)/bin/clibtest
ALL_CLEAN    += initprogs-keyboard-clean
ALL_CLEAN    += initprogs-ps2mouse-clean
ALL_CLEAN    += initprogs-test-clean
ALL_CLEAN    += initprogs-pci-clean
ALL_CLEAN    += initprogs-faulter-clean
ALL_CLEAN    += initprogs-ipcbench-clean
ALL_CLEAN    += initprogs-alloctest-clean
ALL_CLEAN    += initprogs-atatest-clean
ALL_CLEAN    += initprogs-clibtest-clean
