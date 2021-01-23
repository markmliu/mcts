#include "mcts.h"
#include "tic-tac-toe.h"

typedef TTTState State;
typedef TTTAction Action;

int main() {
  std::unique_ptr<Game<State, Action>> game = std::make_unique<TicTacToe>();

  MCTS<State, Action> mcts;

  // train
  {
    auto opponent_policy = std::make_unique<RandomValidPolicy<State, Action>>();
    for (int i = 0; i < 10000; i++) {
      mcts.train(game.get(), opponent_policy.get());
    }
  }
  // mcts.renderTree(/*max_depth=*/3);

  game->reset();
  // play against it!
  auto opponent_policy = std::make_unique<UserInputPolicy<State, Action>>();

  // Let's play against it with us as first player!
  mcts.evaluate(game.get(), opponent_policy.get(),
                /*opponent_goes_first=*/false, /*verbose=*/true);

  return 0;
};
