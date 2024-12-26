#pragma once
#include "items.h"

class Inventory {
    using Stash = std::vector<std::unique_ptr<IGameState::Item>>;

public:
  explicit Inventory(int max_stash_size);

  bool put_item(std::unique_ptr<IGameState::Item> &item);

  bool drop_item(int pos);

  std::unique_ptr<IGameState::Item> take_item(int pos);

  const Stash &get_stash() const;

  int get_max_stash_size() const;

private:
  const int max_stash_size;
  Stash stash;
};

class InventoryItem {

};