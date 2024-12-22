#pragma once
#include "entities.h"

struct Map;
struct World;

struct Player;
struct Wall;
struct DungeonBlock;
struct Mob;
struct Enter;
struct Exit;
struct Border;

struct GameState : IGameState {
  friend class Player;
  friend class Wall;
  friend class DungeonBlock;
  friend class Mob;
  friend class Enter;
  friend class Exit;
  friend class Border;

  GameState(std::unique_ptr<World> world);
  const std::vector<Object*>& get_objects() const override;

  Object* get_player() const override;

  void apply_event(const Event& event) override;

 private:
  void player_move(const PlayerMoveEvent& event);

  void move_on(Map* map);

  void move_back();

  void apply(const ApplyEvent& e);

  struct MapStackNode {
    /* Position in the previous map. */
    int x;
    int y;
    const Map* map;
  };

  std::unique_ptr<World> world;
  std::vector<MapStackNode> map_stack;
};

// Objects of concrete state `GameState`.
struct GameStateObject {
  friend class GameState;

  GameStateObject();

  void set_state(GameState* state);

  // Apply object.
  virtual void apply() const {}

 protected:
  GameState* state{};
};
