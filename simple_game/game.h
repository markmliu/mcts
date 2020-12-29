#ifndef MCTS_GAME
#define MCTS_GAME

#include <vector>

// Game should tell you all valid moves at any state.
template <class State, class Action> class Game {
public:
  virtual void reset() = 0;
  virtual double simulate(Action a) = 0;
  virtual std::vector<Action> getValidActions() const = 0;
  virtual const State &getCurrentState() const = 0;

  // Returns true if our turn, false if opponents turn.
  virtual bool isOurTurn() const = 0;
  virtual bool isTerminal() const = 0;
  virtual void render() const = 0;
  virtual ~Game() = default;
};

#endif // MCTS_GAME
