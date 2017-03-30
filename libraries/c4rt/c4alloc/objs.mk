c4alloc-src  = $(wildcard $(LIBRARY_ROOT)/c4alloc/src/*.c)
c4alloc-obj  = $(c4alloc-src:.c=.o)
c4alloc-deps = $(BUILD)/libs/c4rt.a

$(BUILD)/libs/c4alloc.a: $(c4alloc-obj) $(c4alloc-deps)
	ar rvs $@ $(c4alloc-obj)

.PHONY: c4alloc-clean
c4alloc-clean:
	rm -f $(c4alloc-obj)

ALL_CLEAN += c4alloc-clean
