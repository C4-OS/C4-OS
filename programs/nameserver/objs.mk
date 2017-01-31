nameserver-libs = $(BUILD)/libs/c4rt.a
nameserver-src  = $(wildcard $(PROGRAM_ROOT)/nameserver/src/*.c)
nameserver-obj  = $(nameserver-src:.c=.o)

$(BUILD)/bin/nameserver: $(nameserver-libs) $(nameserver-obj)
	@echo CC $^ -o $@
	@$(C4_CC) $(C4_CFLAGS) $^ -o $@ $(nameserver-libs)

.PHONY: nameserver-clean
nameserver-clean:
	rm -f $(BUILD)/bin/nameserver
	rm -f $(nameserver-obj)

ALL_PROGRAMS += $(BUILD)/bin/nameserver
ALL_CLEAN    += nameserver-clean
