#include <string>

#include "event.h"
#include "ncurses.h"

const std::string continueMsg = "press any key to continue...";

struct ConfirmUI {
  ConfirmUI(std::string msg) : msg(std::move(msg)) {}

  void draw() {
    erase();
    move(0, 0);
    for (int i = 0; i < W; ++i) {
      addch('*');
    }
    for (int i = 1; i < H - 1; ++i) {
      move(i, 0);
      addch('*');
      move(i, W - 1);
      addch('*');
      if (i == H / 2) {
        int x = (W - msg.size()) / 2;
        move(i, x);
        printw(msg.data());
      } else if (i == H / 2 + 2) {
        int x = (W - continueMsg.size()) / 2;
        move(i, x);
        printw(continueMsg.data());
      }
    }
    move(H - 1, 0);
    for (int i = 0; i < W; ++i) {
      addch('*');
    }
  }

  Event next() {
    getch();
    return SystemEvent{.type = SystemEventType::Win};
  }

 private:
  const int W = 40;
  const int H = 20;

  std::string msg;
};
