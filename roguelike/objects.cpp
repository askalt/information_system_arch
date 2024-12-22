#include "objects.h"

#include <string_view>

#include "map.h"

/* Player impl. */
Player::Player(int x, int y, int health, int max_health)
    : IGameState::Object{x, y}, health{health}, max_health{max_health} {}

IGameState::ObjectDescriptor Player::get_descriptor() const {
  return IGameState::ObjectDescriptor::PLAYER;
}

std::optional<std::tuple<int, int>> Player::get_health() const {
  return {{health, max_health}};
}

void Player::move(const IGameState::PlayerMoveEvent& event) {
  int xx = x;
  int yy = y;
  apply_move(xx, yy, event);

  auto as_object = static_cast<IGameState::Object*>(this);
  /* Check on intersection, for now only with static objects. */
  auto map = state->get_current_map();
  for (const auto obj : map->objects) {
    if (obj != as_object) {
      auto [xo, yo] = obj->get_pos();
      if (xo == xx && yo == yy) {
        return;
      }
    }
  }
  set_pos(xx, yy);
}

void Player::heal(int hp) { health = std::min(max_health, health + hp); }

void Player::set_pos(int xx, int yy) {
  x = xx;
  y = yy;
}

/* Mob impl/ */
Mob::Mob(int x, int y, int health, int max_health)
    : IGameState::Object{x, y}, health{health}, max_health{max_health} {}

void Mob::damage(int x) {
  health -= x;
  health = std::max(health, 0);
}

/* Wall impl. */
Wall::Wall(int x, int y) : IGameState::Object{x, y} {}
Wall::Wall(int x, int y, std::string_view label)
    : IGameState::Object{x, y}, label(std::move(label)) {}
IGameState::ObjectDescriptor Wall::get_descriptor() const {
  return IGameState::ObjectDescriptor::WALL;
}
std::optional<std::string_view> Wall::get_label() const { return label; }

/* Chest impl. */
Chest::Chest(int x, int y) : IGameState::Object{x, y} {}
IGameState::ObjectDescriptor Chest::get_descriptor() const {
  return IGameState::ObjectDescriptor::CHEST;
}

/* DungeonBlock impl. */
DungeonBlock::DungeonBlock(int x, int y, std::string_view label)
    : IGameState::Object{x, y}, label(label) {}
IGameState::ObjectDescriptor DungeonBlock::get_descriptor() const {
  return IGameState::ObjectDescriptor::STONE;
}
std::optional<std::string_view> DungeonBlock::get_label() const {
  return label;
}

/* Enter impl. */
Enter::Enter(int x, int y, std::string_view label, std::string transition)
    : IGameState::EnterObj{x, y, std::move(transition)}, label(label) {}

IGameState::ObjectDescriptor Enter ::get_descriptor() const {
  return IGameState::ObjectDescriptor::ENTER;
}

void Enter::set_map(Map* to_map) { map = to_map; }

std::optional<std::string_view> Enter::get_label() const { return label; }

const std::string& Enter::get_transition() const { return transition; }

const Map* Enter::get_map() const { return map; }

void Enter::apply() const {
  auto [xp, yp] = state->get_player()->get_pos();
  if (map != nullptr && abs(xp - x) + abs(yp - y) <= 1) {
    state->move_on(map);
  }
}

/* Border impl. */
Border::Border(int x, int y, IGameState::ObjectDescriptor descriptor)
    : IGameState::Object{x, y}, descriptor{descriptor} {}

IGameState::ObjectDescriptor Border::get_descriptor() const {
  return descriptor;
}

/* Exit impl. */
Exit::Exit(int x, int y) : IGameState::Object{x, y} {}

IGameState::ObjectDescriptor Exit::get_descriptor() const {
  return IGameState::ObjectDescriptor::EXIT;
}

void Exit::apply() const {
  auto [xp, yp] = state->get_player()->get_pos();
  if (abs(xp - x) + abs(yp - y) <= 1) {
    state->move_back();
  }
}

/* Orc impl. */
