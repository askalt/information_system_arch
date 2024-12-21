#pragma once

#include "entities.h"

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

struct Wall : IGameState::Object {
  Wall(int x, int y) : Object{x, y} {}
  Wall(int x, int y, std::string_view label)
      : Object{x, y}, label(std::move(label)) {}
  IGameState::ObjectDescriptor get_descriptor() const override {
    return IGameState::ObjectDescriptor::WALL;
  }
  std::optional<std::string_view> get_label() const override { return label; }
  std::optional<std::string_view> label;
};

struct Chest : IGameState::Object {
  Chest(int x, int y) : Object{x, y} {}
  IGameState::ObjectDescriptor get_descriptor() const override {
    return IGameState::ObjectDescriptor::CHEST;
  }
};

struct DungeonBlock : IGameState::Object {
  DungeonBlock(int x, int y, std::string_view label)
      : Object{x, y}, label(label) {}
  IGameState::ObjectDescriptor get_descriptor() const override {
    return IGameState::ObjectDescriptor::STONE;
  }
  std::optional<std::string_view> get_label() const override { return label; }
  std::string_view label;
};

struct Enter : IGameState::Object {
  Enter(int x, int y, std::string_view label) : Object{x, y}, label(label) {}
  IGameState::ObjectDescriptor get_descriptor() const override {
    return IGameState::ObjectDescriptor::ENTER;
  }
  std::optional<std::string_view> get_label() const override { return label; }
  std::string_view label;
};

struct GameState : IGameState {
  GameState() {
    player = std::make_unique<Player>(5, 5, 20, 20);
    objects.push_back(player.get());

    /* Home. */
    for (size_t i = 0; i < 10; ++i) {
      push_new_object(walls,
                      std::move(std::make_unique<Wall>(0, i, HOME_LABEL)));
    }
    for (size_t i = 1; i < 9; ++i) {
      push_new_object(walls,
                      std::move(std::make_unique<Wall>(i, 0, HOME_LABEL)));
      if (i < 4 || i > 7) {
        push_new_object(walls,
                        std::move(std::make_unique<Wall>(i, 9, HOME_LABEL)));
      }
    }
    for (size_t i = 0; i < 10; ++i) {
      push_new_object(walls,
                      std::move(std::make_unique<Wall>(9, i, HOME_LABEL)));
    }
    push_new_object(chests, std::move(std::make_unique<Chest>(1, 2)));
    push_new_object(chests, std::move(std::make_unique<Chest>(1, 3)));
    push_new_object(chests, std::move(std::make_unique<Chest>(4, 1)));
    push_new_object(chests, std::move(std::make_unique<Chest>(5, 1)));
    push_new_object(chests, std::move(std::make_unique<Chest>(6, 1)));

    push_new_object(dungeon_blocks, std::move(std::make_unique<DungeonBlock>(
                                        1, 25, LEVEL_0_DUNGEON)));
    push_new_object(dungeon_blocks, std::move(std::make_unique<DungeonBlock>(
                                        1, 26, LEVEL_0_DUNGEON)));
    push_new_object(dungeon_blocks, std::move(std::make_unique<DungeonBlock>(
                                        1, 27, LEVEL_0_DUNGEON)));
    push_new_object(dungeon_blocks, std::move(std::make_unique<DungeonBlock>(
                                        2, 24, LEVEL_0_DUNGEON)));
    push_new_object(dungeon_blocks, std::move(std::make_unique<DungeonBlock>(
                                        2, 28, LEVEL_0_DUNGEON)));
    push_new_object(dungeon_blocks, std::move(std::make_unique<DungeonBlock>(
                                        3, 25, LEVEL_0_DUNGEON)));
    push_new_object(dungeon_blocks, std::move(std::make_unique<DungeonBlock>(
                                        3, 27, LEVEL_0_DUNGEON)));
    push_new_object(enters,
                    std::move(std::make_unique<Enter>(2, 26, LEVEL_0_DUNGEON)));
  }

  const std::vector<Object*>& get_objects() const override { return objects; }

  Object* get_player() const override { return player.get(); }

  void apply(const Event& event) override {
    if (event.type == EventType::PlayerMove) {
      player->move(event.player_move);
    }
  }

 private:
  template <typename T>
  void push_new_object(std::vector<std::unique_ptr<T>>& container,
                       std::unique_ptr<T> object) {
    objects.push_back(object.get());
    container.push_back(std::move(object));
  }

  std::vector<std::unique_ptr<Enter>> enters;
  std::vector<std::unique_ptr<DungeonBlock>> dungeon_blocks;
  std::vector<std::unique_ptr<Chest>> chests;
  std::vector<std::unique_ptr<Wall>> walls;
  std::unique_ptr<Player> player;
  std::vector<Object*> objects;
};
