
BIN ?= bin

CPP   := composite.cpp decorator.cpp
STEMS := $(CPP:.cpp=)
BINS  := $(STEMS:%=$(BIN)/%)
OBJS  := $(BINS:%=%.o)
DEPS  := $(OBJS:.o=.d)
RUNS  := $(BINS:%=%/run)

all: build

$(BINS): $(BIN)/% : %.cpp
	@mkdir -p $(@D)
	$(CXX) $(FLAGS) $< -o $@

$(DEPS): $(BIN)/%.d : %.cpp
	@mkdir -p $(@D)
	$(CXX) -E $(FLAGS) $< -MM -MT $(@:.d=) > $@

.PHONY: $(RUNS)
$(RUNS): %/run: %
	@echo "Running: ${@:$(BIN)/%/run=%}"
	@if ./${@:/run=}; then echo ""; else echo "FAIL"; exit 1; fi

.PHONY: build
build: $(BINS)

.PHONY: run
run: $(RUNS)

.PHONY: clean
clean:
	rm -rf ./$(BIN)

# targets which we have no need to recollect deps.
NODEPS = clean

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
include $(DEPS)
endif
