forth-libs  = $(BUILD)/lib/miniforth.a
forth-src   = $(wildcard $(PROGRAM_ROOT)/forth/src/*.c)
forth-obj   = $(forth-src:.c=.o)
forth-files = $(wildcard $(PROGRAM_ROOT)/forth/tree/*)
forth-tree  = $(subst $(PROGRAM_ROOT)/forth/tree,$(BUILD)/tree,$(forth-files))

$(BUILD)/bin/forth: $(forth-obj) $(forth-libs) $(forth-tree)
	@echo CC $^ -o $@
	@$(C4_CC) $(forth-obj) -o $@ $(forth-libs) $(C4_CFLAGS) 

$(BUILD)/tree/%: $(PROGRAM_ROOT)/forth/tree/%
	cp -r $< $@

.PHONY: forth-clean
forth-clean:
	rm -f $(BUILD)/bin/forth
	rm -f $(forth-obj)

ALL_PROGRAMS += $(BUILD)/bin/forth
ALL_CLEAN    += forth-clean
