stubbywm-libs =
stubbywm-src  = $(wildcard $(PROGRAM_ROOT)/stubbywm/src/*.c)
stubbywm-obj  = $(stubbywm-src:.c=.o)

$(BUILD)/bin/stubbywm: $(stubbywm-libs) $(stubbywm-obj)
	@echo CC $^ -o $@
	@$(C4_CC) $^ -o $@ $(stubbywm-libs) $(C4_CFLAGS) 

.PHONY: stubbywm-clean
stubbywm-clean:
	rm -f $(BUILD)/bin/stubbywm
	rm -f $(stubbywm-obj)

ALL_PROGRAMS += $(BUILD)/bin/stubbywm
ALL_CLEAN    += stubbywm-clean
