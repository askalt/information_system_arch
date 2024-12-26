#pragma once

#include <locale.h>

#include <algorithm>
#include <set>
#include <sstream>
#include <thread>
#include <unordered_map>

#include "curses.h"
#include "event.h"
#include "panic.h"

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

/* Add it to color pair argument [1;6] to use different backrground. */
const int BLUE_SHIFT = 6;
const int RED_SHIFT = BLUE_SHIFT * 2;

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

  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(4, COLOR_RED, COLOR_BLACK);
  init_pair(5, COLOR_YELLOW, COLOR_BLACK);
  init_pair(6, COLOR_BLUE, COLOR_BLACK);

  init_pair(7, COLOR_WHITE, COLOR_BLUE);
  init_pair(8, COLOR_GREEN, COLOR_BLUE);
  init_pair(9, COLOR_MAGENTA, COLOR_BLUE);
  init_pair(10, COLOR_RED, COLOR_BLUE);
  init_pair(11, COLOR_YELLOW, COLOR_BLUE);
  init_pair(12, COLOR_BLUE, COLOR_BLUE);

  init_pair(13, COLOR_WHITE, COLOR_RED);
  init_pair(14, COLOR_GREEN, COLOR_RED);
  init_pair(15, COLOR_MAGENTA, COLOR_RED);
  init_pair(16, COLOR_BLACK, COLOR_RED);
  init_pair(17, COLOR_YELLOW, COLOR_RED);
  init_pair(18, COLOR_BLUE, COLOR_RED);

  noecho();
  resize_term(H, W);
}

const std::unordered_map<IGameState::ObjectDescriptor, char>
    PART_DESCRIPTOR_CHAR = {
        {IGameState::ObjectDescriptor::PLAYER, 'p'},
        {IGameState::ObjectDescriptor::WALL, '*'},
        {IGameState::ObjectDescriptor::CHEST, '@'},
        {IGameState::ObjectDescriptor::STONE, '#'},
        {IGameState::ObjectDescriptor::HORIZONTAL_BORDER, '-'},
        {IGameState::ObjectDescriptor::VERTICAL_BORDER, '|'},
        {IGameState::ObjectDescriptor::CORNER, '+'},
        {IGameState::ObjectDescriptor::EXIT, '%'},
        {IGameState::ObjectDescriptor::ORC, 'O'},
        {IGameState::ObjectDescriptor::BAT, 'B'},
};

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

std::string make_object_info(IGameState::Object *object) {
  if (object == nullptr) {
    return "nil";
  }
  std::stringstream ss;
  switch (object->get_descriptor()) {
    case IGameState::ObjectDescriptor::PLAYER: {
      ss << "player";
      break;
    }
    case IGameState::ObjectDescriptor::WALL: {
      ss << "wall";
      break;
    }
    case IGameState::ObjectDescriptor::CHEST: {
      ss << "chest";
      break;
    }
    case IGameState::ObjectDescriptor::STONE: {
      ss << "stone";
      break;
    }
    case IGameState::ObjectDescriptor::ENTER: {
      ss << "enter";
      break;
    }
    case IGameState::ObjectDescriptor::CORNER:
    case IGameState::ObjectDescriptor::HORIZONTAL_BORDER:
    case IGameState::ObjectDescriptor::VERTICAL_BORDER: {
      ss << "nil";
      break;
    }
    case IGameState::ObjectDescriptor::EXIT: {
      ss << "exit";
      break;
    }
    case IGameState::ObjectDescriptor::ORC: {
      ss << "orc";
      break;
    }
    case IGameState::ObjectDescriptor::BAT: {
      ss << "bat";
      break;
    }
    default:
      panic("uncovered");
  }
  ss << " { pos = (";
  auto [x, y] = object->get_pos();
  ss << std::to_string(x) << ", " << std::to_string(y) << ")";

  if (auto healthable = dynamic_cast<IGameState::IHealthable *>(object);
      healthable != nullptr) {
    auto [health, max_health] = healthable->get_health();
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
    int header_end_x = draw_header();
    /* Leave place for current object description. */
    int field_start_x = header_end_x + 3;
    draw_field(field_start_x);
    draw_help(field_start_x + H_FIELD + 1);
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
          return {GameState::ApplyEvent{
              .object = current_object,
          }};
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
    auto player = state->get_player();
    auto [player_x, player_y] = player->get_pos();
    printw("Pos:    (%d, %d)\n", player_x, player_y);
    printw("Health: ");
    auto [health, max_health] = player->get_health();
    draw_healthbar(health, max_health);
    printw("Level:  %d\n", player->get_lvl());
    printw("Exp:    (%d/%d)\n", player->get_exp(), player->get_lvl_exp());
    printw("Items:  []\n");
    printw("Stash:  []\n");
    return 7;
  }

  void draw_current_object_info(int start_x) {
    move(start_x, 0);
    printw("Obj:    %s\n", make_object_info(current_object).data());
  }

  void draw_healthbar(int health, int max_health) {
    attron(COLOR_PAIR(2));
    for (size_t i = 0; i < health; ++i) {
      printw("|");
    }
    attroff(COLOR_PAIR(2));
    for (size_t i = 0; i < max_health - health; ++i) {
      printw("|");
    }
    printw("\n");
  }

  char make_object_symbol(IGameState::Object *obj) {
    auto desc = obj->get_descriptor();
    if (desc == IGameState::ObjectDescriptor::ENTER) {
      auto as_enter = dynamic_cast<IGameState::IEnter *>(obj);
      assert(as_enter);
      return as_enter->get_transition().at(0);
    }
    auto it = PART_DESCRIPTOR_CHAR.find(desc);
    assert(it != PART_DESCRIPTOR_CHAR.end());
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
      /* Location changes. So pin the carriage and move to it to player. */
      carriage_pinned = true;
      set_carriage_to_player = true;
    }
    previous_location = map.name;

    auto previous_object = current_object;
    current_object = nullptr;

    /* First pass: determine on which objects carriage locates. */
    for (const auto &object : map.objects) {
      auto [x, y] = object->get_pos();
      /* Check that object is contained in a visual field. */
      if (lx <= x && x < ux && ly <= y && y < uy) {
        auto descriptor = object->get_descriptor();
        x = rem(x, H_FIELD - 2) + 1;
        y = rem(y, W_FIELD - 2) + 1;

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
          current_object = object;
        }
      }
    }

    /* Second pass, draw a field. */
    int attack_field_color_pair_shift = 0;
    std::set<std::pair<int, int>> attack_area;
    if (current_object == static_cast<IGameState::Object *>(player)) {
      attack_area = player->get_attack_area();
      attack_field_color_pair_shift = BLUE_SHIFT;
    } else if (auto mob = dynamic_cast<IGameState::IMob *>(current_object);
               mob != nullptr) {
      attack_area = mob->get_attack_area();
      attack_field_color_pair_shift = RED_SHIFT;
    }

    for (const auto &object : map.objects) {
      auto [x, y] = object->get_pos();
      /* Check that object is contained in a visual field. */
      if (lx <= x && x < ux && ly <= y && y < uy) {
        auto descriptor = object->get_descriptor();
        bool in_attack_area = attack_area.find({x, y}) != attack_area.end();
        if (in_attack_area) {
          attack_area.erase({x, y});
        }

        x = rem(x, H_FIELD - 2) + 1;
        y = rem(y, W_FIELD - 2) + 1;
        auto symbol = make_object_symbol(object);
        int attr = 0;
        switch (descriptor) {
          case IGameState::ObjectDescriptor::ENTER: {
            attr =
                COLOR_PAIR(3 + attack_field_color_pair_shift * in_attack_area);
            break;
          }
          case IGameState::ObjectDescriptor::ORC: {
            attr =
                COLOR_PAIR(4 + attack_field_color_pair_shift * in_attack_area);
            break;
          }
          case IGameState::ObjectDescriptor::BAT: {
            attr =
                COLOR_PAIR(5 + attack_field_color_pair_shift * in_attack_area);
            break;
          }
          default: {
            attr =
                COLOR_PAIR(1 + attack_field_color_pair_shift * in_attack_area);
            break;
          }
        }
        attron(attr);
        mvaddch(start_x + x, y, symbol);
        attroff(attr);
      }
    }
    for (auto [x, y] : attack_area) {
      /* Check that object is contained in a visual field. */
      if (lx <= x && x < ux && ly <= y && y < uy) {
        x = rem(x, H_FIELD - 2) + 1;
        y = rem(y, W_FIELD - 2) + 1;
        char symbol = ' ';
        attron(COLOR_PAIR(1 + attack_field_color_pair_shift));
        mvaddch(start_x + x, y, symbol);
        attroff(COLOR_PAIR(1 + attack_field_color_pair_shift));
      }
    }
  }

  void draw_help(int start_x) {
    move(start_x, 0);
    printw("Help:\n");
    printw("- Carriage:      ARROWS\n");
    printw("- Pin carriage:  P\n");
    printw("- Hero:          WASD\n");
    printw("- Apply:         ENTER\n");
  }

  std::shared_ptr<const IGameState> state;
  int carriage_x{};
  int carriage_y{};
  bool carriage_pinned{};
  std::string_view previous_location;
  IGameState::Object *current_object = nullptr;
};
