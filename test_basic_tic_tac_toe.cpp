#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
                          // in one cpp file
#include "catch_amalgamated.hpp"

#include "mcts.h"
#include "tic-tac-toe.h"
#include "uct.h"

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

  MCTS<State, Action>::RolloutConfig config;
  config.update_weights = true;

  auto rollout_history = mcts.rollout(game.get(), self_policy.get(),
                                      opponent_policy.get(), config);

  // get the nodes for introspection
  const auto &nodes = mcts.getNodes();

  for (const auto &frame : rollout_history) {
    std::cout << "Verifying assumptions for state: " << frame.state.render()
              << std::endl;
    REQUIRE(nodes.find(frame.state) != nodes.end());
    REQUIRE(nodes.at(frame.state).num_rollouts_involved == 1);
    // Each frame should have a positive reward since we won.
    REQUIRE(nodes.at(frame.state).total_reward_from_here == Approx(1.0));
  }
}

TEST_CASE("Rollout backprop loss when playing as player 2", "[mcts]") {
  // Make a game where we play as player 2 and lose
  //
  //  x3, x7, x5
  //      x1, o2
  //  o6,   , o4
  std::vector<Action> opponent_moves = {Action(4), Action(0), Action(2),
                                        Action(1)};
  std::vector<Action> self_moves = {Action(5), Action(8), Action(6)};
  std::unique_ptr<Policy<State, Action>> self_policy =
      std::make_unique<HardCodedPolicy<State, Action>>(std::move(self_moves));
  std::unique_ptr<Policy<State, Action>> opponent_policy =
      std::make_unique<HardCodedPolicy<State, Action>>(
          std::move(opponent_moves));

  MCTS<State, Action> mcts;
  std::unique_ptr<Game<State, Action>> game = std::make_unique<TicTacToe>();

  MCTS<State, Action>::RolloutConfig config;
  config.opponent_goes_first = true;
  config.update_weights = true;

  auto rollout_history = mcts.rollout(game.get(), self_policy.get(),
                                      opponent_policy.get(), config);

  // get the nodes for introspection
  const auto &nodes = mcts.getNodes();

  for (const auto &frame : rollout_history) {
    std::cout << "Verifying assumptions for state: " << std::endl;
    frame.state.render();
    REQUIRE(nodes.find(frame.state) != nodes.end());
    REQUIRE(nodes.at(frame.state).num_rollouts_involved == 1);
    // Each frame should have a negative reward since we lost.
    REQUIRE(nodes.at(frame.state).total_reward_from_here == Approx(-1.0));
  }
}

TEST_CASE("Rollout backprop win when playing as player 2", "[mcts]") {
  // Make a game where we play as player 2 and win
  //
  //  x3,   , x5
  //  x7  x1, o2
  //  o6, o8, o4
  std::vector<Action> opponent_moves = {Action(4), Action(0), Action(2),
                                        Action(3)};
  std::vector<Action> self_moves = {Action(5), Action(8), Action(6), Action(7)};
  std::unique_ptr<Policy<State, Action>> self_policy =
      std::make_unique<HardCodedPolicy<State, Action>>(std::move(self_moves));
  std::unique_ptr<Policy<State, Action>> opponent_policy =
      std::make_unique<HardCodedPolicy<State, Action>>(
          std::move(opponent_moves));

  MCTS<State, Action> mcts;
  std::unique_ptr<Game<State, Action>> game = std::make_unique<TicTacToe>();

  MCTS<State, Action>::RolloutConfig config;
  config.opponent_goes_first = true;
  config.update_weights = true;

  auto rollout_history = mcts.rollout(game.get(), self_policy.get(),
                                      opponent_policy.get(), config);

  // get the nodes for introspection
  const auto &nodes = mcts.getNodes();

  for (const auto &frame : rollout_history) {
    std::cout << "Verifying assumptions for state: " << std::endl;
    frame.state.render();
    REQUIRE(nodes.find(frame.state) != nodes.end());
    REQUIRE(nodes.at(frame.state).num_rollouts_involved == 1);
    // Each frame should have a positive reward since we lost.
    REQUIRE(nodes.at(frame.state).total_reward_from_here == Approx(1.0));
  }
}

TEST_CASE("Test rewardmap plus operator", "[RewardMap]") {
  RewardMap a = {{0, 1.0}, {1, 2.0}};
  RewardMap b = {{0, 4.0}, {1, -2.0}};

  Rewardmap c = a + b;
  REQUIRE(c.at(0) == Approx(5.0));
  REQUIRE(c.at(1) == Approx(0.0));
}

TEST_CASE("First rollout backprop is working", "[uct]") {
  std::cout << "----------------TESTING UCT ----------------------"
            << std::endl;
  // Make a game where x wins and make sure the tree is updated correctly.
  //
  //  x1, x7, x5
  //      x3, o2
  //  o6,   , o4
  std::vector<Action> moves = {Action(0), Action(5), Action(4), Action(8),
                               Action(2), Action(6), Action(1)};
  std::unique_ptr<Policy<State, Action>> simulation_policy =
      std::make_unique<HardCodedPolicy<State, Action>>(std::move(moves));

  UCT<State, Action> uct;
  std::unique_ptr<Game<State, Action>> game = std::make_unique<TicTacToe>();

  auto rollout_history =
      uct.rollout(game.get(), simulation_policy.get(), /*verbose=*/true);

  // get the nodes for introspection
  const auto &nodes = uct.getNodes();

  for (const auto &frame : rollout_history) {
    std::cout << "Verifying assumptions for state: " << std::endl;
    frame.state.render();
    REQUIRE(nodes.find(frame.state) != nodes.end());
    REQUIRE(nodes.at(frame.state).num_rollouts_involved == 1);
    // Each frame should have a positive reward since we won.
    REQUIRE(nodes.at(frame.state).total_reward_from_here == Approx(1.0));
  }

  // Let's roll it out again?
  {
    std::vector<Action> moves = {Action(0), Action(5), Action(4), Action(8),
                                 Action(2), Action(6), Action(1)};
    std::unique_ptr<Policy<State, Action>> simulation_policy =
        std::make_unique<HardCodedPolicy<State, Action>>(std::move(moves));

    auto rollout_history =
        uct.rollout(game.get(), simulation_policy.get(), /*verbose=*/true);
  }
}
