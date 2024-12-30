#pragma once
#include "decision_tree.h"
#include "inventory.h"
#include "items.h"
#include "state.h"

struct Level {
  Level();
  int get_exp() const;
  int get_lvl() const;

  /* Limit expr for current level. */
  int get_lvl_exp() const;

  void add_exp(int count);

 private:
  int lvl;
  int exp;
  int lvl_exp;
};

struct Player : public GameStateObject, IGameState::IPlayer, Inventory {
  friend class GameState;
  Player(int x, int y, int health, int max_health);

  IGameState::ObjectDescriptor get_descriptor() const override;

  std::tuple<int, int> get_health() const override;

  void move(const IGameState::PlayerMoveEvent& event);

  std::set<std::pair<int, int>> get_attack_area() const override;

  void heal(int hp);

  void set_pos(int xx, int yy);

  void damage(int hp);

  void add_exp(int count);

  int get_lvl() const override;

  int get_exp() const override;

  int get_lvl_exp() const override;
  void set_hand(std::unique_ptr<Stick> _hand);

  const Stick* get_hand();

 private:
  int health;
  int max_health;
  Level lvl{};
  std::unique_ptr<Stick> hand;
};

struct Mob : public GameStateObject, IGameState::IMob {
  friend class GameState;

  Mob(int x, int y, int health, int max_health, int attack_radius, int dmg,
      int exp, IGameState::ObjectDescriptor descriptor);

  void damage(int x);

  int internal_get_damage() const;

  virtual void move() = 0;

  std::set<std::pair<int, int>> get_attack_area() const override;

  std::tuple<int, int> get_health() const override;

  IGameState::ObjectDescriptor get_descriptor() const override;

 protected:
  virtual void apply();

 private:
  IGameState::ObjectDescriptor descriptor;
  int health;
  int max_health;
  int attack_radius;
  int dmg;
  int exp;
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

struct Enter : public GameStateObject, IGameState::IEnter {
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

// DecisionTreeMob executes decision tree to decide
// where to go.
struct DecisionTreeMob : public Mob {
  friend class GameState;

  DecisionTreeMob(int x, int y, int max_health, int dmg, int exp,
                  int attack_radius, IGameState::ObjectDescriptor descriptor,
                  std::shared_ptr<DecisionTreeNode> root);

  void move() override;

 private:
  std::shared_ptr<DecisionTreeNode> root;
};

struct OrcDecideAttack;
struct OrcDecideCloser;

// Stupid, just damages player.
// Runs to player when see him.
struct Orc : public DecisionTreeMob {
  friend class GameState;
  friend class OrcDecideAttack;
  friend class OrcDecideCloser;

  Orc(int x, int y);
};

struct OrcDecideAttack {
  bool operator()(Orc&);
};

struct OrcDecideCloser {
  bool operator()(Orc&);
};

struct BatDecideRun;
struct BatDecideSleep;

struct Bat : public DecisionTreeMob {
  friend class GameState;
  friend class BatDecideRun;
  friend class BatDecideSleep;

  Bat(int x, int y);
};

struct BatDecideRun {
  bool operator()(Bat&);
};

struct BatDecideSleep {
  bool operator()(Bat&);
};

struct ItemObject : public GameStateObject, IGameState::Object {
  friend class GameState;

  ItemObject(std::unique_ptr<GameState::Item> item, int x, int y);

  IGameState::ObjectDescriptor get_descriptor() const override;

  const IGameState::Item* get_item() const;

  std::optional<int> get_damage() const override;

  std::unique_ptr<GameState::Item> item;

  void apply() override;
};
