#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "chars.h"
#include "consts.h"
#include "entities.h"
#include "panic.h"
#include "static_objs.h"

struct Map {
  /* Each map contains player. */
  Map(IGameState::Object* player) { objects.push_back(player); }
  Map() {}

  std::vector<std::unique_ptr<Enter>> enters;
  std::vector<std::unique_ptr<DungeonBlock>> dungeon_blocks;
  std::vector<std::unique_ptr<Chest>> chests;
  std::vector<std::unique_ptr<Wall>> walls;
  std::vector<std::unique_ptr<Border>> borders;
  std::unique_ptr<Exit> exit;

  /* All objects that map contains. */
  std::vector<IGameState::Object*> objects;

  template <typename T>
  void push_new_object(std::vector<std::unique_ptr<T>>& container,
                       std::unique_ptr<T> object) {
    objects.push_back(object.get());
    container.push_back(std::move(object));
  }

  void push_player(IGameState::Object* player) { objects.push_back(player); }

  void push_exit(std::unique_ptr<Exit> exit_obj) {
    objects.push_back(exit_obj.get());
    exit = std::move(exit_obj);
  }

  std::tuple<int, int> start_pos() const { return exit->get_pos(); }
};

std::optional<std::string> extractExtension(const std::filesystem::path& p) {
  auto path_str = p.string();
  auto dot_pos = path_str.find_last_of(path_str);
  if (dot_pos == std::string::npos) {
    return std::nullopt;
  }
  return path_str.substr(dot_pos + 1, path_str.size() - dot_pos - 1);
}

std::unique_ptr<Map> load_map(const std::filesystem::path& p) {
  std::fstream in(p);
  std::string s;
  int x = 0;

  auto map = std::make_unique<Map>();
  while (std::getline(in, s)) {
    for (int y = 0; y < s.size(); ++y) {
      char c = s[y];
      if ('A' <= c && c <= 'Z') {
        map->push_new_object(map->enters,
                             std::move(std::make_unique<Enter>(
                                 x, y, LEVEL_0_DUNGEON, std::string{c})));
      } else {
        switch (c) {
          case '|': {
            map->push_new_object(
                map->borders,
                std::move(std::make_unique<Border>(
                    x, y, IGameState::ObjectDescriptor::VERTICAL_BORDER)));
            break;
          }
          case '+': {
            map->push_new_object(
                map->borders, std::move(std::make_unique<Border>(
                                  x, y, IGameState::ObjectDescriptor::CORNER)));
            break;
          }
          case '-': {
            map->push_new_object(
                map->borders,
                std::move(std::make_unique<Border>(
                    x, y, IGameState::ObjectDescriptor::HORIZONTAL_BORDER)));
            break;
          }
          case '@': {
            map->push_new_object(map->chests,
                                 std::move(std::make_unique<Chest>(x, y)));
            break;
          }
          case '*': {
            map->push_new_object(map->dungeon_blocks,
                                 std::move(std::make_unique<DungeonBlock>(
                                     x, y, LEVEL_0_DUNGEON)));
            break;
          }
          case '%': {
            map->push_exit(std::move(std::make_unique<Exit>(x, y)));
            break;
          }
          case ' ': {
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

  return map;
}

struct World {
  std::vector<std::unique_ptr<Map>> maps;
  std::unique_ptr<Player> player;
  Map* start_map;
};

// Loads a world from directory.
World load_world(const std::filesystem::path& p) {
  // Collect all maps by name.
  std::vector<std::unique_ptr<Map>> maps;
  std::unordered_map<std::string, Map*> map_by_name;
  for (auto file : std::filesystem::directory_iterator(p)) {
    if (!file.is_regular_file()) {
      continue;
    }
    if (file.path().extension() == ".rl") {
      maps.push_back(load_map(file.path()));
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

  return World{
      .maps = std::move(maps),
      .player = std::move(player),
      .start_map = start_map,
  };
}
