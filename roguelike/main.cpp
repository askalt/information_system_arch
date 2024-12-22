#include <iostream>

#include "app.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "usage: ./app <WORLD_PATH>";
    exit(1);
  }
  auto world = std::make_unique<World>(std::filesystem::path{argv[1]});
  init_UI(argc, argv);
  auto app = App{std::move(world)};
  return app.run();
}
