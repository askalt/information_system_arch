
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
  };

  struct Object {
    Object(int x, int y) : x{x}, y{y} {}

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

  enum class EventType {
    PlayerMove,
    ApplyActiveItem,
    MoveItemToActive,
    NoOp,
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
    Event(NoOpEvent event)
        : no_op_event(std::move(no_op_event)), type(EventType::NoOp) {}

    union {
      PlayerMoveEvent player_move;
      ApplyActiveItemEvent select_inventory_item;
      MoveItemToActiveEvent move_item_to_active;
      NoOpEvent no_op_event;
    };
    EventType type;
  };

  virtual Object* get_player() const = 0;

  virtual const std::vector<Object*>& get_objects() const = 0;

  virtual void apply(const Event& event) = 0;

  virtual ~IGameState() {}
};
