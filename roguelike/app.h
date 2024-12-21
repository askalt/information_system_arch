#include "game_ui.h"

struct App {
  App() : state{std::make_shared<GameState>()}, game_ui{state} {}

  int run() {
    while (true) {
      auto event = game_ui.next();
      switch (event.type) {
        case EventType::System: {
          // todo.
          break;
        }
        case EventType::Game: {
          state->apply(event.game_event);
          break;
        }
        default:
          break;
      }
      game_ui.draw();
    }
    return 0;
  }

 private:
  std::shared_ptr<GameState> state;
  GameUI game_ui;
};
