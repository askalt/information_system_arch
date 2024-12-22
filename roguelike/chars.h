#pragma once
#include "entities.h"

void apply_move(int& x, int& y, const IGameState::PlayerMoveEvent& event) {
  switch (event) {
    case IGameState::PlayerMoveEvent::Down: {
      x += 1;
      break;
    }
    case IGameState::PlayerMoveEvent::Up: {
      x -= 1;
      break;
    }
    case IGameState::PlayerMoveEvent::Left: {
      y -= 1;
      break;
    }
    case IGameState::PlayerMoveEvent::Right: {
      y += 1;
      break;
    }
  }
}

struct Player : IGameState::Object {
  Player(int x, int y, int health, int max_health)
      : Object{x, y}, health{health}, max_health{max_health} {}
  IGameState::ObjectDescriptor get_descriptor() const override {
    return IGameState::ObjectDescriptor::PLAYER;
  }
  std::optional<std::tuple<int, int>> get_health() const override {
    return {{health, max_health}};
  }
  void move(const IGameState::PlayerMoveEvent& event) {
    apply_move(x, y, event);
  }
  void set_pos(int xx, int yy) {
    x = xx;
    y = yy;
  }

 private:
  int health;
  int max_health;
};
