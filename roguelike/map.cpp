
#include "map.h"

#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "consts.h"
#include "entities.h"
#include "panic.h"

/* Map impl. */
Map::Map(IGameState::Object* player) { objects.push_back(player); }

Map::Map() {}

Map::Map(const std::filesystem::path& p) {
  std::fstream in(p);
  std::string s;
  int x = 0;

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

/***/
