#pragma once

#include "cassert"
#include "entities.h"
#include "static_objs.h"
#include "zero_map.h"

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

 private:
  int health;
  int max_health;
};

struct GameState : IGameState {
  GameState() {
    player = std::make_unique<Player>(9, 5, 20, 20);
    maps.push_back(std::unique_ptr{make_zero_map(player.get())});
    current_map = maps.back().get();
  }

  const std::vector<Object*>& get_objects() const override {
    assert(current_map);
    return current_map->objects;
  }

  Object* get_player() const override { return player.get(); }

  void apply(const Event& event) override {
    if (event.type == EventType::PlayerMove) {
      auto [x, y] = player->get_pos();
      apply_move(x, y, event.player_move);
      /* Check on intersection, for now only with static objects. */
      for (const auto obj : current_map->objects) {
        if (obj != player.get()) {
          auto [xo, yo] = obj->get_pos();
          if (xo == x && yo == y) {
            return;
          }
        }
      }
      player->move(event.player_move);
    }
  }

 private:
  std::unique_ptr<Player> player;
  std::vector<std::unique_ptr<Map>> maps;
  Map* current_map;
};
