#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>
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
    EXIT,
    STICK,
    SALVE,
    ORC,
    BAT,
  };

  struct Object {
    Object(int x, int y);

    bool on_same_pos(const Object* other) const;

    virtual std::tuple<int, int> get_pos() const;

    virtual ObjectDescriptor get_descriptor() const = 0;

    // Get current, max health.
    virtual std::optional<std::tuple<int, int>> get_health() const;

    // Get optional label, e.g. wall may be labeled as "dungeon".
    virtual std::optional<std::string_view> get_label() const;

    virtual ~Object() {}

   protected:
    int x, y;
  };

  struct MapDescription {
    const std::string_view name;
    const std ::vector<Object*>& objects;
  };

  struct EnterObj : Object {
    EnterObj(int x, int y, std::string transition);

    const std::string& get_transition();

   protected:
    std::string transition;
  };

  enum class PlayerMoveEvent { Left, Right, Up, Down };
  struct NoOpEvent {};
  struct ApplyEvent {
    // Object to apply.
    Object* object;
  };

  enum class EventType {
    PlayerMove,
    NoOp,
    Apply,
  };

  struct Event {
    Event(PlayerMoveEvent event);
    Event(NoOpEvent event);
    Event(ApplyEvent event);

    union {
      PlayerMoveEvent player_move;
      NoOpEvent no_op;
      ApplyEvent apply;
    };
    EventType type;
  };

  virtual Object* get_player() const = 0;

  virtual const MapDescription get_map() const = 0;

  virtual void apply_event(const Event& event) = 0;

  virtual ~IGameState();
};

void apply_move(int& x, int& y, const IGameState::PlayerMoveEvent& event);
