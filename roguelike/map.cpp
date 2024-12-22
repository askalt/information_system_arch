#include "map.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <random>

#include "consts.h"
#include "entities.h"
#include "objects.h"
#include "panic.h"

/* Map impl. */
Map::Map(IGameState::Object* player) { objects.push_back(player); }

Map::Map() {}

Map::Map(const std::filesystem::path& p) {
  std::fstream in(p);
  std::string s;
  int x = 0;

  name = p.stem();
  while (std::getline(in, s)) {
    for (int y = 0; y < s.size(); ++y) {
      char c = s[y];
      if ('A' <= c && c <= 'Z') {
        push_new_object(enters, std::move(std::make_unique<Enter>(
                                    x, y, LEVEL_0_DUNGEON, std::string{c})));
      } else {
        switch (c) {
          case '|': {
            push_new_object(
                borders,
                std::move(std::make_unique<Border>(
                    x, y, IGameState::ObjectDescriptor::VERTICAL_BORDER)));
            break;
          }
          case '+': {
            push_new_object(borders,
                            std::move(std::make_unique<Border>(
                                x, y, IGameState::ObjectDescriptor::CORNER)));
            break;
          }
          case '-': {
            push_new_object(
                borders,
                std::move(std::make_unique<Border>(
                    x, y, IGameState::ObjectDescriptor::HORIZONTAL_BORDER)));
            break;
          }
          case '@': {
            push_new_object(chests, std::move(std::make_unique<Chest>(x, y)));
            break;
          }
          case '*': {
            push_new_object(dungeon_blocks,
                            std::move(std::make_unique<DungeonBlock>(
                                x, y, LEVEL_0_DUNGEON)));
            break;
          }
          case '%': {
            push_exit(std::move(std::make_unique<Exit>(x, y)));
            break;
          }
          case ' ': {
            break;
          }
          case '$': {
            push_new_object(mobs, std::move(std::unique_ptr<Mob>(
                                      std::move(std::make_unique<Orc>(x, y)))));
            break;
          }
          case '&': {
            push_new_object(mobs, std::move(std::unique_ptr<Mob>(
                                      std::move(std::make_unique<Bat>(x, y)))));
            break;
          }
          default: {
            panic("unexpected symbol");
          }
        }
      }
    }
    ++x;
  }
}

void Map::push_player(IGameState::Object* player) { objects.push_back(player); }

void Map::push_exit(std::unique_ptr<Exit> exit_obj) {
  objects.push_back(exit_obj.get());
  exit = std::move(exit_obj);
}

bool Map::has_object(int x, int y, const IGameState::Object* exclude) const {
  for (auto obj : objects) {
    if (obj != exclude) {
      auto [xo, yo] = obj->get_pos();
      if (xo == x && yo == y) {
        return true;
      }
    }
  }
  return false;
}

std::set<std::pair<int, int>> Map::get_obstacles() const {
  std::set<std::pair<int, int>> obstacles;

#define run_over(objs)            \
  for (const auto& obj : objs) {  \
    auto [x, y] = obj->get_pos(); \
    obstacles.insert({x, y});     \
  }

  run_over(walls);
  run_over(dungeon_blocks);
  run_over(borders);
  run_over(chests);

  return obstacles;
}

std::tuple<int, int> Map::start_pos() const { return exit->get_pos(); }
/** */

/* World impl. */

// Loads a world from directory.
World::World(const std::filesystem::path& dir) {
  // Collect all maps by name.
  std::vector<std::unique_ptr<Map>> maps;
  std::unordered_map<std::string, Map*> map_by_name;
  for (auto file : std::filesystem::directory_iterator(dir)) {
    if (!file.is_regular_file()) {
      continue;
    }
    if (file.path().extension() == ".rl") {
      maps.push_back(std::make_unique<Map>(file.path()));
      map_by_name.insert({file.path().stem().string(), maps.back().get()});
    }
  }

  auto start_map_it = map_by_name.find(STARTING_MAP);
  if (start_map_it == map_by_name.end()) {
    panic("starting map is not found");
  }
  auto start_map = start_map_it->second;
  auto [x, y] = start_map->start_pos();
  auto player = std::make_unique<Player>(x, y, MAX_HEALTH, MAX_HEALTH);

  // Fill transitions.
  for (const auto& mp : maps) {
    mp->push_player(player.get());
    for (auto& enter : mp->enters) {
      if (auto it = map_by_name.find(enter->get_transition());
          it != map_by_name.end()) {
        enter->set_map(it->second);
      }
    }
  }

  this->maps = std::move(maps);
  this->player = std::move(player);
  this->start_map = start_map;
}

int inline gen_int(std::mt19937 &gen, int l, int r) {
  return l + (int)((unsigned long)gen() % (r - l + 1));
}

plan gen_plan(int n) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::vector<int> p(2 * n - 1, 1);
  for (int i = 2; i < 2 * n - 1; i++)
    p[i] = p[i - 1] << 1;

  std::discrete_distribution<> d(p.begin(), p.end());

  plan nodes(n);
  nodes[0] = plan_node(0, 0);

  const int shift = n - 1;
  int global_minmax[2][2] = {{shift, shift}, {shift, shift}};
  std::vector<std::pair<int, int>> minmax[2][2];
  for (auto &comp : minmax) {
    comp[0].assign(2 * n - 1, std::make_pair(INT32_MAX, 0));
    comp[1].assign(2 * n - 1, std::make_pair(INT32_MIN, 0));
    comp[0][shift] = std::make_pair(shift, 0);
    comp[1][shift] = std::make_pair(shift, 0);
  }
  int coord[2];
  for (int i = 1; i < n; i++) {
    int comp0 = gen_int(gen, 0, 1), comp1 = 1 - comp0;
    int coord0 = gen_int(gen,
      global_minmax[comp0][0], global_minmax[comp0][1]);
    size_t direct1_idx = gen_int(gen, 0, 1);
    int direct1 = 2 * (int)direct1_idx - 1;
    auto &minmax1 = minmax[comp1][direct1_idx][coord0];
    auto &global_minmax1 = global_minmax[comp1][direct1_idx];
    int max_delta = std::abs(global_minmax1 - minmax1.first);
    int delta1 = std::min(d(gen), max_delta) + 1;
    int coord1 = minmax1.first + direct1 * delta1;
    coord[comp0] = coord0 - shift;
    coord[comp1] = coord1 - shift;
    nodes[i] = plan_node(coord[0], coord[1]);
    nodes[i].edges[comp0][1 - direct1_idx] = minmax1.second;
    assert(nodes[minmax1.second].edges[comp0][direct1_idx] == -1);
    nodes[minmax1.second].edges[comp0][direct1_idx] = i;
    minmax[comp0][0][coord1] =
      std::min(minmax[comp0][0][coord1], std::make_pair(coord0, i));
    minmax[comp0][1][coord1] =
      std::max(minmax[comp0][1][coord1], std::make_pair(coord0, i));
    minmax1 = std::make_pair(coord1, i);
    if ((coord1 - global_minmax1) / direct1 >= 0)
      global_minmax1 = coord1;
  }
  return nodes;
}

void build_box_from_node(Map &mp, plan_node *node, int box_width, int tunnel_width) {
  int x = node->x, y = node->y;
  for (int i = 0; i < 2 * box_width + 1; i++)
    if (i <= box_width - tunnel_width || i >= box_width + tunnel_width || node->edges[0][0] == -1)
      mp.push_new_object(mp.walls, std::move(
        std::make_unique<Wall>(x - box_width + i, y - box_width)));
  for (int i = 0; i < 2 * box_width + 1; i++)
    if (i <= box_width - tunnel_width || i >= box_width + tunnel_width || node->edges[1][1] == -1)
      mp.push_new_object(mp.walls, std::move(
        std::make_unique<Wall>(x + box_width, y - box_width + i)));
  for (int i = 0; i < 2 * box_width + 1; i++)
    if (i <= box_width - tunnel_width || i >= box_width + tunnel_width || node->edges[0][1] == -1)
      mp.push_new_object(mp.walls, std::move(
        std::make_unique<Wall>(x + box_width - i, y + box_width)));
  for (int i = 0; i < 2 * box_width + 1; i++)
    if (i <= box_width - tunnel_width || i >= box_width + tunnel_width || node->edges[1][0] == -1)
      mp.push_new_object(mp.walls, std::move(
        std::make_unique<Wall>(x - box_width, y + box_width - i)));
}

void build_tunnels_from_node(
  Map &mp, const plan &plan, int node_idx, int box_width, int tunnel_width) {
  int coord[2];
  auto &node = plan[node_idx];
  for (int comp0 = 0; comp0 < 2; comp0++) {
    for (int direct1_idx = 0; direct1_idx < 2; direct1_idx++) {
      int neigh_idx = plan[node_idx].edges[comp0][direct1_idx];
      assert(node_idx != neigh_idx);
      if (node_idx < neigh_idx) {
        auto &neigh = plan[neigh_idx];
        int comp1 = 1 - comp0;
        int coord0 = reinterpret_cast<const int *>(&node.x)[comp0];
        assert(coord0 ==
          reinterpret_cast<const int *>(&neigh.x)[comp0]);
        int direct1 = 2 * direct1_idx - 1;
        int from1 = reinterpret_cast<const int *>(&node.x)[comp1] +
          direct1 * (box_width);
        int to1 = reinterpret_cast<const int *>(&neigh.x)[comp1] -
          direct1 * (box_width);
        assert((to1 - from1) / direct1 >= 0);
        for (int coord1 = from1; coord1 != to1; coord1 += direct1) {
          coord[comp1] = coord1;
          coord[comp0] = coord0 - tunnel_width;
          mp.push_new_object(mp.walls, std::move(
            std::make_unique<Wall>(coord[0], coord[1])));
          coord[comp0] = coord0 + tunnel_width;
          mp.push_new_object(mp.walls, std::move(
            std::make_unique<Wall>(coord[0], coord[1])));
        }
      }
    }
  }
}

std::unique_ptr<Map> gen_map(int n) {
  auto plan = gen_plan(n);
  const int min_tunnel_length = 3;
  const int box_width = 5;
  const int tunnel_width = 2;
  const int factor = min_tunnel_length + 2 * box_width + 1;
  for (auto &node : plan) {
    node.x *= factor;
    node.y *= factor;
  }
  auto mp = std::make_unique<Map>();
  for (int i = 0; i < n; i++) {
    build_box_from_node(*mp, &plan[i], box_width, tunnel_width);
    build_tunnels_from_node(*mp, plan, i, box_width, tunnel_width);
  }
  return mp;
}

std::unique_ptr<World> gen_world(int n) {
  std::vector<std::unique_ptr<Map>> maps;
  maps.push_back(std::move(gen_map(n)));
  auto start_map = maps.back().get();
  auto player = std::make_unique<Player>(0, 0, MAX_HEALTH, MAX_HEALTH);
  start_map->push_player(player.get());
  auto world = std::make_unique<World>();
  world->maps = std::move(maps);
  world->player = std::move(player);
  world->start_map = start_map;
  return world;
}

/***/
