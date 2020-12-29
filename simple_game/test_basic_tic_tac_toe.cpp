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

using Catch::Approx;

TEST_CASE("Basic rollout backprop is working", "[mcts]") {
  // Make a game where x wins and make sure the tree is updated correctly.
  //
  //  x3, x7, x5
  //      x1, o2
  //  o6,   , o4
  std::vector<Action> self_moves = {Action(4), Action(0), Action(2), Action(1)};
  std::vector<Action> opponent_moves = {Action(5), Action(8), Action(6)};
  std::unique_ptr<Policy<State, Action>> self_policy =
      std::make_unique<HardCodedPolicy<State, Action>>(std::move(self_moves));
  std::unique_ptr<Policy<State, Action>> opponent_policy =
      std::make_unique<HardCodedPolicy<State, Action>>(
          std::move(opponent_moves));

  MCTS<State, Action> mcts;
  std::unique_ptr<Game<State, Action>> game = std::make_unique<TicTacToe>();
  auto rollout_history =
      mcts.rollout(game.get(), self_policy.get(), opponent_policy.get());

  // get the nodes for introspection
  const auto &nodes = mcts.getNodes();

  for (const auto &frame : rollout_history) {
    std::cout << "Verifying assumptions for state: " << std::endl;
    frame.state.render();
    REQUIRE(nodes.find(frame.state) != nodes.end());
    REQUIRE(nodes.at(frame.state).num_rollouts_involved == 1);
    // Each frame should have a positive reward since we won.
    REQUIRE(nodes.at(frame.state).total_reward_from_here == Approx(1.0));
  }

  // REQUIRE(Factorial(1) == 1);
  // REQUIRE(Factorial(2) == 2);
  // REQUIRE(Factorial(3) == 6);
  // REQUIRE(Factorial(10) == 3628800);
}
