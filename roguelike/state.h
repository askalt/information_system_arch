#pragma once

#include "entities.h"

struct PlayerObject : IGameState::Object {
  virtual const std::vector<IGameState::ObjectPart>& get_layout() {
    return parts;
  }

  std::vector<IGameState::ObjectPart> parts = {IGameState::ObjectPart{
      .descriptor = IGameState::PartDescriptor::PLAYER,
      .x = 0,
      .y = 0,
  }};

  void move(const IGameState::PlayerMoveEvent& event) {
    switch (event) {
      case IGameState::PlayerMoveEvent::Down: {
        parts[0].x += 1;
        break;
      }
      case IGameState::PlayerMoveEvent::Up: {
        parts[0].x -= 1;
        break;
      }
      case IGameState::PlayerMoveEvent::Left: {
        parts[0].y -= 1;
        break;
      }
      case IGameState::PlayerMoveEvent::Right: {
        parts[0].y += 1;
        break;
      }
    }
  }
};

struct GameState : IGameState {
  GameState() {
    player = std::make_unique<PlayerObject>();
    objects.push_back(player.get());
  }

  const std::vector<Object*>& get_objects() const override { return objects; }

  void apply(const Event& event) override {
    if (event.type == EventType::PlayerMove) {
      player->move(event.player_move);
    }
  }

  std::unique_ptr<PlayerObject> player;
  std::vector<Object*> objects;
};
