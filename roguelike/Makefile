
DEPS = app.h event.h entities.h game_ui.h main.cpp panic.h state.h system.h static_objs.h
BIN := bin

all: build

bin:
	mkdir -p bin

build_pdcurses: $(DEPS) bin
	$(CXX) main.cpp -L$(PDCURSESPATH) $(FLAGS) -lSDL2 -l:pdcurses.so -o ./$(BIN)/app

generate_zero_map: build_map_codegen
	make INPUT=zero_map.txt OUTPUT=zero_map.h gen_map

build: $(DEPS) bin generate_zero_map
	$(CXX) main.cpp $(FLAGS) -lncurses -o ./$(BIN)/app

build_map_codegen: bin map_codegen.cpp
	$(CXX) map_codegen.cpp -o ./$(BIN)/map_codegen

run: build
	LD_LIBRARY_PATH=$(PDCURSESPATH):$(LD_LIBRARY_PATH) ./$(BIN)/app

gen_map: build_map_codegen
	./$(BIN)/map_codegen < $(INPUT) > $(OUTPUT)

clean:
	rm -rf ./$(BIN)
