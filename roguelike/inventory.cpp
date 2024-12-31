#include <stdexcept>
#include "inventory.h"

Inventory::Inventory(int max_stash_size)
  : max_stash_size(max_stash_size) {}

bool Inventory::put_item(std::unique_ptr<IGameState::Item> &item) {
  if (stash.size() < max_stash_size) {
    stash.push_back(std::move(item));
    return true;
  }
  return false;
}

bool Inventory::drop_item(int pos) {
  throw std::runtime_error("Not implemented!");
}

std::unique_ptr<IGameState::Item> Inventory::take_item(int pos) {
  if (pos < stash.size()) {
    auto &item = stash[pos];
    switch (item->get_descriptor()) {
      case (IGameState::ItemDescriptor::STICK): {
        std::unique_ptr<Stick> current_item(
          dynamic_cast<Stick *>(item.release()));
        stash.erase(stash.begin() + pos);
        return std::move(current_item);
      }
      default:
        break;
    }
  }
  return nullptr;
}

const Inventory::Stash &Inventory::get_stash() const {
  return stash;
}

int Inventory::get_max_stash_size() const {
  return max_stash_size;
}
