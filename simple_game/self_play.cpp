#include "eps_scheduler.h"

// Let's try to do some self-play!
#include "mcts.h"
#include "tic-tac-toe.h"

#include <iostream>

typedef TTTState State;
typedef TTTAction Action;

int main() {
  // Let's seed a first player tree by playing against randoms
  std::unique_ptr<Game<State, Action>> game = std::make_unique<TicTacToe>();
  MCTS<State, Action> first_player_mcts;
  {
    auto opponent_policy = std::make_unique<RandomValidPolicy<State, Action>>();
    FixedEpsilonScheduler sched(0.05);
    first_player_mcts.train(game.get(), opponent_policy.get(), 20000,
                            sched.getEpsilon(),
                            /*opponent_goes_first*/ false);
    std::cout << "finished training first player tree." << std::endl;
  }

  MCTS<State, Action> second_player_mcts;
  {
    // For some reason the second player learns a lot better with eps = 1.0
    auto opponent_policy = std::make_unique<RandomValidPolicy<State, Action>>();
    FixedEpsilonScheduler sched(1.0);
    second_player_mcts.train(game.get(), opponent_policy.get(), 20000,
                             sched.getEpsilon(),
                             /*opponent_goes_first*/ true);
    std::cout << "finished training second player tree." << std::endl;
  }

  // Make them play each other and learn from each other?

  const int NUM_ROLLOUTS_PER_SELF_PLAY_EPOCH = 2000;
  for (int i = 0; i < 10; i++) {
    bool training_first_player = i % 2 == 0;
    std::cout << "self play iteration: " << i << std::endl;
    // trainee will learn from playing the trainer.
    MCTS<State, Action> *trainee =
        training_first_player ? &first_player_mcts : &second_player_mcts;
    MCTS<State, Action> *trainer =
        training_first_player ? &second_player_mcts : &first_player_mcts;

    bool opponent_goes_first = !training_first_player;

    trainee->train(game.get(), trainer, NUM_ROLLOUTS_PER_SELF_PLAY_EPOCH,
                   /*eps=*/0.05, opponent_goes_first);
  }

  // play against the second player tree

  auto opponent_policy = std::make_unique<UserInputPolicy<State, Action>>();
  std::cout << "Play a game? ;) (y/n)" << std::endl;
  char play;
  std::cin >> play;
  while (play == 'y') {

    second_player_mcts.evaluate(game.get(), opponent_policy.get(),
                                /*opponent_goes_first*/ true,
                                /*verbose=*/true);
    std::cout << "Play a game? ;) (y/n)" << std::endl;
    std::cin >> play;
  }
  std::cout << "Good games!" << std::endl;
}
