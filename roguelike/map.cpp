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
#include "items.h"

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
          case '/': {
            auto item = std::unique_ptr<GameState::Item>(
                            std::move(std::make_unique<Stick>()));
            push_new_object(items, std::move(std::make_unique<ItemObject>(
                                      std::move(item), x, y)));
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
  return std::any_of(objects.begin(), objects.end(),
    [&](IGameState::Object *obj) {
      if (obj != exclude) {
        auto [xo, yo] = obj->get_pos();
        return (xo == x && yo == y);
      }
      return false;
    }
  );
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

std::tuple<int, int> Map::start_pos() const { assert(exit != nullptr); return exit->get_pos(); }
/** */

/* World impl. */

// Loads a world from directory.
World::World(const std::filesystem::path& dir) : dir(dir) {
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
  //auto [x, y] = start_map->start_pos();
  auto player = std::make_unique<Player>(-1, -1, MAX_HEALTH, MAX_HEALTH);

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

  std::vector<plan_node> nodes(n);
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
    nodes[i].edges[comp0][1 - direct1_idx].dst = minmax1.second;
    assert(nodes[minmax1.second].edges[comp0][direct1_idx].dst == -1);
    nodes[minmax1.second].edges[comp0][direct1_idx].dst = i;
    minmax[comp0][0][coord1] =
      std::min(minmax[comp0][0][coord1], std::make_pair(coord0, i));
    minmax[comp0][1][coord1] =
      std::max(minmax[comp0][1][coord1], std::make_pair(coord0, i));
    minmax1 = std::make_pair(coord1, i);
    if ((coord1 - global_minmax1) / direct1 >= 0)
      global_minmax1 = coord1;
  }

  //resolve intersections
  for (int i = 0; i < n; i++) {
    edge &comp0_edge = nodes[i].edges[0][1];
    if (comp0_edge.dst == -1)
      continue;
    for (int j = 0; j < n; j++) {
      edge &comp1_edge = nodes[j].edges[1][1];
      if (comp1_edge.dst == -1)
        continue;
      int y1 = nodes[i].y, y2 = nodes[comp0_edge.dst].y;
      int x1 = nodes[j].x, x2 = nodes[comp1_edge.dst].x;
      int y = nodes[j].y, x = nodes[i].x;
      assert(y == nodes[comp1_edge.dst].y);
      assert(x == nodes[comp0_edge.dst].x);
      assert(x1 < x2);
      assert(y1 < y2);
      //check for intersection
      if ((x1 < x && x < x2) && (y1 < y && y < y2)) {
        comp0_edge.intersections.push_back(j);
        comp1_edge.intersections.push_back(i);
      }
    }
  }

  plan plan(n, std::move(nodes));
  return plan;
}

IGameState::ObjectDescriptor border_type[2] = {
  IGameState::ObjectDescriptor::HORIZONTAL_BORDER,
  IGameState::ObjectDescriptor::VERTICAL_BORDER,
};

void build_box_from_node(Map &mp, plan_node *node, int box_width, int tunnel_width) {
  int x = node->x, y = node->y;
  int l = box_width - tunnel_width - 1;
  int r = box_width + tunnel_width - 1;
  for (int comp0 = 0; comp0 < 2; comp0++) {
    for (int direct0_idx = 0; direct0_idx < 2; direct0_idx++) {
      int comp1 = 1 - comp0;
      int direct0 = 2 * direct0_idx - 1;
      int direct1 = (comp0 ? 1 : -1) * direct0;
      int coords[2] = { x, y };
      coords[comp0] += direct0 * box_width;
      coords[comp1] -= direct1 * box_width;
      mp.push_new_object(mp.borders, std::move(std::make_unique<Border>(
        coords[0], coords[1], IGameState::ObjectDescriptor::CORNER)));
      for (int i = 0; i < 2 * box_width - 1; i++) {
        coords[comp1] += direct1;
        if (node->edges[comp1][direct0_idx].dst == -1 || i < l || r < i)
          mp.push_new_object(mp.borders, std::move(std::make_unique<Border>(
            coords[0], coords[1], border_type[comp0])));
      }
    }
  }
}

void build_tunnels_from_node(
  Map &mp, const plan &plan, int node_idx, int box_width, int tunnel_width) {
  int coord[2];
  auto &node = plan.nodes[node_idx];
  for (int comp0 = 0; comp0 < 2; comp0++) {
    int comp1 = 1 - comp0;
    int direct1_idx = 1;
    //for (int direct1_idx = 0; direct1_idx < 2; direct1_idx++) {
    auto &edge = plan.nodes[node_idx].edges[comp0][direct1_idx];
    int neigh_idx = edge.dst;
    if (neigh_idx == -1)
      continue;
    std::vector<int> intersections(edge.intersections);
    std::transform(edge.intersections.begin(), edge.intersections.end(),
                   intersections.begin(), [&](int idx) {
      return reinterpret_cast<const int *>(&plan.nodes[idx].x)[comp1];
    });
    std::sort(intersections.begin(), intersections.end());
    assert(node_idx != neigh_idx);
    //if (node_idx < neigh_idx) {
    auto &neigh = plan.nodes[neigh_idx];
    int coord0 = reinterpret_cast<const int *>(&node.x)[comp0];
    assert(coord0 ==
      reinterpret_cast<const int *>(&neigh.x)[comp0]);
    int direct1 = 2 * direct1_idx - 1;
    int from1 = reinterpret_cast<const int *>(&node.x)[comp1] +
      direct1 * (box_width);
    int to1 = reinterpret_cast<const int *>(&neigh.x)[comp1] -
      direct1 * (box_width);
    //assert((to1 - from1) / direct1 >= 0);
    assert(from1 <= to1);
    assert(direct1 == 1);
    coord[comp1] = from1;

    auto actual = intersections.begin();
    for (int coord1 = from1; coord1 <= to1; coord1 += direct1) {
      int l = -1, r = -1;
      if (actual != intersections.end()) {
        l = *actual - tunnel_width;
        r = *actual + tunnel_width;
      }
      IGameState::ObjectDescriptor type = border_type[comp0];
      if (l == coord1) {
        type = IGameState::ObjectDescriptor::CORNER;
      } else if (r == coord1) {
        type = IGameState::ObjectDescriptor::CORNER;
        actual++;
      }
      if (coord1 <= l || r <= coord1) {
        //IGameState::ObjectDescriptor type = border_type[comp0];
        if (coord1 == from1 || coord1 == to1)
          type = IGameState::ObjectDescriptor::CORNER;
        coord[comp0] = coord0 - tunnel_width;
        mp.push_new_object(mp.borders, std::move(
          std::make_unique<Border>(coord[0], coord[1], type)));
        coord[comp0] = coord0 + tunnel_width;
        mp.push_new_object(mp.borders, std::move(
          std::make_unique<Border>(coord[0], coord[1], type)));
      }
      coord[comp1] += direct1;
    }
    //}
    //}
  }
}

std::unique_ptr<Map> gen_map(int n) {
  auto plan = gen_plan(n);
  const int min_tunnel_length = 3;
  const int box_width = 5;
  const int tunnel_width = 2;
  const int factor = min_tunnel_length + 2 * box_width + 1;
  const int fixed_offset = 10;
  auto start_node = &plan.nodes[0];
  for (auto &node : plan.nodes) {
    if (node.y < start_node->y) {
      start_node = &node;
    } else if (node.y == start_node->y) {
      if (node.x < start_node->x)
        start_node = &node;
    }
  }
  int x_offset = start_node->x, y_offset = start_node->y;
  for (auto &node : plan.nodes) {
    node.x = (node.x - x_offset) * factor + fixed_offset;
    node.y = (node.y - y_offset) * factor + fixed_offset;
  }
  auto mp = std::make_unique<Map>();
  for (int i = 0; i < n; i++) {
    build_box_from_node(*mp, &plan.nodes[i], box_width, tunnel_width);
    build_tunnels_from_node(*mp, plan, i, box_width, tunnel_width);
  }
  mp->push_exit(std::move(
    std::make_unique<Exit>(start_node->x, start_node->y)));

  //spawn mobs
  std::random_device rd;
  std::mt19937 gen(rd());
  std::vector<int> p(3, 1);
  std::discrete_distribution<> d(p.begin(), p.end());
  for (auto &node : plan.nodes) {
    if (&node == start_node)
      continue;

    switch (d(gen)) {
      case 0:
        break;
      case 1: {
        if ((unsigned long) gen() % 2 == 0)
          mp->push_new_object(mp->mobs, std::move(std::unique_ptr<Mob>(
            std::move(std::make_unique<Orc>(node.x, node.y)))));
        else
          mp->push_new_object(mp->mobs, std::move(std::unique_ptr<Mob>(
            std::move(std::make_unique<Bat>(node.x, node.y)))));
        break;
      } case 2: {
        auto item = std::unique_ptr<GameState::Item>(
          std::move(std::make_unique<Stick>()));
        mp->push_new_object(mp->items, std::move(std::make_unique<ItemObject>(
          std::move(item), node.x, node.y)));
        break;
      }
    }
  }
  return mp;
}

/***/
