ata-libs = $(BUILD)/libs/c4rt.a
ata-src  = $(wildcard $(PROGRAM_ROOT)/ata/src/*.c)
ata-obj  = $(ata-src:.c=.o)

$(BUILD)/bin/ata: $(ata-libs) $(ata-obj)
	@echo CC $^ -o $@
	@$(C4_CC) $(C4_CFLAGS) $^ -o $@ $(ata-libs)

.PHONY: ata-clean
ata-clean:
	rm -f $(BUILD)/bin/ata
	rm -f $(ata-obj)

ALL_PROGRAMS += $(BUILD)/bin/ata
ALL_CLEAN    += ata-clean
