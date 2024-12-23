
PROJS := cli

.PHONY: $(PROJS)-test
$(PROJS)-test:
	make -C $(patsubst %-test, %, $@) test

test: $(PROJS)-test
