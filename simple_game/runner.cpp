#include "tic-tac-toe.h"
#include "mcts.h"

int main() {
    std::unique_ptr<Game<TTTState, TTTAction>> game = std::make_unique<TicTacToe>();

    MCTS<TTTState, TTTAction> mcts;

    // train
    for (int i = 0; i < 10000; i++) {
        mcts.rollout(game.get());
    }
    mcts.renderTree(/*max_depth=*/3);

    return 0;
};
