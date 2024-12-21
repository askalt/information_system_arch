
DEPS = app.h event.h entities.h game_ui.h main.cpp panic.h state.h system.h static_objs.h

all: build

bin:
	mkdir -p bin

build_pdcurses: $(DEPS) bin
	$(CXX) main.cpp -L$(PDCURSESPATH) $(FLAGS) -lSDL2 -l:pdcurses.so -o ./bin/app

build: $(DEPS) bin
	$(CXX) main.cpp $(FLAGS) -lncurses -o ./bin/app

run: build
	LD_LIBRARY_PATH=$(PDCURSESPATH):$(LD_LIBRARY_PATH) ./bin/app
