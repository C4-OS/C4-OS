c4rt-src  = $(wildcard $(LIBRARY_ROOT)/c4rt/src/*.c)
c4rt-src += $(wildcard $(LIBRARY_ROOT)/c4rt/libc/*.c)
c4rt-obj  = $(c4rt-src:.c=.o)

c4rt-crt-src = $(wildcard $(LIBRARY_ROOT)/c4rt/crt/*.c)
c4rt-crt-obj = $(c4rt-crt-src:.c=.o)

$(BUILD)/lib/libc.a: $(c4rt-obj)
	ar rvs $@ $^

$(BUILD)/lib/crt0.o: $(c4rt-crt-obj)
	cp $(LIBRARY_ROOT)/c4rt/crt/crt0.o $@

.PHONY: c4rt-clean
c4rt-clean:
	rm -f $(c4rt-obj) $(c4rt-crt-obj)

ALL_CLEAN += c4rt-clean
