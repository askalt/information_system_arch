
DEPS = app.h event.h entities.h game_ui.h main.cpp panic.h state.h system.h static_objs.h consts.h chars.h
BIN := bin
WORLD_PATH = ./world

all: build

bin:
	mkdir -p bin

build: $(DEPS) bin
	$(CXX) main.cpp $(FLAGS) -lncurses -o ./$(BIN)/app

build_pdcurses: $(DEPS) bin
	$(CXX) main.cpp -L$(PDCURSESPATH) $(FLAGS) -lSDL2 -l:pdcurses.so -o ./$(BIN)/app_pdcurses

run: build
	./$(BIN)/app $(WORLD_PATH)

run_pdcurses: build_pdcurses
	LD_LIBRARY_PATH=$(PDCURSESPATH):$(LD_LIBRARY_PATH) ./$(BIN)/app_pdcurses $(WORLD_PATH)

clean:
	rm -rf ./$(BIN)
