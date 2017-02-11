skeleton-prog-libs = $(BUILD)/libs/c4rt.a
skeleton-prog-src  = $(wildcard $(PROGRAM_ROOT)/skeleton-prog/src/*.c)
skeleton-prog-obj  = $(skeleton-prog-src:.c=.o)

$(BUILD)/bin/skeleton-prog: $(skeleton-prog-libs) $(skeleton-prog-obj)
	@echo CC $^ -o $@
	@$(C4_CC) $(C4_CFLAGS) $^ -o $@ $(skeleton-prog-libs)

.PHONY: skeleton-prog-clean
skeleton-prog-clean:
	rm -f $(BUILD)/bin/skeleton-prog
	rm -f $(skeleton-prog-obj)

ALL_PROGRAMS += $(BUILD)/bin/skeleton-prog
ALL_CLEAN    += skeleton-prog-clean
