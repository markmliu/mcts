#include "tic-tac-toe.h"
#include "agent.h"

int main() {
    std::unique_ptr<Game<TTTState, TTTAction>> game = std::make_unique<TicTacToe>();
    // Agent<TTTState, TTTAction> agent;
    // agent.play(game.get());

    MCTS<TTTState, TTTAction> mcts;
    mcts.rollout(game.get());

    return 0;
};
