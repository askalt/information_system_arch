#pragma once
#include "state.h"
#include "system.h"

enum class EventType {
  System,
  Game,
};

struct Event {
  Event(SystemEvent sys_event)
      : sys_event{std::move(sys_event)}, type{EventType::System} {}

  Event(GameState::Event game_event)
      : game_event(std::move(game_event)), type{EventType::Game} {}

  union {
    SystemEvent sys_event;
    GameState::Event game_event;
  };
  EventType type;
};
