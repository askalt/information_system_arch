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
struct Chest;
struct Orc;
struct Bat;

struct GameState : IGameState {
  friend class Player;
  friend class Wall;
  friend class DungeonBlock;
  friend class Mob;
  friend class Enter;
  friend class Exit;
  friend class Border;
  friend class Orc;
  friend class Bat;
  friend class ItemObject;

  GameState(std::unique_ptr<World> world);
  const MapDescription get_map() const override;

  IGameState::IPlayer* get_player() const override;

  void apply_event(const Event& event) override;

  Map* get_current_map() const;

  void damage_player(int dmg);

  bool is_win() const override;

 private:
  void player_move(const PlayerMoveEvent& event);

  void move_on(Map* map);

  void move_back();

  void apply(const ApplyObjectEvent& e);

  struct MapStackNode {
    /* Position in the previous map. */
    int x;
    int y;
    Map* map;
  };

  std::unique_ptr<World> world;
  std::vector<MapStackNode> map_stack;
};

// Objects of concrete state `GameState`.
struct GameStateObject {
  friend class GameState;

  GameStateObject() = default;

  virtual void set_state(GameState* state);

  // Apply object.
  virtual void apply() {};

  virtual ~GameStateObject() = default;

  GameState* get_state() const;

 protected:
  GameState* state{};
};
