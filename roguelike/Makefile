
BIN      ?= bin
TEST_BIN ?= test_bin
TEST     ?= test
LIBS     ?= -lncurses

CPP           := main.cpp map.cpp objects.cpp state.cpp entities.cpp decision_tree.cpp
TEST_CPP      := $(wildcard $(TEST)/*.cpp)

TEST_STEMS    := $(TEST_CPP:.cpp=)
TEST_BINARIES := $(TEST_STEMS:$(TEST)/%=$(TEST_BIN)/%)
TEST_OBJS     := $(TEST_BINARIES:%=%.o)
TEST_DEPS     := $(TEST_OBJS:.o=.d)

# Default world path.
WORLD_PATH = ./world

CPPOBJ := $(addprefix $(BIN)/,$(CPP:.cpp=.o))
DEPS   := $(CPPOBJ:.o=.d)

all: build

$(CPPOBJ): $(BIN)/%.o : %.cpp
	@mkdir -p $(@D)
	$(CXX) $(FLAGS) $(LIBS) -c $< -o $@

$(DEPS): $(BIN)/%.d : %.cpp
	@mkdir -p $(@D)
	$(CXX) -E $(FLAGS) $(LIBS) $< -MM -MT $(@:.d=.o) > $@

.PHONY: build
build: $(CPPOBJ)
	$(CXX) $^ $(LIBS) $(FLAGS) -o $(BIN)/app

run: build
	./$(BIN)/app $(WORLD_PATH)

# .PHONY: build_pdcurses
# build_pdcurses:
# 	LIBS = -lSDL2 -l:pdcurses.so
# 	FLAGS += -L$(PDCURSESPATH)
# 	make build

# run_pdcurses:
# 	LD_LIBRARY_PATH=$(PDCURSESPATH):$(LD_LIBRARY_PATH) ./$(BIN)/app ($WORLD_PATH)

$(TEST_BINARIES) : $(TEST_BIN)/% : $(TEST)/%.cpp
	@mkdir -p $(@D)
	$(CXX) -I. $(FLAGS) $(LIBS) $< -o $@

$(TEST_DEPS) : $(TEST_BIN)/%.d : $(TEST)/%.cpp
	@mkdir -p $(@D)
	$(CXX) -I. -E $(FLAGS) $(LIBS) $< -MM -MT $(@:.d=) > $@

.PHONY: $(TEST_BINARIES)/run
$(TEST_BINARIES)/run: %/run : %
	@echo "Running test: ${@:$(TEST_BIN)/%/run=%}"
	@if ./${@:/run=}; then echo ""; else echo "FAIL"; exit 1; fi

# targets which we have no need to recollect deps.
NODEPS = clean

.PHONY: test
test: $(TEST_BINARIES)/run

.PHONY: clean
clean:
	rm -rf ./$(BIN) ./$(TEST_BIN)

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))

ifneq (0, $(words $(findstring $(MAKECMDGOALS), test)))
include $(TEST_DEPS)
endif

include $(DEPS)

endif
