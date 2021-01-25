#include "tic-tac-toe.h"
#include "uct.h"

typedef TTTState State;
typedef TTTAction Action;

int main() {
  std::unique_ptr<Game<State, Action>> game = std::make_unique<TicTacToe>();

  UCT<State, Action> uct;

  // rollout
  {
    for (int i = 0; i < 4000; i++) {
      uct.rollout(game.get());
    }
  }
  // mcts.renderTree(/*max_depth=*/3);

  game->reset();
  // play against it!
  auto opponent_policy = std::make_unique<UserInputPolicy<State, Action>>();

  // // Let's play against it with us as first player!
  uct.evaluate(game.get(), opponent_policy.get(),
               /*opponent_goes_first=*/false);

  return 0;
};
