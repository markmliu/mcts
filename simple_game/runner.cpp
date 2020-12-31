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
  mcts.renderTree(/*max_depth=*/3);

  game->reset();
  // play against it!
  auto opponent_policy = std::make_unique<UserInputPolicy<State, Action>>();
  mcts.rollout(game.get(), &mcts, opponent_policy.get());

  return 0;
};
