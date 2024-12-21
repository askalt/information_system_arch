#include "app.h"
#include "curses.h"

int main(int argc, char *argv[]) {
#ifdef XCURSES
  Xinitscr(argc, argv);
#else
  initscr();
#endif
  auto app = App{};
  return app.run();
}
