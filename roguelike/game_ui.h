#pragma once

#include <locale.h>

#include <algorithm>
#include <sstream>
#include <thread>
#include <unordered_map>

#include "curses.h"
#include "event.h"
#include "panic.h"
#include "objects.h"

#define safe_call(f) \
  if (f() == ERR) {  \
    panic(#f);       \
  }

/* Console sizes. */
const int W = 200;
const int H = 200;

/* Game field sizes. */
const int W_FIELD = 70;
const int H_FIELD = 40;

void init_UI(int argc, char *argv[]) {
  setlocale(LC_ALL, "");
#ifdef XCURSES
  Xinitscr(argc, argv);
#else
  initscr();
#endif
  if (has_colors() == FALSE) {
    endwin();
    printf("Your terminal does not support color\n");
    std::terminate();
  }
  keypad(stdscr, true);
  start_color();
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(3, COLOR_RED, COLOR_BLACK);
  init_pair(4, COLOR_YELLOW, COLOR_BLACK);
  noecho();
  resize_term(H, W);
}

#define CHAR_MAP_MEMBER(s, v, ...) {s, v}
#define INFO_MAP_MEMBER(s, _, v, ...) {s, v}
#define MAP_INIT_LIST(map_members, accessor) { map_members(accessor) }

#define OBJECT_DESCRIPTOR_LIST(_) \
  _(IGameState::ObjectDescriptor::PLAYER, 'p', "player"),\
  _(IGameState::ObjectDescriptor::WALL, '*', "wall"),\
  _(IGameState::ObjectDescriptor::CHEST, '@', "chest"),\
  _(IGameState::ObjectDescriptor::STONE, '#', "stone"),\
  _(IGameState::ObjectDescriptor::ENTER, '\0', "enter"),\
  _(IGameState::ObjectDescriptor::HORIZONTAL_BORDER, '-', "nil"),\
  _(IGameState::ObjectDescriptor::VERTICAL_BORDER, '|', "nil"),\
  _(IGameState::ObjectDescriptor::CORNER, '+', "nil"),\
  _(IGameState::ObjectDescriptor::EXIT, '%', "exit"),\
  _(IGameState::ObjectDescriptor::ORC, 'O', "orc"),\
  _(IGameState::ObjectDescriptor::BAT, 'B', "bat"),\

const std::unordered_map<IGameState::ObjectDescriptor, char>
  OBJECT_DESCRIPTOR_CHAR = MAP_INIT_LIST(OBJECT_DESCRIPTOR_LIST, CHAR_MAP_MEMBER);

const std::unordered_map<IGameState::ObjectDescriptor, std::string>
  OBJECT_DESCRIPTOR_INFO = MAP_INIT_LIST(OBJECT_DESCRIPTOR_LIST, INFO_MAP_MEMBER);

#define ITEM_DESCRIPTOR_LIST(_) \
  _(IGameState::ItemDescriptor::SALVE, '&', "salve"),\
  _(IGameState::ItemDescriptor::STICK, '/', "stick"),\

const std::unordered_map<IGameState::ItemDescriptor, char>
  ITEM_DESCRIPTOR_CHAR = MAP_INIT_LIST(ITEM_DESCRIPTOR_LIST, CHAR_MAP_MEMBER);

const std::unordered_map<IGameState::ItemDescriptor, std::string>
  ITEM_DESCRIPTOR_INFO = MAP_INIT_LIST(ITEM_DESCRIPTOR_LIST, INFO_MAP_MEMBER);

int rem(int a, int mod) {
  if (a > 0) {
    return a % mod;
  }
  return (mod - ((-a) % mod)) % mod;
}

std::pair<int, int> get_bounds(int x, int width) {
  if (x >= 0) {
    int div = x / width;
    return {div * width, (div + 1) * width};
  }
  int div = abs(x) / width;
  return {-(div + 1) * width, -div * width};
}

std::string make_item_info(IGameState::Item *item) {
  (void)item;
  return "item";
}

std::string make_object_info(IGameState::Object *object) {
  if (object == nullptr) {
    return "nil";
  }
  std::stringstream ss;
  auto desc = object->get_descriptor();
  if (desc == IGameState::ObjectDescriptor::ITEM) {
    auto as_item_object = dynamic_cast<ItemObject *>(object);
    assert(as_item_object);
    auto item_desc = as_item_object->get_item()->get_descriptor();
    auto it = ITEM_DESCRIPTOR_INFO.find(item_desc);
    assert(it != ITEM_DESCRIPTOR_INFO.end());
    ss << it->second;
  } else {
    auto it = OBJECT_DESCRIPTOR_INFO.find(desc);
    assert(it != OBJECT_DESCRIPTOR_INFO.end());
    ss << it->second;
  }
  ss << " { pos = (";
  auto [x, y] = object->get_pos();
  ss << std::to_string(x) << ", " << std::to_string(y) << ")";

  auto health_opt = object->get_health();
  if (health_opt.has_value()) {
    auto [health, max_health] = health_opt.value();
    ss << ", health = " << std::to_string(health) << "/"
       << std::to_string(max_health);
  }
  if (auto label = object->get_label(); label.has_value()) {
    ss << ", label = " << label.value();
  }
  ss << " }";
  return ss.str();
}

struct GameUI {
  GameUI(std::shared_ptr<const IGameState> state) : state(std::move(state)) {}

  void draw() {
    safe_call(erase);
    under_carriage.type = UnderCarriage::Type::NONE;
    int header_end_x = draw_header();
    /* Leave place for current object description. */
    int field_start_x = header_end_x + 3;
    draw_field(field_start_x);
    draw_info(field_start_x + H_FIELD + 1);
    draw_current_object_info(header_end_x);
    move(carriage_x, carriage_y);
    refresh();
  }

  Event next() {
    while (true) {
      flushinp();
      auto c = getch();
      switch (c) {
        /* Internal UI events. */
        case KEY_LEFT:
          carriage_pinned = false;
		      carriage_y = std::max(0, carriage_y - 1);
          break;
        case KEY_RIGHT:
          carriage_pinned = false;
		      carriage_y = std::min(W - 1, carriage_y + 1);
          break;
        case KEY_UP:
          carriage_pinned = false;
		      carriage_x = std::max(0, carriage_x - 1);
          break;
        case KEY_DOWN:
          carriage_pinned = false;
		      carriage_x = std::min(H - 1, carriage_x + 1);
          break;
        case 'p':
        case 'P':
          carriage_pinned = true;
          break;

        /* External events. */
        case 'd':
        case 'D':
          return {GameState::PlayerMoveEvent::Right};
        case 'a':
        case 'A':
          return {GameState::PlayerMoveEvent::Left};
        case 'w':
        case 'W':
          return {GameState::PlayerMoveEvent::Up};
        case 's':
        case 'S':
          return {GameState::PlayerMoveEvent::Down};
        case KEY_ENTER:
        case '\n':
          switch (under_carriage.type) {
            case UnderCarriage::Type::ITEM:
              return {GameState::ApplyItemEvent{
                .pos = under_carriage.item_pos,
              }};
            case UnderCarriage::Type::OBJECT:
              return {GameState::ApplyObjectEvent{
                .object = under_carriage.object,
              }};
            default:
              break;
          }
        default:
          return {GameState::NoOpEvent{}};
      }
      draw();
    }
  }

 private:


  // Draws header, returns x where finished.
  int draw_header() {
    move(0, 0);
    auto player = dynamic_cast<Player *>(state->get_player());

    auto [player_x, player_y] = player->get_pos();
    printw("Pos:    (%d, %d)\n", player_x, player_y);
    //int draw_x, draw_y;
    //getyx(stdscr, draw_y, draw_x);
    //printw("Draw Pos:    (%d, %d)\n", draw_x, draw_y);
    printw("Health: ");
    auto [health, max_health] = player->get_health().value();
    draw_healthbar(health, max_health);
    draw_hand(player->get_hand());
    auto as_inventory = dynamic_cast<Inventory *>(player);
    draw_inventory(*as_inventory);
    return 4;
  }

  void draw_current_object_info(int start_x) {
    move(start_x, 0);
    printw("Obj:    ");
    if (under_carriage.type == UnderCarriage::Type::OBJECT)
      printw("%s\n", make_object_info(under_carriage.object).data());
    else if (under_carriage.type == UnderCarriage::Type::ITEM) {
      auto player = dynamic_cast<Player *>(state->get_player());
      auto item = player->get_stash()[under_carriage.item_pos].get();
      printw("%s\n", make_item_info(item).data());
    }
    printw("\n");
  }

  static void draw_healthbar(int health, int max_health) {
    attron(COLOR_PAIR(1));
    for (size_t i = 0; i < health; ++i) {
      printw("|");
    }
    attroff(COLOR_PAIR(1));
    for (size_t i = 0; i < max_health - health; ++i) {
      printw("|");
    }
    printw("\n");
  }

  void draw_hand(const Stick *hand) {
    char c = '_';
    if (hand != nullptr) {
      auto hand_desc = hand->get_descriptor();
      auto it = ITEM_DESCRIPTOR_CHAR.find(hand_desc);
      assert(it != ITEM_DESCRIPTOR_CHAR.end());
      c = it->second;
    }
    printw("Hand:  %c\n", c);
  }

  void draw_inventory(const Inventory &inventory) {
    printw("Stash:  [");
    auto &stash = inventory.get_stash();
    //int y, x;
    //getyx(stdscr, y, x);
    for (int i = 0; i < stash.size(); i++) {
      int y, x;
      getyx(stdscr, y, x);
      auto &item = stash[i];
      auto item_desc = item->get_descriptor();
      auto it = ITEM_DESCRIPTOR_CHAR.find(item_desc);
      assert(it != ITEM_DESCRIPTOR_CHAR.end());
      printw("%c", it->second);
      if (carriage_x == y && carriage_y == x) {
        /* Remember current item. */
        under_carriage.type = UnderCarriage::Type::ITEM;
        under_carriage.item_pos = i;
      }
    }
    int empty_cells = inventory.get_max_stash_size() -
      (int)inventory.get_stash().size();
    printw("%s", std::string(empty_cells, '_').c_str());
    printw("]\n");
  }

  static char make_object_char(IGameState::Object *obj) {
    auto desc = obj->get_descriptor();
    switch (desc) {
      case IGameState::ObjectDescriptor::ENTER: {
        auto as_enter = dynamic_cast<IGameState::EnterObj *>(obj);
        assert(as_enter);
        return as_enter->get_transition().at(0);
      }
      case IGameState::ObjectDescriptor::ITEM: {
        auto as_item_object = dynamic_cast<ItemObject *>(obj);
        assert(as_item_object);
        auto item_desc = as_item_object->get_item()->get_descriptor();
        auto it = ITEM_DESCRIPTOR_CHAR.find(item_desc);
        assert(it != ITEM_DESCRIPTOR_CHAR.end());
        return it->second;
      }
      default:
        break;
    }
    auto it = OBJECT_DESCRIPTOR_CHAR.find(desc);
    assert(it != OBJECT_DESCRIPTOR_CHAR.end());
    return it->second;
  }

  // Draws a field.
  void draw_field(int start_x) {
    auto player = state->get_player();
    auto [player_x, player_y] = player->get_pos();
    auto [lx, ux] = get_bounds(player_x, H_FIELD - 2);
    auto [ly, uy] = get_bounds(player_y, W_FIELD - 2);

    bool set_carriage_to_player = false;
    const auto map = state->get_map();
    if (map.name != previous_location) {
      /* Location changes. So pin carriage and move to it to player. */
      carriage_pinned = true;
	    set_carriage_to_player = true;
    }
    previous_location = map.name;

    auto previous_object = ((under_carriage.type == UnderCarriage::Type::OBJECT) ?
                            under_carriage.object : nullptr);
    for (const auto &object : map.objects) {
      auto [x, y] = object->get_pos();
      /* Check that object is contained in a visual field. */
      if (lx <= x && x < ux && ly <= y && y < uy) {
        auto descriptor = object->get_descriptor();
        x = rem(x, H_FIELD - 2) + 1;
        y = rem(y, W_FIELD - 2) + 1;

        auto symbol = make_object_char(object);
        int attr = 0;
        switch (descriptor) {
          case IGameState::ObjectDescriptor::ENTER: {
            attr = COLOR_PAIR(2);
            break;
          }
          case IGameState::ObjectDescriptor::ORC: {
            attr = COLOR_PAIR(3);
            break;
          }
          case IGameState::ObjectDescriptor::BAT: {
            attr = COLOR_PAIR(4);
          }
          default: {
            break;
          }
        }

        attron(attr);
        mvaddch(start_x + x, y, symbol);
        attroff(attr);

        if (descriptor == IGameState::ObjectDescriptor::PLAYER &&
            set_carriage_to_player) {
          carriage_x = start_x + x;
          carriage_y = y;
        }

        if (carriage_pinned && object == previous_object) {
          carriage_x = start_x + x;
          carriage_y = y;
        }
        if (carriage_x == start_x + x && carriage_y == y) {
          /* Remember current object. */
          under_carriage.type = UnderCarriage::Type::OBJECT;
          under_carriage.object = object;
        }
      }
    }
  }

  static void draw_info(int start_x) {
    move(start_x, 0);
    printw("Help:\n");
    printw("- Carriage:       ARROWS\n");
    printw("- Pin carriage:   P\n");
    printw("- Hero:           WASD\n");
    printw("- Apply:          ENTER\n");
  }

  std::shared_ptr<const IGameState> state;
  int carriage_x{};
  int carriage_y{};
  bool carriage_pinned{};
  std::string_view previous_location;

  struct UnderCarriage {
      enum class Type {
          NONE,
          ITEM,
          OBJECT,
      };
      Type type;
      union {
          int item_pos;
          IGameState::Object *object;
      };
  } under_carriage;
};
