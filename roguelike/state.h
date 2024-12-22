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
  void set_pos(int xx, int yy) {
    x = xx;
    y = yy;
  }

 private:
  int health;
  int max_health;
};

struct GameState : IGameState {
  GameState() {
    player = std::make_unique<Player>(9, 5, 20, 20);
    maps.push_back(std::unique_ptr{make_zero_map(player.get())});
    map_stack.push_back(maps.back().get());
  }

  const std::vector<Object*>& get_objects() const override {
    assert(!map_stack.empty());
    return map_stack.back()->objects;
  }

  Object* get_player() const override { return player.get(); }

  void apply(const Event& event) override {
    switch (event.type) {
      case EventType::PlayerMove:
        apply_player_move(event.player_move);
        break;
      case EventType::Enter:
        apply_enter(event.enter);
        break;
      default:
        break;
    }
  }

 private:
  void apply_player_move(const PlayerMoveEvent& event) {
    auto [x, y] = player->get_pos();
    apply_move(x, y, event);
    auto current_map = map_stack.back();
    /* Check on intersection, for now only with static objects. */
    for (const auto obj : current_map->objects) {
      if (obj != player.get()) {
        auto [xo, yo] = obj->get_pos();
        if (xo == x && yo == y) {
          return;
        }
      }
    }
    player->move(event);
  }

  void apply_enter(const EnterEvent&) {
    auto [x, y] = player->get_pos();
    int dx[] = {1, -1, 0, 0};
    int dy[] = {0, 0, 1, -1};
    Map* transition = nullptr;
    auto current_map = map_stack.back();
    for (const auto& enter : current_map->enters) {
      for (int i = 0; i < 4; ++i) {
        int xx = x + dx[i];
        int yy = y + dy[i];
        auto [xo, yo] = enter->get_pos();
        if (xo == xx && yo == yy) {
          transition = enter->map;
        }
      }
    }
    if (transition != nullptr) {
      map_stack.push_back(transition);
      auto [sx, sy] = transition->start_pos();
      player->set_pos(sx, sy);
    }
  }

  std::unique_ptr<Player> player;
  std::vector<std::unique_ptr<Map>> maps;
  std::vector<Map*> map_stack;
};
