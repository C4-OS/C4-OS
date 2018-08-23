ata-libs =
ata-src  = $(wildcard $(PROGRAM_ROOT)/ata/src/*.c)
ata-obj  = $(ata-src:.c=.o)

$(BUILD)/bin/ata: $(ata-libs) $(ata-obj)
	@echo CC $^ -o $@
	@$(C4_CC) $^ -o $@ $(ata-libs) $(C4_CFLAGS)

.PHONY: ata-clean
ata-clean:
	rm -f $(BUILD)/bin/ata
	rm -f $(ata-obj)

ALL_PROGRAMS += $(BUILD)/bin/ata
ALL_CLEAN    += ata-clean
