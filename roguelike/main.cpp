#include <iostream>

#include "app.h"

struct EndWinGuard {
  ~EndWinGuard() { deinit_UI(); }
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "usage: ./app <WORLD_PATH>";
    exit(1);
  }
  auto world = std::make_unique<World>(std::filesystem::path{argv[1]});
  init_UI(argc, argv);
  auto guard = EndWinGuard{};
  auto app = App{std::move(world)};
  int rc = app.run();
  return rc;
}
