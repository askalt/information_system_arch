#pragma once

// System event:
// - game over
// - save game
// - etc.

enum class SystemEventType {
  Win,
  Died,
};

struct SystemEvent {
  SystemEventType type;
};
