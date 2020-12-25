#include "tic-tac-toe.h"
#include "agent.h"

int main() {
    std::unique_ptr<Game<TTTState, TTTAction, TTTOutcome>> game = std::make_unique<TicTacToe>();
    Agent<TTTState, TTTAction, TTTOutcome> agent;
    agent.play(game.get());
    return 0;
};
