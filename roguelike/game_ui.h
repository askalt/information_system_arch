#pragma once

#include <unordered_map>

#include "curses.h"
#include "event.h"
#include "panic.h"
#include "state.h"

#define safe_call(f) \
  if (f() == ERR) {  \
    panic(#f);       \
  }

/* Console sizes. */
const size_t W = 300;
const size_t H = 200;

/* Game field sizes. */
const size_t W_FIELD = 150;
const size_t H_FIELD = 70;

void init_UI(int argc, char *argv[]) {
#ifdef XCURSES
  Xinitscr(argc, argv);
#else
  initscr();
#endif
  resize_term(H, W);
}

const std::unordered_map<IGameState::PartDescriptor, char>
    PART_DESCRIPTOR_CHAR = {
        {IGameState::PartDescriptor::PLAYER, 'p'},
        {IGameState::PartDescriptor::WALL, '#'},
};

struct GameUI {
  GameUI(std::shared_ptr<const IGameState> state) : state(std::move(state)) {}

  void draw() {
    safe_call(clear);
    draw_border();
    const auto objects = state->get_objects();

    for (const auto &object : objects) {
      const auto parts = object->get_layout();
      for (const auto &part : parts) {
        int x = part.x % (H_FIELD - 2) + 1;
        int y = part.y % (W_FIELD - 2) + 1;
        auto chr = PART_DESCRIPTOR_CHAR.find(part.descriptor);
        assert(chr != PART_DESCRIPTOR_CHAR.end());
        mvaddch(x, y, chr->second);
      }
    }

    refresh();
  }

  Event next() {
    getch();
    return Event{.game_event =
                     GameState::Event{
                         .player_move = GameState::PlayerMoveEvent::Right,
                         .type = GameState::EventType::PlayerMove,
                     },
                 .type = EventType::Game};
  }

 private:
  void draw_border() {
    mvaddch(0, 0, '*');
    for (size_t i = 0; i < W_FIELD - 2; ++i) addch('-');
    addch('*');
    for (size_t i = 1; i < H_FIELD - 1; ++i) {
      mvaddch(i, 0, '|');
      mvaddch(i, W_FIELD - 1, '|');
    }
    mvaddch(H_FIELD - 1, 0, '*');
    for (size_t i = 0; i < W_FIELD - 2; ++i) addch('-');
    addch('*');
    refresh();
  }

  std::shared_ptr<const IGameState> state;
};
