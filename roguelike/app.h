#pragma once
#include "game_ui.h"
#include "map.h"

struct App {
  App(std::unique_ptr<World> world)
      : state{std::make_shared<GameState>(std::move(world))}, game_ui{state} {}

  int run() {
    while (true) {
      game_ui.draw();
      auto event = game_ui.next();
      switch (event.type) {
        case EventType::System: {
          // todo.
          break;
        }
        case EventType::Game: {
          state->apply_event(event.game_event);
          break;
        }
        default:
          break;
      }
    }
    return 0;
  }

 private:
  std::shared_ptr<GameState> state;
  GameUI game_ui;
};
