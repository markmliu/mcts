#include <optional>
#include <array>

#include "game.h"


struct TTTState {
    TTTState();

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

    // board position to play at.
    int board_position;
};


class TicTacToe : public Game<TTTState, TTTAction> {
public:
    TicTacToe();
    void reset() override;
    double simulate(TTTAction action) override;
    std::vector<TTTAction> getValidActions() override;
    bool isOurTurn() override;
    bool isTerminal() override;
    void render() override;
    ~TicTacToe() = default;
private:
    TTTState state;
};
