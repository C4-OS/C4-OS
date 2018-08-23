skeleton-src  = $(wildcard $(LIBRARY_ROOT)/skeleton/src/*.c)
skeleton-obj  = $(skeleton-src:.c=.o)
skeleton-deps = $(BUILD)/lib/c4rt.a

$(BUILD)/lib/skeleton.a: $(skeleton-obj) $(skeleton-deps)
	ar rvs $@ $(skeleton-obj)

.PHONY: skeleton-clean
skeleton-clean:
	rm -f $(skeleton-obj)

ALL_CLEAN += skeleton-clean
