#include "assert.h"
#include <iostream>
#include <algorithm>

#include "tic-tac-toe.h"

namespace {
bool isThreeInARow(const std::array<char, 9>& board, char c) {
    std::vector<std::array<int, 3>> winning_lines = {{0, 3, 6},
                                                     {1, 4, 7},
                                                     {2, 5, 8},
                                                     {0, 1, 2},
                                                     {3, 4, 5},
                                                     {6, 7, 8},
                                                     {0, 4, 8},
                                                     {2, 4, 6}};
    // we need there to be at least one line where all the positions are 'c'.
    for (const auto& line : winning_lines) {
        bool line_broken = false;
        for (const int pos : line) {
            if (board[pos] != c) {
                line_broken = true;
                break;
            }
        }
        // every pos in line is 'c'
        if (!line_broken) {
            std::cout << "game over on indices: " << line[0] << "," << line[1] << "," << line[2] << std::endl;
            return true;
        }
    }
    // no line has all positions as 'c'
    return false;
}

int numFreeSpaces(const std::array<char, 9>& board) {
    return std::count_if(board.begin(), board.end(), [](char c) {return c == '_';});
}
} // namespace

TTTState::TTTState() :
    board({'_','_','_','_','_','_','_','_','_'}),
    x_turn(true)
{
}

void TTTState::render() const {
    std::cout << board[0] << "," << board[1] << "," << board[2] << std::endl;
    std::cout << board[3] << "," << board[4] << "," << board[5] << std::endl;
    std::cout << board[6] << "," << board[7] << "," << board[8] << std::endl;
    std::cout << "_____________________________________" << std::endl;
}

TTTAction::TTTAction(int board_position_): board_position(board_position_)
{
}

TicTacToe::TicTacToe() {}

void TicTacToe::reset() {
    state = TTTState();
}

double TicTacToe::simulate(TTTAction action) {
    // Must place at empty square
    assert(state.board[action.board_position] == '_');

    char char_to_place = state.x_turn ? 'x' : 'o';
    state.board[action.board_position] = char_to_place;
    state.x_turn = !state.x_turn;

    if (isThreeInARow(state.board, 'x')) {
        return 1.0;
    } else if (isThreeInARow(state.board, 'o')) {
        return -1.0;
    }
    return 0.0;
}

std::vector<TTTAction> TicTacToe::getValidActions() {
    std::vector<TTTAction> valid_actions;
    for (int i = 0; i < 9; i++) {
        if (state.board[i] == '_') {
            valid_actions.push_back(TTTAction(i));
        }
    }
    return valid_actions;
}

const TTTState& TicTacToe::getCurrentState() {
    return state;
}

bool TicTacToe::isOurTurn() {
    return state.x_turn;
}

bool TicTacToe::isTerminal() {
    bool is_terminal =  isThreeInARow(state.board, 'x')
        || isThreeInARow(state.board, 'o')
        || numFreeSpaces(state.board) == 0;
    std::cout << "game is terminal: " << is_terminal << std::endl;
    return is_terminal;
}

void TicTacToe::render() {
    state.render();
}
