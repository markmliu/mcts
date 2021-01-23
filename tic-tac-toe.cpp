#include "assert.h"
#include <algorithm>
#include <iostream>

#include "tic-tac-toe.h"

namespace {
bool isThreeInARow(const std::array<char, 9> &board, char c) {
  std::vector<std::array<int, 3>> winning_lines = {
      {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 1, 2},
      {3, 4, 5}, {6, 7, 8}, {0, 4, 8}, {2, 4, 6}};
  // we need there to be at least one line where all the positions are 'c'.
  for (const auto &line : winning_lines) {
    bool line_broken = false;
    for (const int pos : line) {
      if (board[pos] != c) {
        line_broken = true;
        break;
      }
    }
    // every pos in line is 'c'
    if (!line_broken) {
      return true;
    }
  }
  // no line has all positions as 'c'
  return false;
}

int numFreeSpaces(const std::array<char, 9> &board) {
  return std::count_if(board.begin(), board.end(),
                       [](char c) { return c == '_'; });
}
} // namespace

TTTState::TTTState()
    : board({'_', '_', '_', '_', '_', '_', '_', '_', '_'}), x_turn(true) {}

void TTTState::render() const {
  auto char_to_display = [](char c, int idx) {
    if (c == 'x' || c == 'o')
      return std::string(1, c);
    return std::to_string(idx);
  };
  std::cout << char_to_display(board[0], 0) << ","
            << char_to_display(board[1], 1) << ","
            << char_to_display(board[2], 2) << std::endl;
  std::cout << char_to_display(board[3], 3) << ","
            << char_to_display(board[4], 4) << ","
            << char_to_display(board[5], 5) << std::endl;
  std::cout << char_to_display(board[6], 6) << ","
            << char_to_display(board[7], 7) << ","
            << char_to_display(board[8], 8) << std::endl;
  std::cout << "_____________________________________" << std::endl;
}

TTTAction::TTTAction(int board_position_) : board_position(board_position_) {}

TicTacToe::TicTacToe() {}

void TicTacToe::reset() { state_ = TTTState(); }

RewardMap TicTacToe::simulate(const TTTAction &action) {
  // Must place at empty square
  assert(state_.board[action.board_position] == '_');

  char char_to_place = state_.x_turn ? 'x' : 'o';
  state_.board[action.board_position] = char_to_place;
  state_.x_turn = !state_.x_turn;

  if (isThreeInARow(state_.board, 'x')) {
    return TwoPlayerFirstPlayerWinsReward;
  } else if (isThreeInARow(state_.board, 'o')) {
    return TwoPlayerSecondPlayerWinsReward;
  }
  return TwoPlayerNobodyWinsReward;
}

// TODO: can we avoid code duplication between simulate and simulateDry without
// extra copies for simulate?
std::pair<TTTState, RewardMap>
TicTacToe::simulateDry(const TTTState &state, const TTTAction &action) const {
  // Must place at empty square
  assert(state.board[action.board_position] == '_');

  char char_to_place = state.x_turn ? 'x' : 'o';

  TTTState updated_state = state;
  updated_state.board[action.board_position] = char_to_place;
  updated_state.x_turn = !state.x_turn;

  if (isThreeInARow(state.board, 'x')) {
    return std::make_pair(updated_state, TwoPlayerFirstPlayerWinsReward);
  } else if (isThreeInARow(state.board, 'o')) {
    return std::make_pair(updated_state, TwoPlayerSecondPlayerWinsReward);
  }
  return std::make_pair(updated_state, TwoPlayerNobodyWinsReward);
}

std::vector<TTTAction> TicTacToe::getValidActions() const {
  std::vector<TTTAction> valid_actions;
  for (int i = 0; i < 9; i++) {
    if (state_.board[i] == '_') {
      valid_actions.push_back(TTTAction(i));
    }
  }
  return valid_actions;
}

const TTTState &TicTacToe::getCurrentState() const { return state_; }

int TicTacToe::turn() const {
  if (state_.x_turn)
    return 0;
  return 1;
}

bool TicTacToe::isTerminal() const {
  bool is_terminal = isThreeInARow(state_.board, 'x') ||
                     isThreeInARow(state_.board, 'o') ||
                     numFreeSpaces(state_.board) == 0;
  return is_terminal;
}

void TicTacToe::render() const { state_.render(); }
