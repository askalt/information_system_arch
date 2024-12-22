#pragma once
#include <memory>
#include <optional>
#include <vector>

// Abstract class of game state.
struct IGameState {
  enum class ObjectDescriptor {
    PLAYER,
    WALL,
    CHEST,
    STONE,
    ENTER,
    HORIZONTAL_BORDER,
    VERTICAL_BORDER,
    CORNER,
  };

  struct Object {
    Object(int x, int y) : x{x}, y{y} {}

    bool on_same_pos(const Object* other) {
      auto [xo, yo] = other->get_pos();
      return x == xo && y == yo;
    }

    virtual std::tuple<int, int> get_pos() const { return {x, y}; }

    virtual ObjectDescriptor get_descriptor() const = 0;

    // Get current, max health.
    virtual std::optional<std::tuple<int, int>> get_health() const {
      return std::nullopt;
    }

    // Get optional label, e.g. wall may be labeled as "dungeon".
    virtual std::optional<std::string_view> get_label() const {
      return std::nullopt;
    }
    virtual ~Object() {}

   protected:
    int x, y;
  };

  enum class PlayerMoveEvent { Left, Right, Up, Down };
  struct ApplyActiveItemEvent {};
  struct MoveItemToActiveEvent {};
  struct NoOpEvent {};
  struct EnterEvent {};

  enum class EventType {
    PlayerMove,
    ApplyActiveItem,
    MoveItemToActive,
    NoOp,
    Enter,
  };

  struct Event {
    Event(PlayerMoveEvent event)
        : player_move(std::move(event)), type(EventType::PlayerMove) {}
    Event(ApplyActiveItemEvent event)
        : select_inventory_item(std::move(event)),
          type(EventType::ApplyActiveItem) {}
    Event(MoveItemToActiveEvent event)
        : move_item_to_active(std::move(event)),
          type(EventType::MoveItemToActive) {}
    Event(NoOpEvent event) : no_op(std::move(event)), type(EventType::NoOp) {}
    Event(EnterEvent event) : enter(std::move(event)), type(EventType::Enter) {}

    union {
      PlayerMoveEvent player_move;
      ApplyActiveItemEvent select_inventory_item;
      MoveItemToActiveEvent move_item_to_active;
      NoOpEvent no_op;
      EnterEvent enter;
    };
    EventType type;
  };

  virtual Object* get_player() const = 0;

  virtual const std::vector<Object*>& get_objects() const = 0;

  virtual void apply(const Event& event) = 0;

  virtual ~IGameState() {}
};
