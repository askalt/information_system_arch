
BIN ?= bin
CPP := main.cpp map.cpp objects.cpp state.cpp entities.cpp
LIBS ?= -lncurses

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

# targets which we have no need to recollect deps.
NODEPS = clean

.PHONY: clean
clean:
	rm -rf ./$(BIN)

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
include $(DEPS)
endif
