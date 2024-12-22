#pragma once
#include "entities.h"

struct Map;

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
  Enter(int x, int y, std::string_view label, std::string transition)
      : Object{x, y}, label(label), transition(std::move(transition)) {}
  IGameState::ObjectDescriptor get_descriptor() const override {
    return IGameState::ObjectDescriptor::ENTER;
  }

  void set_map(Map* to_map) { map = to_map; }
  std::optional<std::string_view> get_label() const override { return label; }
  const std::string& get_transition() const { return transition; }
  const Map* get_map() const { return map; }

 private:
  std::string_view label;
  std::string transition;
  // Each enter leads to some map (edge).
  Map* map;
};

struct Border : IGameState::Object {
  Border(int x, int y, IGameState::ObjectDescriptor descriptor)
      : Object{x, y}, descriptor{descriptor} {}
  IGameState::ObjectDescriptor get_descriptor() const override {
    return descriptor;
  }
  IGameState::ObjectDescriptor descriptor;
};

struct Exit : IGameState::Object {
  Exit(int x, int y) : Object{x, y} {}
  IGameState::ObjectDescriptor get_descriptor() const override {
    return IGameState::ObjectDescriptor::EXIT;
  }
};
