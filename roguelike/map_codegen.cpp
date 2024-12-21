#include <iostream>
#include <sstream>
#include <string>

using namespace std;

void solve() {
  string s;
  int x = 0;
  stringstream ss;
  ss << "#include \"consts.h\"\n";
  ss << "#include \"map.h\"\n";
  ss << "std::unique_ptr<Map> make_zero_map(IGameState::Object* player) {\n";
  ss << "  auto mp = std::make_unique<Map>(player);\n";
  while (getline(cin, s)) {
    ++x;
    for (int y = 0; y < s.size(); ++y) {
      if (s[y] == '+') {
        ss << "  mp->push_new_object(mp->borders, "
              "std::move(std::make_unique<Border>("
           << x << ", " << y << ", IGameState::ObjectDescriptor::CORNER)));\n";
      } else if (s[y] == '-') {
        ss << "  mp->push_new_object(mp->borders, "
              "std::move(std::make_unique<Border>("
           << x << ", " << y
           << ", IGameState::ObjectDescriptor::HORIZONTAL_BORDER)));\n";
      } else if (s[y] == '|') {
        ss << "  mp->push_new_object(mp->borders, "
              "std::move(std::make_unique<Border>("
           << x << ", " << y
           << ", IGameState::ObjectDescriptor::VERTICAL_BORDER)));\n";
      } else if (s[y] == '@') {
        ss << "  mp->push_new_object(mp->chests, "
              "std::move(std::make_unique<Chest>("
           << x << ", " << y << ")));\n";
      } else if (s[y] == 'E') {
        ss << "  mp->push_new_object(mp->enters, "
              "std::move(std::make_unique<Enter>("
           << x << ", " << y << ", LEVEL_0_DUNGEON)));\n";
      } else if (s[y] == '*') {
        ss << "  mp->push_new_object(mp->dungeon_blocks, "
              "std::move(std::make_unique<DungeonBlock>("
           << x << ", " << y << ", LEVEL_0_DUNGEON)));\n";
      }
    }
  }
  ss << "  return mp;";
  ss << "}\n";
  cout << ss.str();
}

// Usage:
// ./map_code_generator < input.txt > output.cpp
signed main() {
  ios::sync_with_stdio(false);
  cin.tie(0);
  solve();
  return 0;
}
