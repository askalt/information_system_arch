#include "entities.h"
#include "static_objs.h"

struct Map {
  /* Each map contains player. */
  Map(IGameState::Object* player) { objects.push_back(player); }
  std::vector<std::unique_ptr<Enter>> enters;
  std::vector<std::unique_ptr<DungeonBlock>> dungeon_blocks;
  std::vector<std::unique_ptr<Chest>> chests;
  std::vector<std::unique_ptr<Wall>> walls;
  std::vector<std::unique_ptr<Border>> borders;

  /* All objects that map contains. */
  std::vector<IGameState::Object*> objects;

  template <typename T>
  void push_new_object(std::vector<std::unique_ptr<T>>& container,
                       std::unique_ptr<T> object) {
    objects.push_back(object.get());
    container.push_back(std::move(object));
  }
};
