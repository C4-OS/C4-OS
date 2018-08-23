display-libs =
display-src  = $(wildcard $(PROGRAM_ROOT)/display/src/*.c)
display-obj  = $(display-src:.c=.o)

$(BUILD)/bin/display: $(display-libs) $(display-obj)
	@echo CC $^ -o $@
	@$(C4_CC) $^ -o $@ $(display-libs) $(C4_CFLAGS) 

.PHONY: display-clean
display-clean:
	rm -f $(BUILD)/bin/display
	rm -f $(display-obj)

ALL_PROGRAMS += $(BUILD)/bin/display
ALL_CLEAN    += display-clean
