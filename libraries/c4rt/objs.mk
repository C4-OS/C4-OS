c4rt-src  = $(wildcard $(LIBRARY_ROOT)/c4rt/src/*.c)
c4rt-src += $(wildcard $(LIBRARY_ROOT)/c4rt/src/stublibc/*.c)
c4rt-obj  = $(c4rt-src:.c=.o)

$(BUILD)/libs/c4rt.a: $(c4rt-obj)
	ar rvs $@ $^

.PHONY: c4rt-clean
c4rt-clean:
	rm -f $(c4rt-obj)

ALL_CLEAN += c4rt-clean
