
#include <cassert>

#include "entities.h"
#include "map.h"

/* GameState impl. */
GameState::GameState(std::unique_ptr<World> world) : world{std::move(world)} {
  for (const auto& map : this->world->maps) {
    for (const auto object : map->objects) {
      auto as_state_object = dynamic_cast<GameStateObject*>(object);
      assert(as_state_object && "world contains specific objects");
      as_state_object->set_state(this);
    }
  }
  map_stack.push_back(
      MapStackNode{.x = -1, .y = -1, .map = this->world->start_map});
}

IGameState::Object* GameState::get_player() const {
  return world->player.get();
}

void GameState::apply_event(const Event& event) {
  switch (event.type) {
    case EventType::PlayerMove:
      player_move(event.player_move);
      break;
    case EventType::Apply:
      apply(event.apply);
      break;
    default:
      break;
  }
}

void GameState::player_move(const PlayerMoveEvent& event) {
  auto [x, y] = world->player->get_pos();
  apply_move(x, y, event);
  auto current_map = map_stack.back().map;
  /* Check on intersection, for now only with static objects. */
  for (const auto obj : current_map->objects) {
    if (obj != world->player.get()) {
      auto [xo, yo] = obj->get_pos();
      if (xo == x && yo == y) {
        return;
      }
    }
  }
  world->player->move(event);
}

const std::vector<IGameState::Object*>& GameState::get_objects() const {
  return map_stack.back().map->objects;
}

void GameState::move_on(Map* map) {
  auto [x, y] = world->player->get_pos();
  map_stack.push_back(MapStackNode{.x = x, .y = y, .map = map});
  auto [sx, sy] = map->start_pos();
  world->player->set_pos(sx, sy);
}

void GameState::move_back() {
  if (map_stack.size() > 1) {
    world->player->set_pos(map_stack.back().x, map_stack.back().y);
    map_stack.pop_back();
  }
}

void GameState::apply(const ApplyEvent& e) {
  auto obj = dynamic_cast<GameStateObject*>(e.object);
  if (obj == nullptr) {
    /* Do nothing. */
    return;
  }
  obj->apply();
}

/***/

/* GameStateObject impl. */
GameStateObject::GameStateObject() {}

void GameStateObject::set_state(GameState* state) { this->state = state; }
