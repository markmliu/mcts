#ifndef TTT_GAME
#define TTT_GAME

#include <vector>

// Game should tell you all valid moves at any state.
template <class State, class Action, class Outcome>
class Game {
public:
    virtual const State& simulate(Action a) = 0;
    virtual std::vector<Action> getValidActions() = 0;
    virtual bool isTerminal() = 0;
    virtual ~Game() = default;
};

#endif
