
/**
 *
 * Ticket 27.
 *
 * The adapter design pattern solves problems like:
 * - How can a class be reused that does not have an interface that a client
 * requires?
 * - How can classes that have incompatible interfaces work together?
 * - How can an alternative interface be provided for a class?
 *
 * The key idea in this pattern is to work through a separate adapter
 * that adapts the interface of an (already existing) class without changing it.
 */

#include <cassert>
#include <string>
#include <unordered_map>

using T = int;
using S = std::string;

struct Adaptee {
  void set(S s) { m[s] = true; }

  std::unordered_map<S, bool> m;
};

struct Setter {
  virtual void set(T) = 0;
};

struct Adapter : public Setter {
  virtual void set(T t) { adaptee.set(std::to_string(std::move(t))); }

  Adapter(Adaptee& adaptee) : adaptee{adaptee} {}

 private:
  Adaptee& adaptee;
};

int main() {
  Adaptee adaptee{};
  Adapter adapter{adaptee};

  adapter.set(1);
  adapter.set(42);

  std::unordered_map<std::string, bool> expected = {
      {"1", true},
      {"42", true},
  };

  assert(adaptee.m == expected);

  return 0;
}
