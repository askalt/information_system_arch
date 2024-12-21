
#include <memory>
#include <vector>

// Abstract class of game state.
struct IGameState {
  enum class PartDescriptor {
    PLAYER,
    WALL,
  };

  struct ObjectPart {
    PartDescriptor descriptor;
    int x;
    int y;
  };

  struct Object {
    virtual const std::vector<ObjectPart>& get_layout() = 0;
  };

  enum class PlayerMoveEvent { Left, Right, Up, Down };
  struct ApplyActiveItemEvent {};
  struct MoveItemToActiveEvent {};

  enum class EventType {
    PlayerMove,
    ApplyActiveItem,
    MoveItemToActive,
  };

  struct Event {
    union {
      PlayerMoveEvent player_move;
      ApplyActiveItemEvent select_inventory_item;
      MoveItemToActiveEvent move_item_to_active;
    };
    EventType type;
  };

  virtual const std::vector<Object*>& get_objects() const = 0;

  virtual void apply(const Event& event) = 0;

  virtual ~IGameState() {}
};
