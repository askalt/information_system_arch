
DEPS = app.h event.h game_ui.h main.cpp panic.h state.h system.h

all: build

bin:
	mkdir -p bin

build: $(DEPS) bin
	$(CXX) main.cpp -L$(PDCURSESPATH) -lSDL2 -l:pdcurses.so -o ./bin/app

run: build
	LD_LIBRARY_PATH=$(PDCURSESPATH):$(LD_LIBRARY_PATH) ./bin/app
