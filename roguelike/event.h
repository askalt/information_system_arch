#include "state.h"
#include "system.h"

enum class EventType {
  System,
  Game,
};

struct Event {
  union {
    SystemEvent sys_event;
    GameState::Event game_event;
  };
  EventType type;
};
