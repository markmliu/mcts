#include "tic-tac-toe.h"
#include "uct.h"

typedef TTTState State;
typedef TTTAction Action;

int main() {
  std::unique_ptr<Game<State, Action>> game = std::make_unique<TicTacToe>();
  std::unique_ptr<Policy<State, Action>> random_policy =
      std::make_unique<RandomValidPolicy<State, Action>>();
  UCT<State, Action> uct;

  // rollout
  {
    for (int i = 0; i < 10000; i++) {
      if (i % 100 == 0) {
        std::cout << "rollout iteration: " << i << std::endl;
      }
      uct.rollout(game.get(), random_policy.get());
    }
  }
  // mcts.renderTree(/*max_depth=*/3);

  game->reset();
  // play against it!
  auto opponent_policy = std::make_unique<UserInputPolicy<State, Action>>();

  // // Let's play against it with us as first player!
  std::cout << "Play a game? ;) (y/n)" << std::endl;
  char play;
  std::cin >> play;
  while (play == 'y') {
    uct.evaluate(game.get(), opponent_policy.get(),
                 /*opponent_goes_first*/ true);
    std::cout << "Play a game? ;) (y/n)" << std::endl;
    std::cin >> play;
  }
  std::cout << "Good games!" << std::endl;

  return 0;
};
