#include "app.h"
#include "curses.h"

int main(int argc, char *argv[]) {
  init_UI(argc, argv);
  auto app = App{};
  return app.run();
}
