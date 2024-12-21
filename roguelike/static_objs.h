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
  Enter(int x, int y, std::string_view label) : Object{x, y}, label(label) {}
  IGameState::ObjectDescriptor get_descriptor() const override {
    return IGameState::ObjectDescriptor::ENTER;
  }
  std::optional<std::string_view> get_label() const override { return label; }
  std::string_view label;
  // Each enter leads to the map.
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
