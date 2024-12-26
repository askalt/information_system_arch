

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
      apply(event.apply_object);
      break;
    case EventType::ApplyItem: {
      auto player = dynamic_cast<Player *>(get_player());
      auto item_from_stash =
        std::move(player->take_item(event.apply_item.pos));
      assert(item_from_stash != nullptr);
      auto item_from_hand =
        std::unique_ptr<Item>(std::move(player->hand));
      player->hand = std::unique_ptr<Stick>{dynamic_cast<Stick *>(item_from_stash.release())};
      if (item_from_hand != nullptr) {
        bool ok = player->put_item(item_from_hand);
        assert(ok);
      }
      break;
    }
    default:
      break;
  }
}

Map* GameState::get_current_map() const { return map_stack.back().map; }

void GameState::player_move(const PlayerMoveEvent& event) {
  world->player->move(event);
  for (const auto& mob : get_current_map()->mobs) {
    mob->move();
  }
}

const IGameState::MapDescription GameState::get_map() const {
  auto map = map_stack.back().map;
  return IGameState::MapDescription{
      .name = map->name,
      .objects = map->objects,
  };
}

void GameState::move_on(Map* map) {
  assert(map != nullptr);
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

void GameState::apply(const ApplyObjectEvent& e) {
  auto object = dynamic_cast<GameStateObject *>(e.object);
  if (object != nullptr)
    object->apply();
}

/***/

void GameStateObject::set_state(GameState* state) { this->state = state; }
