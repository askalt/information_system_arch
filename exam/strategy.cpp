
/**
 *
 * Ticket 26.
 *
 * https://en.wikipedia.org/wiki/Strategy_pattern
 *
 * In computer programming, the strategy pattern (also known as the policy
 * pattern) is a behavioral software design pattern that enables selecting an
 * algorithm at runtime. Instead of implementing a single algorithm directly,
 * code receives runtime instructions as to which in a family of algorithms to
 * use.
 *
 *
 * The strategy pattern uses composition instead of inheritance. In the strategy
 * pattern, behaviors are defined as separate interfaces and specific classes
 * that implement these interfaces. This allows better decoupling between the
 * behavior and the class that uses the behavior. The behavior can be changed
 * without breaking the classes that use it, and the classes can switch between
 * behaviors by changing the specific implementation used without requiring any
 * significant code changes
 *
 *
 * Example: different loggers. For example our algorithm writes something
 * to log and it uses logger interface, logging strategy is determined by user.
 */

#include <cassert>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

enum class LOG_LEVEL {
  DEBUG,
  ERROR,
};

struct Logger {
  virtual void log(LOG_LEVEL level, std::string &&msg) = 0;
  virtual ~Logger() = default;
};

struct MemLogger : public Logger {
  void log(LOG_LEVEL level, std::string &&msg) override {
    messages.emplace_back(std::move(msg));
  }

  std::vector<std::string> messages;
};

struct CriticalMemLogger : public Logger {
  void log(LOG_LEVEL level, std::string &&msg) override {
    if (level == LOG_LEVEL::ERROR) {
      messages.emplace_back(std::move(msg));
    }
  }

  std::vector<std::string> messages;
};

void run_algorithm(std::shared_ptr<Logger> logger) {
  for (int i = 0; i < 5; ++i) {
    std::stringstream ss;
    ss << "Visit number " << i;
    LOG_LEVEL level = LOG_LEVEL::DEBUG;
    if (i % 2 == 0) {
      level = LOG_LEVEL::ERROR;
    }
    logger->log(level, ss.str());
  }
}

int main() {
  {
    auto logger = std::make_shared<MemLogger>();
    run_algorithm(logger);

    auto expected = std::vector<std::string>{
        "Visit number 0", "Visit number 1", "Visit number 2",
        "Visit number 3", "Visit number 4",
    };

    assert(logger->messages == expected);
  }

  {
    auto logger = std::make_shared<CriticalMemLogger>();
    run_algorithm(logger);
    auto expected = std::vector<std::string>{
        "Visit number 0",
        "Visit number 2",
        "Visit number 4",
    };

    assert(logger->messages == expected);
  }

  return 0;
}
