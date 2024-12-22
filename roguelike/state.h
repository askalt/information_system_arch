#pragma once

#include "cassert"
#include "chars.h"
#include "entities.h"
#include "map.h"
#include "static_objs.h"

struct GameState : IGameState {
  GameState(World world) : world(std::move(world)) {
    map_stack.push_back(
        MapStackNode{.x = -1, .y = -1, .map = this->world.start_map});
  }

  const std::vector<Object*>& get_objects() const override {
    assert(!map_stack.empty());
    return map_stack.back().map->objects;
  }

  Object* get_player() const override { return world.player.get(); }

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
    auto [x, y] = world.player->get_pos();
    apply_move(x, y, event);
    auto current_map = map_stack.back().map;
    /* Check on intersection, for now only with static objects. */
    for (const auto obj : current_map->objects) {
      if (obj != world.player.get()) {
        auto [xo, yo] = obj->get_pos();
        if (xo == x && yo == y) {
          return;
        }
      }
    }
    world.player->move(event);
  }

  void apply_enter(const EnterEvent&) {
    auto [x, y] = world.player->get_pos();
    int dx[] = {1, -1, 0, 0};
    int dy[] = {0, 0, 1, -1};
    const Map* transition = nullptr;
    auto current_map = map_stack.back().map;
    /* Check on enter. */
    for (const auto& enter : current_map->enters) {
      for (int i = 0; i < 4; ++i) {
        int xx = x + dx[i];
        int yy = y + dy[i];
        auto [xo, yo] = enter->get_pos();
        if (xo == xx && yo == yy) {
          transition = enter->get_map();
        }
      }
    }
    if (transition != nullptr) {
      map_stack.push_back(MapStackNode{.x = x, .y = y, .map = transition});
      auto [sx, sy] = transition->start_pos();
      world.player->set_pos(sx, sy);
    }
    /* Check on exit. */
    auto [xe, ye] = current_map->start_pos();
    bool near_exit = false;
    for (int i = 0; i < 4; ++i) {
      int xx = x + dx[i];
      int yy = y + dy[i];
      if (xx == xe && yy == ye) {
        near_exit = true;
      }
    }
    if (near_exit && map_stack.size() > 1) {
      world.player->set_pos(map_stack.back().x, map_stack.back().y);
      map_stack.pop_back();
    }
  }

  struct MapStackNode {
    /* Position in the previous map. */
    int x;
    int y;
    const Map* map;
  };

  World world;
  std::vector<MapStackNode> map_stack;
};
