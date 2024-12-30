#include "objects.h"

#include <algorithm>
#include <deque>
#include <set>
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
    : IGameState::IPlayer{x, y},
      Inventory{5},
      health{health},
      max_health{max_health} {}

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

std::set<std::pair<int, int>> bfs_get_attack_area(Map* map, int x, int y,
                                                  int d) {
  const int dx[] = {0, 0, 1, -1};
  const int dy[] = {-1, 1, 0, 0};

  std::set<std::pair<int, int>> vis;
  std::deque<std::pair<int, int>> q;
  q.push_back({x, y});
  vis.insert({x, y});
  auto obstacles = map->get_obstacles();
  while (!q.empty()) {
    auto [nx, ny] = q.front();
    q.pop_front();
    for (size_t i = 0; i < sizeof(dx) / sizeof(int); ++i) {
      auto xx = nx + dx[i];
      auto yy = ny + dy[i];
      if (obstacles.find({xx, yy}) == obstacles.end() &&
          vis.find({xx, yy}) == vis.end() && abs(xx - x) + abs(yy - y) <= d) {
        vis.insert({xx, yy});
        q.push_back({xx, yy});
      }
    }
  }

  return vis;
}

std::set<std::pair<int, int>> Player::get_attack_area() const {
  const int d = hand ? hand->radius : 0;
  return bfs_get_attack_area(state->get_current_map(), x, y, d);
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

void Player::set_hand(std::unique_ptr<Stick> _hand) { hand = std::move(_hand); }

const Stick* Player::get_hand() { return hand.get(); }

/* Mob impl/ */
Mob::Mob(int x, int y, int health, int max_health, int attack_radius, int dmg,
         int exp, IGameState::ObjectDescriptor descriptor)
    : IGameState::IMob{x, y},
      descriptor{descriptor},
      health{health},
      max_health{max_health},
      attack_radius{attack_radius},
      dmg{dmg},
      exp{exp} {}

IGameState::ObjectDescriptor Mob::get_descriptor() const { return descriptor; }

std::tuple<int, int> Mob::get_health() const { return {health, max_health}; }

int Mob::internal_get_damage() const { return dmg; }

void Mob::damage(int x) {
  health = std::max(health - x, 0);
  if (health == 0) {
    auto map = state->get_current_map();
    /* Add exp. */
    dynamic_cast<Player*>(state->get_player())->add_exp(exp);
    map->remove_object(map->mobs, this);
  }
}

std::set<std::pair<int, int>> Mob::get_attack_area() const {
  return bfs_get_attack_area(state->get_current_map(), x, y, attack_radius);
}

void Mob::apply() {
  auto as_player = dynamic_cast<Player*>(state->get_player());
  auto hand = as_player->get_hand();
  if (hand != nullptr) {
    auto [xp, yp] = as_player->get_pos();
    auto [xm, ym] = get_pos();
    int dist = std::abs(xp - xm) + std::abs(yp - ym);
    if (dist <= hand->radius) damage(hand->damage);
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
    : IGameState::IEnter{x, y, std::move(transition)},
      label(label),
      map(nullptr) {}

IGameState::ObjectDescriptor Enter ::get_descriptor() const {
  return IGameState::ObjectDescriptor::ENTER;
}

void Enter::set_map(Map* to_map) { map = to_map; }

std::optional<std::string_view> Enter::get_label() const { return label; }

const std::string& Enter::get_transition() const { return transition; }

const Map* Enter::get_map() const { return map; }

void Enter::apply() {
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

void Exit::apply() {
  auto [xp, yp] = state->get_player()->get_pos();
  if (abs(xp - x) + abs(yp - y) <= 1) {
    state->move_back();
  }
}

const int ORC_DAMAGE_RADIUS = 3;
const int ORC_VIEW_FIELD = 6;
const int ORC_DMG = 2;

struct DistanceComparatorLess {
  bool operator()(const std::pair<int, int>& lhs,
                  const std::pair<int, int>& rhs) const {
    if (lhs.first != rhs.first) {
      return lhs.first < rhs.first;
    }
    return false;
  }
};

struct DistanceComparatorGreater {
  bool operator()(const std::pair<int, int>& lhs,
                  const std::pair<int, int>& rhs) const {
    if (lhs.first != rhs.first) {
      return lhs.first > rhs.first;
    }
    return false;
  }
};

bool OrcDecideAttack::operator()(Orc& orc) {
  auto [x, y] = orc.get_pos();
  auto player = dynamic_cast<Player*>(orc.state->get_player());
  auto [px, py] = player->get_pos();
  int dist = abs(x - px) + abs(y - py);
  if (dist < ORC_DAMAGE_RADIUS) {
    /* Attack always when player is too close
     * to could attack on next step.
     */
    return true;
  }
  if (dist == ORC_DAMAGE_RADIUS && rand() % 3 == 0) {
    /*
     * Orc can strike despite the fact that he will not
     * catch up with the enemy further with
     * a probability of 33%.
     */
    return true;
  }
  return false;
}

bool OrcDecideCloser::operator()(Orc& orc) {
  auto [x, y] = orc.get_pos();
  auto player = dynamic_cast<Player*>(orc.state->get_player());
  auto [px, py] = player->get_pos();
  int dist = abs(x - px) + abs(y - py);
  return dist <= ORC_VIEW_FIELD;
}

/* Orc impl. */
Orc::Orc(int x, int y)
    : DecisionTreeMob{
          x,
          y,
          15,
          ORC_DMG,
          4,
          ORC_DAMAGE_RADIUS,
          IGameState::ObjectDescriptor::ORC,
          std::make_shared<ConditionNode<Orc>>(
              *this,
              /* Conditions. */
              std::vector<ConditionNode<Orc>::Node>{
                  {
                      .predicate = [](Orc& orc) -> bool {
                        return OrcDecideAttack{}(orc);
                      },
                      .next = std::make_shared<AttackNode<Orc>>(*this),
                  },
                  {
                      .predicate = [](Orc& orc) -> bool {
                        return OrcDecideCloser{}(orc);
                      },
                      .next = std::make_shared<
                          DistanceComparatorNode<Orc, DistanceComparatorLess>>(
                          *this),
                  },
              },
              /* Default. */
              std::make_shared<RandowWalkNode<Orc>>(x, y, 8, *this))} {}

/* Bat impl. */
const int BAT_DMG = 0;
const int BAT_EXP = 2;

bool BatDecideRun::operator()(Bat& bat) {
  auto [x, y] = bat.get_pos();
  auto player = dynamic_cast<Player*>(bat.state->get_player());
  auto [px, py] = player->get_pos();
  int dist = abs(x - px) + abs(y - py);
  return dist <= 5;
}

bool BatDecideSleep::operator()(Bat& bat) {
  /*
   * Bat can sleep with 25% probability.
   */
  return rand() % 4 == 0;
}

Bat::Bat(int x, int y)
    : DecisionTreeMob{
          x,
          y,
          7,
          0,
          BAT_EXP,
          0,
          IGameState::ObjectDescriptor::BAT,
          std::make_shared<ConditionNode<Bat>>(
              *this,
              std::vector<ConditionNode<Bat>::Node>{
                  {
                      .predicate = [](Bat& bat) -> bool {
                        return BatDecideSleep{}(bat);
                      },
                      .next = std::make_shared<NoOpNode>(),
                  },
                  {
                      .predicate = [](Bat& bat) -> bool {
                        return BatDecideRun{}(bat);
                      },
                      .next = std::make_shared<DistanceComparatorNode<
                          Bat, DistanceComparatorGreater>>(*this),
                  },
              },
              /* Default. */
              std::make_shared<RandowWalkNode<Bat>>(x, y, 8, *this))} {}

/* DecisionTreeMob impl. */
DecisionTreeMob::DecisionTreeMob(int x, int y, int max_health, int dmg, int exp,
                                 int attack_radius,
                                 IGameState::ObjectDescriptor descriptor,
                                 std::shared_ptr<DecisionTreeNode> root)
    : Mob{x, y, max_health, max_health, attack_radius, dmg, exp, descriptor},
      root{root} {}

void DecisionTreeMob::move() { interpretate(root); }

/* Item impl. */
ItemObject::ItemObject(std::unique_ptr<GameState::Item> item, int x, int y)
    : item(std::move(item)), IGameState::Object{x, y} {}

IGameState::ObjectDescriptor ItemObject::get_descriptor() const {
  return IGameState::ObjectDescriptor::ITEM;
}

const IGameState::Item* ItemObject::get_item() const { return item.get(); }

std::optional<int> ItemObject::get_damage() const { return item->get_damage(); }

void ItemObject::apply() {
  auto player = dynamic_cast<Player*>(GameStateObject::state->get_player());
  auto [xp, yp] = player->get_pos();
  if (abs(xp - x) + abs(yp - y) <= 1) {
    auto map = state->get_current_map();
    if (player->put_item(item)) map->remove_object(map->items, this);
  }
}
