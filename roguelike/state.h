#pragma once

// abstract class of game state.
struct GameState {
  enum class PlayerMoveEvent { Left, Right, Up, Down };
  // применение предмета из активных
  struct ApplyActiveItemEvent {};
  // перемещение предмета
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

  void apply(const Event &event) {}
};
