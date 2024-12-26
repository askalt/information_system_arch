#include "decision_tree.h"

void interpretate(std::shared_ptr<DecisionTreeNode> root) {
  while (root != nullptr) {
    root = root->decide();
  }
}

std::shared_ptr<DecisionTreeNode> NoOpNode::decide() const { return nullptr; }
