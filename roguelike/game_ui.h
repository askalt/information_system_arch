#pragma once

#include "curses.h"
#include "event.h"
#include "panic.h"
#include "state.h"

#define safe_call(f) \
  if (f() == ERR) {  \
    panic(#f);       \
  }

struct GameUI {
  GameUI(const GameState &state) : state(state) {}

  void draw() {
    static int c = 0;
    safe_call(clear);
    mvaddch(0, 0, ('A' + c) | A_BOLD);
    refresh();

    c = getch();
  }

  Event next() {
    return Event{.game_event = GameState::Event{
                     .player_move = GameState::PlayerMoveEvent{},
                     .type = GameState::EventType::PlayerMove,
                 }};
  }

  const GameState &state;
};
