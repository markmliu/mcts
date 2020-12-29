#include <optional>
#include <array>
#include <string>

#include "game.h"


struct TTTState {
    TTTState();
    void render() const;
    // each square can be 'x', 'o' or '_'
    std::array<char, 9> board;
    // start off as x's turn, flip b/w x and o.
    bool x_turn = true;
};

struct TTTAction {
    TTTAction(int board_position_);

    bool operator<(const TTTAction& rhs) const {
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
    double simulate(TTTAction action) override;
    std::vector<TTTAction> getValidActions() const override;
    const TTTState& getCurrentState() const override;
    bool isOurTurn() const override;
    bool isTerminal() const override;
    void render() const override;
    ~TicTacToe() = default;
private:
    TTTState state;
};
