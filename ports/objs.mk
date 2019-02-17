$(BUILD)/ports/%: libc
	cd $(PORTS_ROOT)/$(subst $(BUILD)/ports/,,$@); \
		make cross=$(CROSS) buildroot=$(BUILD); \
		cd ..

.PHONY: ports-clean
ports-clean:
	for thing in $(PORTS_ROOT)/*; do \
		cd "$$thing"; \
		make cross=$(CROSS) buildroot=$(BUILD) clean; \
		cd ..; \
	done
