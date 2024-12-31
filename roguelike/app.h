#pragma once
#include "confirm_ui.h"
#include "game_ui.h"
#include "map.h"

struct App {
  App(std::unique_ptr<World> world)
      : engine{std::make_shared<GameState>(std::move(world))},
        game_ui{engine},
        state{State::Game} {}

  int run() {
    while (true) {
      switch (state) {
        case State::Game: {
          game_ui.draw();
          auto event = game_ui.next();
          switch (event.type) {
            case EventType::System: {
              switch (event.sys_event.type) {
                case SystemEventType::Died: {
                  state = State::Confirmation;
                  confirm = std::make_unique<ConfirmUI>("game over: your died");
                  break;
                }
                case SystemEventType::Win: {
                  state = State::Confirmation;
                  confirm = std::make_unique<ConfirmUI>("game over: your win!");
                  break;
                }
              }
              break;
            }
            case EventType::Game: {
              engine->apply_event(event.game_event);
              break;
            }
            default:
              break;
          }
          break;
        }
        case State::Confirmation: {
          confirm->draw();
          confirm->next();
          return 0;
        }
        default: {
          panic("unexpected state");
          break;
        }
      }
    }
    return 0;
  }

 private:
  enum class State {
    Game,
    Confirmation,
  };

  std::shared_ptr<GameState> engine;
  GameUI game_ui;
  std::unique_ptr<ConfirmUI> confirm;
  State state;
};
