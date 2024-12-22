#include "objects.h"

#include <algorithm>
#include <string_view>

#include "map.h"

int get_exp_by_lvl(int lvl) {
  int p = 15;
  while (lvl > 0) {
    p = static_cast<int>(1.5 * p);
    lvl--;
  }
  return p;
}

/* LVL impl. */
Level::Level() : lvl{0}, exp{0}, lvl_exp{get_exp_by_lvl(0)} {}

int Level::get_exp() const { return exp; }

int Level::get_lvl() const { return lvl; }

int Level::get_lvl_exp() const { return lvl_exp; }

void Level::add_exp(int count) {
  exp += count;
  while (exp >= lvl_exp) {
    exp -= lvl_exp;
    lvl++;
    lvl_exp = get_exp_by_lvl(lvl);
  }
}

/* Player impl. */
Player::Player(int x, int y, int health, int max_health)
    : IGameState::IPlayer{x, y}, health{health}, max_health{max_health} {}

IGameState::ObjectDescriptor Player::get_descriptor() const {
  return IGameState::ObjectDescriptor::PLAYER;
}

std::tuple<int, int> Player::get_health() const { return {health, max_health}; }

void Player::move(const IGameState::PlayerMoveEvent& event) {
  int xx = x;
  int yy = y;
  apply_move(xx, yy, event);
  auto map = state->get_current_map();
  /* Check on intersection. */
  if (!map->has_object(xx, yy, static_cast<IGameState::Object*>(this))) {
    set_pos(xx, yy);
  }
}

int Player::get_lvl() const { return lvl.get_lvl(); }

int Player::get_exp() const { return lvl.get_exp(); }

int Player::get_lvl_exp() const { return lvl.get_lvl_exp(); }

void Player::heal(int hp) { health = std::min(max_health, health + hp); }

void Player::damage(int x) { health = std::max(health - x, 0); }

void Player::set_pos(int xx, int yy) {
  x = xx;
  y = yy;
}

void Player::add_exp(int count) { lvl.add_exp(count); }

/* Mob impl/ */
Mob::Mob(int x, int y, int health, int max_health, int dmg, int exp,
         IGameState::ObjectDescriptor descriptor)
    : IGameState::IHealthable{x, y},
      descriptor{descriptor},
      health{health},
      max_health{max_health},
      dmg{dmg},
      exp{exp} {}

IGameState::ObjectDescriptor Mob::get_descriptor() const { return descriptor; }

std::tuple<int, int> Mob::get_health() const { return {health, max_health}; }

void Mob::damage(int x) {
  health = std::max(health - x, 0);
  if (health == 0) {
    auto map = state->get_current_map();
    map->remove_object(map->mobs, this);
    /* Add exp. */
    dynamic_cast<Player*>(state->get_player())->add_exp(exp);
  }
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
    : IGameState::IEnter{x, y, std::move(transition)}, label(label) {}

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

const int ORC_DAMAGE_RADIUS = 3;
const int ORC_VIEW_FIELD = 6;
const int ORC_DMG = 2;

/* Orc impl. */
Orc::Orc(int x, int y)
    : Mob{x, y, 15, 15, ORC_DMG, 4, IGameState::ObjectDescriptor::ORC} {}

void Orc::move() {
  auto [px, py] = state->get_player()->get_pos();
  auto map = state->get_current_map();
  int dist = abs(x - px) + abs(y - py);
  if (dist < ORC_DAMAGE_RADIUS) {
    dynamic_cast<Player*>(state->get_player())->damage(2);
  } else if (dist <= ORC_VIEW_FIELD) {
    /* View field, try to be closer. */
    const int dx[] = {0, 1, -1, 0, 0};
    const int dy[] = {0, 0, 0, 1, -1};
    std::pair<int, int> vars[5]{};
    int j = 0;
    for (int i = 0; i < sizeof(dx) / sizeof(int); ++i) {
      int xx = x + dx[i];
      int yy = y + dy[i];
      if (!map->has_object(xx, yy, static_cast<IGameState::Object*>(this))) {
        vars[j].second = i;
        vars[j].first = abs(xx - px) + abs(yy - py);
        j++;
      }
    }
    if (j == 0) {
      return;
    }
    if (dist <= ORC_DAMAGE_RADIUS && rand() % 3 == 0) {
      /*
       * The orc can strike despite the fact that it will not
       * catch up with the enemy further with
       * a probability of 33%.
       */
      dynamic_cast<Player*>(state->get_player())->damage(dmg);
      return;
    }
    /* Choose random step that will bring us as
     * close to the player as possible.
     */
    sort(vars, vars + j);
    int cntv = 0;
    while (cntv < sizeof(vars) / sizeof(vars[0]) &&
           vars[cntv].first == vars[0].first) {
      cntv++;
    }
    int choose = rand() % cntv;
    int new_x = x + dx[vars[choose].second];
    int new_y = y + dy[vars[choose].second];
    x = new_x;
    y = new_y;
  }

  /* TODO: random walk. */
}

const int BAT_DMG = 0;
const int BAT_EXP = 2;

Bat::Bat(int x, int y)
    : Mob{x, y, 7, 7, BAT_DMG, BAT_EXP, IGameState::ObjectDescriptor::BAT} {}

void Bat::move() {
  /*
   * Bat can sleep with 25% probability.
   */
  if (rand() % 4 == 0) {
    return;
  }
  auto [px, py] = state->get_player()->get_pos();
  auto map = state->get_current_map();
  int dist = abs(x - px) + abs(y - py);
  if (dist > 7) {
    /* Sleep. */
    return;
  }
  /* View field, try to be closer. */
  const int dx[] = {0, 1, -1, 0, 0};
  const int dy[] = {0, 0, 0, 1, -1};
  std::pair<int, int> vars[5]{};
  int j = 0;
  for (int i = 0; i < sizeof(dx) / sizeof(int); ++i) {
    int xx = x + dx[i];
    int yy = y + dy[i];
    if (!map->has_object(xx, yy, static_cast<IGameState::Object*>(this))) {
      vars[j].second = i;
      vars[j].first = -(abs(xx - px) + abs(yy - py));
      j++;
    }
    if (j == 0) {
      return;
    }
  }
  sort(vars, vars + j);
  int cntv = 0;
  while (cntv < sizeof(vars) / sizeof(vars[0]) &&
         vars[cntv].first == vars[0].first) {
    cntv++;
  }
  int choose = rand() % cntv;
  int new_x = x + dx[vars[choose].second];
  int new_y = y + dy[vars[choose].second];
  x = new_x;
  y = new_y;
}
