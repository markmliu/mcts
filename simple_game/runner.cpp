#include "mcts.h"
#include "tic-tac-toe.h"

typedef TTTState State;
typedef TTTAction Action;

int main() {
  std::unique_ptr<Game<State, Action>> game = std::make_unique<TicTacToe>();

  MCTS<State, Action> mcts;

  // train
  for (int i = 0; i < 10000; i++) {
    mcts.train(game.get());
  }
  // mcts.renderTree(/*max_depth=*/3);

  game->reset();
  // play against it!
  auto opponent_policy = std::make_unique<UserInputPolicy<State, Action>>();

  // Let's play against it with us as first player!
  MCTS<State, Action>::RolloutConfig config;
  config.update_weights = false;
  config.verbose = true;
  config.opponent_goes_first = true;
  // TODO: Use "evaluate" instead of rollout or "eps" maay be set to non-zero.
  mcts.rollout(game.get(), &mcts, opponent_policy.get(), config);

  return 0;
};
