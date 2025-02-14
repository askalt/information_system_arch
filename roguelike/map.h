#pragma once
#include <algorithm>
#include <filesystem>
#include <utility>
#include <vector>
#include <set>

#include "objects.h"
#include "panic.h"
#include "state.h"

struct edge {
    int dst;
    std::vector<int> intersections;
};

struct plan_node {
    plan_node() = default;

    plan_node(int x, int y) : x(x), y(y) {
      edges[0][0].dst = edges[0][1].dst =
        edges[1][0].dst = edges[1][1].dst = -1;
    }
    int x{}, y{};
    edge edges[2][2]{};
};

struct plan {
    plan(int n, std::vector<plan_node> nodes) : n(n), nodes(std::move(nodes)) {}

    int n;
    std::vector<plan_node> nodes;
};

struct Map {
  friend class GameState;
  friend class Player;
  friend class Mob;
  friend class ItemObject;

  /* Each map contains player. */
  Map(IGameState::Object* player);
  Map();
  Map(const std::filesystem::path& path);
  friend class World;

  bool has_object(int x, int y, const IGameState::Object* exclude) const;
  std::set<std::pair<int, int>> get_obstacles() const;

  template <typename T>
  bool remove_object(std::vector<std::unique_ptr<T>> &container, T *item) {
    auto container_it = std::find_if(container.begin(), container.end(),
      [&](const std::unique_ptr<T> &ptr) { return ptr.get() == item; });

    if (container_it != container.end()) {
      container.erase(container_it);
      auto as_obj = static_cast<IGameState::Object*>(item);
      auto it = std::find(objects.begin(), objects.end(), as_obj);
      if (it == objects.end()) {
        panic("there must be object");
      }
      objects.erase(it);
      return true;
    }
    return false;
  }

  friend void build_box_from_node(
    Map &mp, plan_node *node, int box_width, int tunnel_width);
  friend void build_tunnels_from_node(
    Map &mp, const plan &plan, int node_idx, int box_width, int tunnel_width);
  friend std::unique_ptr<Map> gen_map(int n);
  friend std::unique_ptr<World> gen_world(int n);

 private:
  std::vector<std::unique_ptr<Enter>> enters;
  std::vector<std::unique_ptr<DungeonBlock>> dungeon_blocks;
  std::vector<std::unique_ptr<Chest>> chests;
  std::vector<std::unique_ptr<Wall>> walls;
  std::vector<std::unique_ptr<Border>> borders;
  std::vector<std::unique_ptr<Mob>> mobs;
  std::vector<std::unique_ptr<ItemObject>> items;

  std::unique_ptr<Exit> exit = nullptr;
  std::string name;

  /* All objects that map contains. */
  std::vector<IGameState::Object*> objects;

  template <typename T>
  void push_new_object(std::vector<std::unique_ptr<T>>& container,
                       std::unique_ptr<T> object) {
    objects.push_back(object.get());
    container.push_back(std::move(object));
  }

  void push_player(IGameState::Object* player);
  void push_exit(std::unique_ptr<Exit> exit_obj);
  std::tuple<int, int> start_pos() const;
};

struct World {
  friend class GameState;

  World() = default;

  World(const std::filesystem::path& dir);

  //friend std::unique_ptr<World> gen_world(int n);

  private:
    const std::filesystem::path dir;
    std::vector<std::unique_ptr<Map>> maps;
    std::unique_ptr<Player> player;
    Map* start_map;
};

std::unique_ptr<Map> gen_map(int n);