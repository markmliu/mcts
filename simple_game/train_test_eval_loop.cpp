#include "matplotlibcpp.h"
#include "mcts.h"
#include "tic-tac-toe.h"
#include "eps_scheduler.h"

typedef TTTState State;
typedef TTTAction Action;

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
    double final_reward = mcts->evaluate(game, opponent_policy.get())
                              .back()
                              .reward;
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

void train_test_plot(EpsilonScheduler* sched) {
  std::unique_ptr<Game<State, Action>> game = std::make_unique<TicTacToe>();
  MCTS<State, Action> mcts;

  // Plot winning percentage over number of training rollouts
  std::vector<double> xs;
  std::vector<double> win_percents;
  std::vector<double> loss_percents;
  std::vector<double> draw_percents;

  int num_training_rollouts = 0;

  const int NUM_ROLLOUTS_PER_TRAIN = 1000;

  xs.push_back((double)num_training_rollouts);
  std::array<double, 3> evaluation =
      evaluateAgainstRandomOpponent(&mcts, game.get());
  win_percents.push_back(evaluation[0]);
  loss_percents.push_back(evaluation[1]);
  draw_percents.push_back(evaluation[2]);

  for (int i = 0; i < 20; i++) {
    mcts.train(game.get(), NUM_ROLLOUTS_PER_TRAIN, sched->getEpsilon());
    std::cout << "finishing training iteration: " << i << std::endl;
    num_training_rollouts += NUM_ROLLOUTS_PER_TRAIN;
    xs.push_back((double)num_training_rollouts);
    std::array<double, 3> evaluation =
        evaluateAgainstRandomOpponent(&mcts, game.get());
    win_percents.push_back(evaluation[0]);
    loss_percents.push_back(evaluation[1]);
    draw_percents.push_back(evaluation[2]);
  }
  plt::named_plot("win_percents_eps_"+sched->name(), xs, win_percents);
  plt::named_plot("loss_percents_"+sched->name(), xs, loss_percents);
  plt::named_plot("draw_percents_"+sched->name(), xs, draw_percents);
}


int main() {
  {
    FixedEpsilonScheduler sched(1.0);
    train_test_plot(&sched);
  }
  {
    FixedEpsilonScheduler sched(0.05);
    train_test_plot(&sched);
  }
  plt::title("Tic-tac-toe MCTS performance against random opponent");
  plt::xlabel("Number of rollouts");
  plt::legend();
  plt::save("./training_curve.png");
};
