#ifndef MCTS_TIC_TAC_TOE
#define MCTS_TIC_TAC_TOE

#include <array>
#include <optional>
#include <string>

#include "game.h"

struct TTTState {
  TTTState();
  std::string render() const;
  // each square can be 'x', 'o' or '_'
  std::array<char, 9> board;
  // start off as x's turn, flip b/w x and o.
  bool x_turn = true;

  bool operator<(const TTTState &rhs) const {
    return std::tie(board, x_turn) < std::tie(rhs.board, rhs.x_turn);
  }

  int getTurn() const {
    if (x_turn) {
      return 0;
    }
    return 1;
  }
};

struct TTTAction {
  TTTAction(int board_position_);

  // Needed for use as key in map
  bool operator<(const TTTAction &rhs) const {
    return board_position < rhs.board_position;
  }

  std::string toString() const {
    return "Board position: " + std::to_string(board_position);
  }

  // board position to play at.
  int board_position;
};

class TicTacToe : public Game<TTTState, TTTAction> {
public:
  TicTacToe();
  void reset() override;
  RewardMap simulate(const TTTAction &action) override;
  std::pair<TTTState, RewardMap>
  simulateDry(const TTTState &state, const TTTAction &action) const override;
  std::vector<TTTAction> getValidActions() const override;
  const TTTState &getCurrentState() const override;
  int turn() const override;
  bool isTerminal() const override;
  std::string render() const override;
  ~TicTacToe() = default;

private:
  TTTState state_;
};

#endif // MCTS_TIC_TAC_TOE
