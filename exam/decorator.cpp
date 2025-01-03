#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

/**
 * Ticket 25:
 * Decorator pattern.
 *
 * In object-oriented programming, the decorator pattern is a design pattern
 * that allows behavior to be added to an individual object, dynamically,
 * without affecting the behavior of other instances of the same class.
 *
 * Example:
 * Readers implementations: batched reader can batch operations to provide
 * caller ability to process several at a time.
 *
 * Also, parsers.
 */

using Row = std::tuple<int, std::string>;

struct RowReader {
  virtual std::vector<Row> read_rows() = 0;
  virtual ~RowReader() = default;
};

struct RandomRowReader : public RowReader {
  std::vector<Row> read_rows() override {
    int id = rand() % 10;
    std::string s;
    if (rand() % 2 == 0) {
      s = "a";
    } else {
      s = "b";
    }
    return {{id, s}};
  }
};

struct BatchedRowReader : public RowReader {
  BatchedRowReader(int batch_size, std::shared_ptr<RowReader> reader)
      : batch_size{batch_size}, reader{std::move(reader)} {}

  std::vector<Row> read_rows() override {
    std::vector<Row> rows;
    rows.reserve(batch_size);
    while (rows.size() < batch_size) {
      for (auto &&row : reader->read_rows()) {
        rows.push_back(std::move(row));
      }
    }
    return rows;
  }

 private:
  int batch_size;
  std::shared_ptr<RowReader> reader;
};

int main() {
  auto random_reader = std::make_shared<RandomRowReader>();

  int batch_size = 5;
  auto batched_reader =
      std::make_shared<BatchedRowReader>(batch_size, std::move(random_reader));
  auto rows = batched_reader->read_rows();
  assert(rows.size() == batch_size);
  std::cout << "OK" << std::endl;
  return 0;
}
