#pragma once

#include <locale.h>

#include <algorithm>
#include <sstream>
#include <thread>
#include <unordered_map>

#include "curses.h"
#include "event.h"
#include "panic.h"
#include "state.h"

#define safe_call(f) \
  if (f() == ERR) {  \
    panic(#f);       \
  }

/* Console sizes. */
const int W = 200;
const int H = 100;

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
  noecho();
  resize_term(H, W);
}

const std::unordered_map<IGameState::ObjectDescriptor, char>
    PART_DESCRIPTOR_CHAR = {
        {IGameState::ObjectDescriptor::PLAYER, 'p'},
        {IGameState::ObjectDescriptor::WALL, '*'},
        {IGameState::ObjectDescriptor::CHEST, '@'},
        {IGameState::ObjectDescriptor::STONE, '#'},
        {IGameState::ObjectDescriptor::ENTER, 'E'},
        {IGameState::ObjectDescriptor::HORIZONTAL_BORDER, '-'},
        {IGameState::ObjectDescriptor::VERTICAL_BORDER, '|'},
        {IGameState::ObjectDescriptor::CORNER, '+'},
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
    default:
      panic("uncovered");
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
    int header_end_x = draw_header();
    /* Leave place for current object description. */
    int field_start_x = header_end_x + 3;
    draw_field(field_start_x);
    draw_current_object_info(header_end_x);
    move(carrier_x, carrier_y);
    refresh();
  }

  Event next() {
    while (true) {
      flushinp();
      auto c = getch();
      switch (c) {
        /* Internal UI events. */
        case KEY_LEFT:
          carrier_y = std::max(0, carrier_y - 1);
          break;
        case KEY_RIGHT:
          carrier_y = std::min(W - 1, carrier_y + 1);
          break;
        case KEY_UP:
          carrier_x = std::max(0, carrier_x - 1);
          break;
        case KEY_DOWN:
          carrier_x = std::min(H - 1, carrier_x + 1);
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
          return {GameState::EnterEvent{}};
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
    auto [health, max_health] = player->get_health().value();
    draw_healthbar(health, max_health);
    printw("Items:  []\n");
    printw("Stash:  []\n");
    return 4;
  }

  void draw_current_object_info(int start_x) {
    move(start_x, 0);
    printw("Obj:    %s\n", make_object_info(current_object).data());
  }

  void draw_healthbar(int health, int max_health) {
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

  // Draws a field.
  void draw_field(int start_x) {
    // draw_border(start_x);

    auto player = state->get_player();
    auto [player_x, player_y] = player->get_pos();
    auto [lx, ux] = get_bounds(player_x, H_FIELD - 2);
    auto [ly, uy] = get_bounds(player_y, W_FIELD - 2);

    const auto objects = state->get_objects();

    current_object = nullptr;
    for (const auto &object : objects) {
      auto [x, y] = object->get_pos();
      /* Check that object is contained in a visual field. */
      if (lx <= x && x < ux && ly <= y && y < uy) {
        auto descriptor = object->get_descriptor();
        x = rem(x, H_FIELD - 2) + 1;
        y = rem(y, W_FIELD - 2) + 1;
        auto chr = PART_DESCRIPTOR_CHAR.find(descriptor);
        assert(chr != PART_DESCRIPTOR_CHAR.end());
        mvaddch(start_x + x, y, chr->second);
        if (carrier_x == start_x + x && carrier_y == y) {
          /* Remember current object. */
          current_object = object;
        }
      }
    }
  }

  // Draws a border.
  void draw_border(int start_x) {
    mvaddch(0 + start_x, 0, '+');
    for (size_t i = 0; i < W_FIELD - 2; ++i) addch('-');
    addch('+');
    for (size_t i = 1; i < H_FIELD - 1; ++i) {
      mvaddch(i + start_x, 0, '|');
      mvaddch(i + start_x, W_FIELD - 1, '|');
    }
    mvaddch(H_FIELD - 1 + start_x, 0, '+');
    for (size_t i = 0; i < W_FIELD - 2; ++i) addch('-');
    addch('+');
  }

  std::shared_ptr<const IGameState> state;
  int carrier_x{};
  int carrier_y{};
  IGameState::Object *current_object = nullptr;
};
