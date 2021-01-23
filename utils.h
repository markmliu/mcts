#ifndef MCTS_UTILS
#define MCTS_UTILS

#include "mcts.h"
#include <array>

// Returns win/loss/draw percentage
std::array<double, 3> evaluateAgainstRandomOpponent(MCTS<State, Action> *mcts,
                                                    Game<State, Action> *game) {
  auto opponent_policy = std::make_unique<RandomValidPolicy<State, Action>>();

  const int num_runs = 300;
  int num_wins = 0;
  int num_losses = 0;
  int num_draws = 0;

  for (int i = 0; i < num_runs; i++) {
    // Infer game type from reward.
    double final_reward =
        mcts->evaluate(game, opponent_policy.get()).back().reward;
    if (final_reward == 1.0) {
      num_wins++;
    } else if (final_reward == 0.0) {
      num_draws++;
    } else {
      assert(final_reward == -1.0);
      num_losses++;
    }
  }
  return {(double)num_wins / num_runs, (double)num_losses / num_runs,
          (double)num_draws / num_runs};
}

namespace plt = matplotlibcpp;

#endif // MCTS_UTILS
