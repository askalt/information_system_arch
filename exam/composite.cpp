#include <cassert>
#include <memory>
#include <string>
#include <vector>

/**
 * Ticket 24:
 * Composite pattern.
 * https://en.wikipedia.org/wiki/Composite_pattern
 * When dealing with Tree-structured data, programmers often have to
 * discriminate between a leaf-node and a branch. This makes code more complex,
 * and therefore, more error prone. The solution is an interface that allows
 * treating complex and primitive objects uniformly. In object-oriented
 * programming, a composite is an object designed as a composition of
 * one-or-more similar objects, all exhibiting similar functionality. This is
 * known as a "has-a" relationship between objects. The key concept is that
 * you can manipulate a single instance of the object just as you would
 * manipulate a group of them. The operations you can perform on all the
 * composite objects often have a least common denominator relationship. For
 * example, if defining a system to portray grouped shapes on a screen, it would
 * be useful to define resizing a group of shapes to have the same effect (in
 * some sense) as resizing a single shape.
 *
 * Composite should be used when clients ignore the difference between
 * compositions of objects and individual objects. If programmers find that they
 * are using multiple objects in the same way, and often have nearly identical
 * code to handle each of them, then composite is a good choice; it is less
 * complex in this situation to treat primitives and composites as homogeneous.
 *
 * Example:
 * Filesystem and size calculation: folder and file can implement single
 * interface that can calculate size.
 */

struct Sizeable {
  virtual int get_size() const = 0;
  virtual ~Sizeable() = default;
};

// Leaf file.
struct File : public Sizeable {
  File(std::string name, int sz) : name{std::move(name)}, sz{sz} {}

  int get_size() const override { return sz; }

 private:
  std::string name;
  int sz;
};

struct Dir : public Sizeable {
  Dir(std::vector<std::shared_ptr<Sizeable>> childs)
      : childs{std::move(childs)} {}

  int get_size() const override {
    int tot = 0;
    for (const auto &child : childs) {
      tot += child->get_size();
    }
    return tot;
  }

 private:
  std::vector<std::shared_ptr<Sizeable>> childs;
};

int main() {
  auto file_a = std::make_shared<File>("a.txt", 3);
  auto file_b = std::make_shared<File>("b.txt", 1);
  auto dir_c = std::make_shared<Dir>(
      std::vector{std::static_pointer_cast<Sizeable>(file_a),
                  std::static_pointer_cast<Sizeable>(file_b)});
  auto file_d = std::make_shared<File>("d.txt", 12);
  auto dir_e = std::make_shared<Dir>(
      std::vector{std::static_pointer_cast<Sizeable>(file_d),
                  std::static_pointer_cast<Sizeable>(dir_c)});
  /**
   * d.txt [12b]
   * c:
   *   a.txt [3b]
   *   b.txt [1b]
   */
  auto sz_e = dir_e->get_size();
  assert(sz_e == 3 + 1 + 12);
  auto sz_a = file_a->get_size();
  assert(sz_a == 3);
  auto sz_c = dir_c->get_size();
  assert(sz_c == 3 + 1);
  return 0;
}
