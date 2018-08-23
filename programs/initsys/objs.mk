initsys-libs =
initsys-src  = $(wildcard $(PROGRAM_ROOT)/initsys/src/*.c)
initsys-obj  = $(initsys-src:.c=.o)

$(BUILD)/bin/initsys: $(initsys-libs) $(initsys-obj)
	@echo CC $^ -o $@
	@$(C4_CC) $^ -o $@ $(initsys-libs) $(C4_CFLAGS) 

.PHONY: initsys-clean
initsys-clean:
	rm -f $(BUILD)/bin/initsys
	rm -f $(initsys-obj)

ALL_PROGRAMS += $(BUILD)/bin/initsys
ALL_CLEAN    += initsys-clean
