nameserver-libs =
nameserver-src  = $(wildcard $(PROGRAM_ROOT)/nameserver/src/*.c)
nameserver-obj  = $(nameserver-src:.c=.o)

$(BUILD)/bin/nameserver: $(nameserver-libs) $(nameserver-obj)
	@echo CC $^ -o $@
	@$(C4_CC) $^ -o $@ $(nameserver-libs) $(C4_CFLAGS) 

.PHONY: nameserver-clean
nameserver-clean:
	rm -f $(BUILD)/bin/nameserver
	rm -f $(nameserver-obj)

ALL_PROGRAMS += $(BUILD)/bin/nameserver
ALL_CLEAN    += nameserver-clean
