

#include <cassert>

#include "entities.h"
#include "map.h"

/* GameState impl. */
GameState::GameState(std::unique_ptr<World> world) : world{std::move(world)} {
  for (const auto& map : this->world->maps)
    map_init(map.get());
  move_on(this->world->start_map);
  //map_stack.push_back(
  //    MapStackNode{.x = -1, .y = -1, .map = this->world->start_map});
}

void GameState::map_init(Map *map) {
    for (const auto object : map->objects) {
      auto as_state_object = dynamic_cast<GameStateObject*>(object);
      assert(as_state_object && "world contains specific objects");
      as_state_object->set_state(this);
    }
}

IGameState::IPlayer* GameState::get_player() const {
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
      auto player = dynamic_cast<Player*>(get_player());
      auto item_from_stash = std::move(player->take_item(event.apply_item.pos));
      assert(item_from_stash != nullptr);
      auto item_from_hand = std::unique_ptr<Item>(std::move(player->hand));
      player->hand = std::unique_ptr<Stick>{
          dynamic_cast<Stick*>(item_from_stash.release())};
      if (item_from_hand != nullptr) {
        bool ok = player->put_item(item_from_hand);
        assert(ok);
      }
      break;
    }
    default:
      break;
  }
  for (const auto& mob : get_current_map()->mobs) {
    mob->move();
  }
}

Map* GameState::get_current_map() const { return map_stack.back().map; }

void GameState::player_move(const PlayerMoveEvent& event) {
  world->player->move(event);
  for (const auto& mob : get_current_map()->mobs) {
    mob->move();
  }
}

const int MAX_LEVEL = 5;

bool GameState::is_win() const { return world->player->get_lvl() == MAX_LEVEL; }

void GameState::damage_player(int dmg) { world->player->damage(dmg); }

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

  //generate maps
  for (auto &enter : map->enters) {
    if (enter->get_map() == nullptr) {
      auto label = enter->get_label();
      auto filename = std::string{label->begin(), label->end()};
      filename += ".rl";
      auto file = world->dir / filename;
      if (!std::filesystem::exists(file)) {
        auto generated_map = gen_map(15);
        generated_map->push_player(world->player.get());
        map_init(generated_map.get());
        enter->set_map(generated_map.get());
        world->maps.push_back(std::move(generated_map));
      }
    }
  }
}

void GameState::move_back() {
  if (map_stack.size() > 1) {
    world->player->set_pos(map_stack.back().x, map_stack.back().y);
    map_stack.pop_back();
  }
}

void GameState::apply(const ApplyObjectEvent& e) {
  auto object = dynamic_cast<GameStateObject*>(e.object);
  if (object != nullptr) object->apply();
}

/***/

void GameStateObject::set_state(GameState* state) { this->state = state; }

GameState* GameStateObject::get_state() const { return state; }
