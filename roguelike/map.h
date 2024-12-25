#pragma once
#include <algorithm>
#include <filesystem>
#include <vector>
#include <set>

#include "objects.h"
#include "panic.h"
#include "state.h"

struct Map {
  friend class GameState;
  friend class Player;
  friend class Mob;

  /* Each map contains player. */
  Map(IGameState::Object* player);
  Map();
  Map(const std::filesystem::path& path);
  friend class World;

  bool has_object(int x, int y, const IGameState::Object* exclude) const;
  std::set<std::pair<int, int>> get_obstacles() const;

  template <typename T>
  bool remove_object(std::vector<std::unique_ptr<T>>& container, T* item) {
    size_t pos = std::string::npos;
    for (size_t i = 0; i < container.size(); ++i) {
      if (container[i].get() == item) {
        pos = i;
        break;
      }
    }
    if (pos != std::string::npos) {
      container.erase(container.begin() + pos);
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

 private:
  std::vector<std::unique_ptr<Enter>> enters;
  std::vector<std::unique_ptr<DungeonBlock>> dungeon_blocks;
  std::vector<std::unique_ptr<Chest>> chests;
  std::vector<std::unique_ptr<Wall>> walls;
  std::vector<std::unique_ptr<Border>> borders;
  std::vector<std::unique_ptr<Mob>> mobs;
  std::unique_ptr<Exit> exit;
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

  World(const std::filesystem::path& dir);

 private:
  std::vector<std::unique_ptr<Map>> maps;
  std::unique_ptr<Player> player;
  Map* start_map;
};
