#ifndef MCTS_GAME
#define MCTS_GAME

#include <map>
#include <vector>

typedef std::map<int, double> RewardMap;

const RewardMap TwoPlayerFirstPlayerWinsReward = {{0, 1.0}, {1, -1.0}};
const RewardMap TwoPlayerSecondPlayerWinsReward = {{0, -1.0}, {1, 1.0}};
const RewardMap TwoPlayerNobodyWinsReward = {{0, 0.0}, {1, 0.0}};

// Game should tell you all valid moves at any state.
template <class State, class Action> class Game {
public:
  virtual void reset() = 0;
  // returns a reward for each player
  virtual RewardMap simulate(const Action &a) = 0;
  // Like above, but doesn't actually modify state
  virtual std::pair<State, RewardMap> simulateDry(const State &state,
                                                  const Action &a) const = 0;
  virtual std::vector<Action> getValidActions() const = 0;
  virtual const State &getCurrentState() const = 0;

  // Returns player number whose turn it is. For two player games, numbers are 0
  // and 1.
  virtual int turn() const = 0;
  virtual bool isTerminal() const = 0;
  virtual void render() const = 0;
  virtual ~Game() = default;
};

#endif // MCTS_GAME
