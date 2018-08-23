libc-src  = $(wildcard $(LIBRARY_ROOT)/libc/src/*.c)
libc-obj  = $(libc-src:.c=.o)
libc-deps = $(BUILD)/libs/c4rt.a

$(BUILD)/libs/libc.a: $(libc-obj) $(libc-deps)
	ar rvs $@ $(libc-obj)

.PHONY: libc-clean
libc-clean:
	rm -f $(libc-obj)

ALL_CLEAN += libc-clean
