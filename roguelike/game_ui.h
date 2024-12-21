#pragma once

#include "event.h"
#include "state.h"

struct GameUI {
  GameUI(const GameState &state) : state(state) {}

  void draw();

  Event next();

  const GameState &state;
};
