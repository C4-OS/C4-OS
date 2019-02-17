linux-compat-src  = $(wildcard $(LIBRARY_ROOT)/linux-compat/src/*.c)
linux-compat-obj  = $(linux-compat-src:.c=.o)
linux-compat-deps = $(BUILD)/lib/libc.a

$(BUILD)/lib/linux-compat.a: $(linux-compat-obj) $(linux-compat-deps)
	ar rvs $@ $(linux-compat-obj)

.PHONY: linux-compat-clean
linux-compat-clean:
	rm -f $(linux-compat-obj)

ALL_CLEAN += linux-compat-clean
