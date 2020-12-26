#include "tic-tac-toe.h"
#include "mcts.h"

int main() {
    std::unique_ptr<Game<TTTState, TTTAction>> game = std::make_unique<TicTacToe>();

    MCTS<TTTState, TTTAction> mcts;
    for (int i = 0; i < 1000; i++) {
        mcts.rollout(game.get());
    }
    mcts.renderTree();
    return 0;
};
