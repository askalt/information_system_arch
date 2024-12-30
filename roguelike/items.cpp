#include "items.h"

#include "objects.h"

Salve::Salve(int heal)
    : IGameState::Item{IGameState::ItemDescriptor::SALVE}, heal{heal} {}

void Salve::apply(IGameState::Object *object) const {
  /*if (auto player = dynamic_cast<Player *>(object)) {
    player->heal(heal);
  }*/
}

Stick::Stick()
    : IGameState::Item{IGameState::ItemDescriptor::STICK},
      damage{2},
      radius{2} {}

Stick::Stick(IGameState::ItemDescriptor item_descriptor, int damage, int radius)
    : IGameState::Item{item_descriptor}, damage{damage}, radius{radius} {}

void Stick::apply(IGameState::Object *object) const {
  /*auto [x, y] = GameStateObject::state->get_player()->get_pos();
  if (auto mob = dynamic_cast<Mob *>(object)) {
    auto [mx, my] = mob->get_pos();
    if (abs(mx - x) + abs(y - my) <= radius) {
      mob->damage(damage);
    }
  }*/
}

std::optional<int> Stick::get_damage() const { return damage; }
