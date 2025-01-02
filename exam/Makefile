
BIN ?= bin

CPP   := composite.cpp
STEMS := $(CPP:.cpp=)
BINS  := $(STEMS:%=$(BIN)/%)
OBJS  := $(BINS:%=%.o)
DEPS  := $(OBJS:.o=.d)

all: build

$(BINS): $(BIN)/% : %.cpp
	@mkdir -p $(@D)
	$(CXX) $(FLAGS) $< -o $@

$(DEPS): $(BIN)/%.d : %.cpp
	@mkdir -p $(@D)
	$(CXX) -E $(FLAGS) $< -MM -MT $(@:.d=) > $@

build: $(BINS)

.PHONY: clean
clean:
	rm -rf ./$(BIN)

# targets which we have no need to recollect deps.
NODEPS = clean

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
include $(DEPS)
endif
