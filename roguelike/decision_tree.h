#pragma once
#include <algorithm>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "entities.h"

struct DecisionTreeNode {
  virtual std::shared_ptr<DecisionTreeNode> decide() const = 0;
  virtual ~DecisionTreeNode() = default;
};

template <typename T>
struct RandowWalkNode : DecisionTreeNode {
  RandowWalkNode(int sx, int sy, int max_dist, T &obj)
      : sx{sx}, sy{sy}, max_dist{max_dist}, obj(obj) {}

  std::shared_ptr<DecisionTreeNode> decide() const override {
    const int DX_SZ = 5;
    const int dx[] = {0, 0, 0, 1, -1};
    const int dy[] = {0, 1, -1, 0, 0};
    auto current_map = obj.get_state()->get_current_map();
    auto [x, y] = obj.get_pos();
    int vars[DX_SZ]{};
    int j = 0;
    for (int i = 0; i < DX_SZ; ++i) {
      int xx = x + dx[i];
      int yy = y + dy[i];
      if (!current_map->has_object(xx, yy,
                                   static_cast<IGameState::Object *>(&obj))) {
        vars[j++] = i;
      }
    }
    if (j != 0) {
      int choose = rand() % j;
      x += dx[vars[choose]];
      y += dy[vars[choose]];
      obj.set_pos(x, y);
    }
    return nullptr;
  }

 private:
  /* Spawnpoint. */
  int sx, sy;
  /* If distance from spawnpoint at least `max_dist`,
   * then tries to return back, otherwise chooses a random direction.
   */
  int max_dist;
  T &obj;
};

/* Use a comparator to choose: be closer or farther to the player. */
template <typename T, typename Comparator>
struct DistanceComparatorNode : DecisionTreeNode {
  DistanceComparatorNode(T &obj) : obj{obj} {}

  std::shared_ptr<DecisionTreeNode> decide() const override {
    auto [x, y] = obj.get_pos();
    auto [px, py] = obj.get_state()->get_player()->get_pos();
    auto map = obj.get_state()->get_current_map();
    const int dx[] = {0, 1, -1, 0, 0};
    const int dy[] = {0, 0, 0, 1, -1};
    std::pair<int, int> vars[5]{};
    int j = 0;
    for (int i = 0; i < sizeof(dx) / sizeof(int); ++i) {
      int xx = x + dx[i];
      int yy = y + dy[i];
      if (!map->has_object(xx, yy, static_cast<IGameState::Object *>(&obj))) {
        vars[j].second = i;
        vars[j].first = abs(xx - px) + abs(yy - py);
        j++;
      }
    }
    auto cmp = Comparator{};
    sort(vars, vars + j, cmp);
    int cntv = 0;
    while (cntv < sizeof(vars) / sizeof(vars[0]) && !cmp(vars[cntv], vars[0]) &&
           !cmp(vars[0], vars[cntv])) {
      cntv++;
    }
    if (cntv != 0) {
      int choose = rand() % cntv;
      int new_x = x + dx[vars[choose].second];
      int new_y = y + dy[vars[choose].second];
      obj.set_pos(new_x, new_y);
    }
    return nullptr;
  }

 private:
  T &obj;
};

template <typename T>
struct AttackNode : DecisionTreeNode {
  AttackNode(T &obj) : obj(obj){};

  std::shared_ptr<DecisionTreeNode> decide() const override {
    int dmg = obj.get_damage();
    obj.get_state()->damage_player(dmg);
    return nullptr;
  }

 private:
  T &obj;
};

template <typename T>
struct ConditionNode : DecisionTreeNode {
  struct Node {
    std::function<bool(T &)> predicate;
    std::shared_ptr<DecisionTreeNode> next;
  };

  ConditionNode(T &obj, std::vector<Node> nodes,
                std::shared_ptr<DecisionTreeNode> default_)
      : obj(obj), nodes(std::move(nodes)), default_(std::move(default_)) {}

  std::shared_ptr<DecisionTreeNode> decide() const {
    for (const auto &node : nodes) {
      if (node.predicate(obj)) {
        return node.next;
      }
    }
    return default_;
  }

 private:
  T &obj;
  std::vector<Node> nodes;
  std::shared_ptr<DecisionTreeNode> default_;
};

struct NoOpNode : DecisionTreeNode {
  std::shared_ptr<DecisionTreeNode> decide() const;
};

// Interpretate a decision tree.
void interpretate(std::shared_ptr<DecisionTreeNode> root);
