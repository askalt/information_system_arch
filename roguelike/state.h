#pragma once

#include "cassert"
#include "entities.h"
#include "static_objs.h"

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

 private:
  int health;
  int max_health;
};

const std::string HOME_LABEL = "HOME";
const std::string LEVEL_0_DUNGEON = "DUNGEON (level 0)";

struct Map {
  /* Each map contains player. */
  Map(IGameState::Object* player) { objects.push_back(player); }
  std::vector<std::unique_ptr<Enter>> enters;
  std::vector<std::unique_ptr<DungeonBlock>> dungeon_blocks;
  std::vector<std::unique_ptr<Chest>> chests;
  std::vector<std::unique_ptr<Wall>> walls;

  /* All objects that map contains. */
  std::vector<IGameState::Object*> objects;

  template <typename T>
  void push_new_object(std::vector<std::unique_ptr<T>>& container,
                       std::unique_ptr<T> object) {
    objects.push_back(object.get());
    container.push_back(std::move(object));
  }
};

// Creates a zero map from here player begins.
std::unique_ptr<Map> make_zero_map(IGameState::Object* player) {
  auto mp = std::make_unique<Map>(player);
  /* Home. */
  for (size_t i = 0; i < 10; ++i) {
    mp->push_new_object(mp->walls,
                        std::move(std::make_unique<Wall>(0, i, HOME_LABEL)));
  }
  for (size_t i = 1; i < 9; ++i) {
    mp->push_new_object(mp->walls,
                        std::move(std::make_unique<Wall>(i, 0, HOME_LABEL)));
    if (i < 4 || i > 7) {
      mp->push_new_object(mp->walls,
                          std::move(std::make_unique<Wall>(i, 9, HOME_LABEL)));
    }
  }
  for (size_t i = 0; i < 10; ++i) {
    mp->push_new_object(mp->walls,
                        std::move(std::make_unique<Wall>(9, i, HOME_LABEL)));
  }
  mp->push_new_object(mp->chests, std::move(std::make_unique<Chest>(1, 2)));
  mp->push_new_object(mp->chests, std::move(std::make_unique<Chest>(1, 3)));
  mp->push_new_object(mp->chests, std::move(std::make_unique<Chest>(4, 1)));
  mp->push_new_object(mp->chests, std::move(std::make_unique<Chest>(5, 1)));
  mp->push_new_object(mp->chests, std::move(std::make_unique<Chest>(6, 1)));

  /* First dungeon. */
  mp->push_new_object(
      mp->dungeon_blocks,
      std::move(std::make_unique<DungeonBlock>(1, 25, LEVEL_0_DUNGEON)));
  mp->push_new_object(
      mp->dungeon_blocks,
      std::move(std::make_unique<DungeonBlock>(1, 26, LEVEL_0_DUNGEON)));
  mp->push_new_object(
      mp->dungeon_blocks,
      std::move(std::make_unique<DungeonBlock>(1, 27, LEVEL_0_DUNGEON)));
  mp->push_new_object(
      mp->dungeon_blocks,
      std::move(std::make_unique<DungeonBlock>(2, 24, LEVEL_0_DUNGEON)));
  mp->push_new_object(
      mp->dungeon_blocks,
      std::move(std::make_unique<DungeonBlock>(2, 28, LEVEL_0_DUNGEON)));
  mp->push_new_object(
      mp->dungeon_blocks,
      std::move(std::make_unique<DungeonBlock>(3, 25, LEVEL_0_DUNGEON)));
  mp->push_new_object(
      mp->dungeon_blocks,
      std::move(std::make_unique<DungeonBlock>(3, 27, LEVEL_0_DUNGEON)));
  mp->push_new_object(
      mp->enters, std::move(std::make_unique<Enter>(2, 26, LEVEL_0_DUNGEON)));
  return mp;
}

struct GameState : IGameState {
  GameState() {
    player = std::make_unique<Player>(5, 5, 20, 20);
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
      player->move(event.player_move);
    }
  }

 private:
  std::unique_ptr<Player> player;
  std::vector<std::unique_ptr<Map>> maps;
  Map* current_map;
};
