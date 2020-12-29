#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
                          // in one cpp file
#include "catch_amalgamated.hpp"

#include "mcts.h"
#include "tic-tac-toe.h"

#include "policy.h"

typedef TTTState State;
typedef TTTAction Action;

// make a fake policy that does a hard-coded set of actions.
template <class State, class Action>
class HardCodedPolicy : public Policy<State, Action> {
public:
  HardCodedPolicy(std::vector<Action> actions)
      : actions_(std::move(actions)), current_action_idx_(0) {}

  Action act(const Game<State, Action> *game) override {
    assert(!game->isTerminal());
    assert(current_action_idx_ < actions_.size());
    return actions_.at(current_action_idx_++);
  }

private:
  // This hard-coded policy will apply actions in order
  int current_action_idx_ = 0;
  std::vector<Action> actions_;
};

TEST_CASE("Factorials are computed", "[factorial]") {
  // REQUIRE(Factorial(1) == 1);
  // REQUIRE(Factorial(2) == 2);
  // REQUIRE(Factorial(3) == 6);
  // REQUIRE(Factorial(10) == 3628800);
}
