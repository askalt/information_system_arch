#pragma once
#include "inventory.h"
#include "items.h"
#include "state.h"

struct Player : public GameStateObject, IGameState::Object, Inventory {
  friend class GameState;
  Player(int x, int y, int health, int max_health);

  IGameState::ObjectDescriptor get_descriptor() const override;

  std::optional<std::tuple<int, int>> get_health() const override;

  void move(const IGameState::PlayerMoveEvent& event);

  void heal(int hp);

  void set_pos(int xx, int yy);

  void damage(int hp);

  void set_hand(std::unique_ptr<Stick> _hand);

  const Stick *get_hand();

 private:
  //std::unique_ptr<Stick> stick;
  int health;
  int max_health;
  std::unique_ptr<Stick> hand;
};

struct Mob : public GameStateObject, IGameState::Object {
  friend class GameState;

  Mob(int x, int y, int health, int max_health,
      IGameState::ObjectDescriptor descriptor);

  void damage(int x);

  virtual void move() = 0;

  IGameState::ObjectDescriptor get_descriptor() const override;

  virtual void apply();

 private:
  IGameState::ObjectDescriptor descriptor;
  int health;
  int max_health;
};

struct Wall : public GameStateObject, IGameState::Object {
  Wall(int x, int y);

  Wall(int x, int y, std::string_view label);

  IGameState::ObjectDescriptor get_descriptor() const override;

  std::optional<std::string_view> get_label() const override;

  std::optional<std::string_view> label;

  friend class GameState;
};

struct Chest : public GameStateObject, IGameState::Object {
  Chest(int x, int y);

  IGameState::ObjectDescriptor get_descriptor() const override;

  friend class GameState;
};

struct DungeonBlock : public GameStateObject, IGameState::Object {
  DungeonBlock(int x, int y, std::string_view label);

  IGameState::ObjectDescriptor get_descriptor() const override;
  std::optional<std::string_view> get_label() const override;
  std::string_view label;
  friend class GameState;
};

struct Enter : public GameStateObject, IGameState::EnterObj {
  friend class GameState;

  Enter(int x, int y, std::string_view label, std::string transition);

  IGameState::ObjectDescriptor get_descriptor() const override;

  void set_map(Map* to_map);

  std::optional<std::string_view> get_label() const;

  const std::string& get_transition() const;

  const Map* get_map() const;

  void apply() override;

 private:
  std::string_view label;
  // Each enter leads to some map (edge).
  Map* map;
};

struct Border : public GameStateObject, IGameState::Object {
  friend class GameState;

  Border(int x, int y, IGameState::ObjectDescriptor descriptor);

  IGameState::ObjectDescriptor get_descriptor() const override;

  IGameState::ObjectDescriptor descriptor;
};

struct Exit : public GameStateObject, IGameState::Object {
  friend class GameState;

  Exit(int x, int y);

  IGameState::ObjectDescriptor get_descriptor() const override;

  void apply() override;
};

// Stupid, just damages player.
// Runs to player when see him.
struct Orc : public Mob {
  friend class GameState;

  Orc(int x, int y);

  void move() override;
};

struct Bat : public Mob {
  friend class GameState;

  Bat(int x, int y);

  void move() override;
};

struct ItemObject : public GameStateObject, IGameState::Object {
  friend class GameState;

  ItemObject(std::unique_ptr<GameState::Item> item, int x, int y);

  IGameState::ObjectDescriptor get_descriptor() const override;

  const IGameState::Item *get_item() const;

  std::unique_ptr<GameState::Item> item;

  void apply() override;
};
