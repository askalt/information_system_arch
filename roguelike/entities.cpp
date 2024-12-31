#include "entities.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

/* IGameState impl. */
std::tuple<int, int> IGameState::Object::get_pos() const { return {x, y}; }

bool IGameState::Object::on_same_pos(const IGameState::Object* other) const {
  auto [xo, yo] = other->get_pos();
  return x == xo && y == yo;
}

IGameState::Object::Object(int x, int y) : x{x}, y{y} {}

void IGameState::Object::set_pos(int xx, int yy) {
  x = xx;
  y = yy;
}

std::optional<std::string_view> IGameState::Object::get_label() const {
  return std::nullopt;
}

/* Event impl. */
IGameState::Event::Event(IGameState::PlayerMoveEvent event)
    : player_move(std::move(event)), type(EventType::PlayerMove) {}
IGameState::Event::Event(NoOpEvent event)
    : no_op(std::move(event)), type(EventType::NoOp) {}
IGameState::Event::Event(ApplyObjectEvent event)
    : apply_object(std::move(event)), type(EventType::Apply) {}
IGameState::Event::Event(ApplyItemEvent event)
  : apply_item(std::move(event)), type(EventType::ApplyItem) {}

/* IEnter impl. */
IGameState::IEnter::IEnter(int x, int y, std::string transition)
    : Object{x, y}, transition{std::move(transition)} {};

const std::string& IGameState::IEnter::get_transition() { return transition; }

void apply_move(int& x, int& y, const IGameState::PlayerMoveEvent& event) {
  switch (event) {
    case IGameState::PlayerMoveEvent::Down: {
      x += 1;
      break;
    }
    case IGameState::PlayerMoveEvent::Up: {
      x -= 1;
      break;
    }
    case IGameState::PlayerMoveEvent::Left: {
      y -= 1;
      break;
    }
    case IGameState::PlayerMoveEvent::Right: {
      y += 1;
      break;
    }
  }
}

/* IHealtable impl. */
IGameState::IHealthable::IHealthable(int x, int y) : Object{x, y} {}

/* IPlayer impl. */
IGameState::IPlayer::IPlayer(int x, int y) : IGameState::IHealthable{x, y} {}

/* IMob impl. */
IGameState::IMob::IMob(int x, int y) : IGameState::IHealthable{x, y} {}

IGameState::ItemDescriptor IGameState::Item::get_descriptor() const { return descriptor; }

