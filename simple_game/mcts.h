#ifndef MCTS_MCTS
#define MCTS_MCTS
#include "game.h"
#include "policy.h"

#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <random>

// reward for a state which we haven't explored yet. Higher number here will
// result in a more optimistic policy.
constexpr double UNEXPLORED_STATE_REWARD = 0.0;

template <class State, class Action> class MCTS : public Policy<State, Action> {
public:
  struct Node {
    Node(const State &state_)
        : num_rollouts_involved(0), total_reward_from_here(0), state(state_) {}
    int num_rollouts_involved;
    double total_reward_from_here;
    std::map<Action, Node *> children;
    // let's store the board in the node as well for visualization.
    State state;
  };

  // Vector of these can be used to store history of a rollout.
  struct HistoryFrame {
    HistoryFrame(Action action_, double reward_, const State &state_)
        : action(action_), reward(reward_), state(state_) {}
    // Action that created this node
    Action action;
    // Reward after taking action
    double reward;
    // State after taking action
    State state;
  };

  MCTS() {
    nodes_.insert(std::make_pair(State(), Node(State())));
    root_ = &(nodes_.at(State()));
    eng_ = std::default_random_engine(rd_()); // seed the generator
    distr_ = std::uniform_real_distribution<float>(0.0, 1.0);
  }

  // training with eps-greedy policy for self.
  // TODO: the way training works right now, if we train as x's, we will be
  // learn to throw really hard if we play as o's. Should we fix this by
  // augmenting the state with the player number? Or is there a more elegant way
  // to invert the reward?
  void train(Game<State, Action> *game, int num_rollouts = 1, double eps = 1.0,
             std::unique_ptr<Policy<State, Action>> opponent_policy =
                 std::make_unique<RandomValidPolicy<State, Action>>(),
             bool opponent_goes_first = false, bool verbose = false) {
    // eps is the fraction of the time that we choose random policy.
    assert(eps <= 1.0 && eps >= 0.0);
    eps_ = eps;
    RolloutConfig config;
    config.update_weights = true;
    config.verbose = verbose;
    config.opponent_goes_first = opponent_goes_first;
    for (int i = 0; i < num_rollouts; i++) {
      // pass self as policy
      rollout(game, this, opponent_policy.get(), config);
    }
  }

  std::vector<HistoryFrame> evaluate(Game<State, Action> *game,
                                     Policy<State, Action> *opponent_policy,
                                     bool opponent_goes_first, bool verbose) {
    // when we're evaluating the strength of mcts, we want to be totally greedy
    eps_ = 0.0;
    RolloutConfig config;
    config.update_weights = false;
    config.verbose = verbose;
    config.opponent_goes_first = opponent_goes_first;
    return rollout(game, this, opponent_policy, config);
  }

  struct RolloutConfig {
    bool update_weights;
    bool opponent_goes_first;
    bool verbose = false;
  };

  // Simulate a rollout with 'self_policy' and 'opponent_policy'.
  // If 'update_weights' is true, keep track of reward and update tree to
  // reflect it.
  std::vector<HistoryFrame> rollout(Game<State, Action> *game,
                                    Policy<State, Action> *self_policy,
                                    Policy<State, Action> *opponent_policy,
                                    const RolloutConfig &config) {
    // As we simulate, we want to update the game tree. Each node of the tree
    // stores:
    // - how many rollouts have passed through this node
    // - the total reward of games that have passed through this node.

    // let's assume for now that the reward will only come at a terminal state.

    // 1. Do a playthrough, keeping track of the actions that were played.
    game->reset();
    verbose_ = config.verbose;

    std::vector<HistoryFrame> rollout_history;

    const int player_num = config.opponent_goes_first ? 1 : 0;

    while (!game->isTerminal()) {
      Action action = [&]() {
        if (game->turn() == player_num) {
          return self_policy->act(game);
        } else {
          return opponent_policy->act(game);
        }
      }();

      // TODO: Should we really be using the reward and learning from both our
      // own and opponent's actions?
      double reward = game->simulate(action).at(player_num);
      // TODO: wrap this in a toggle-able logger
      if (verbose_) {
        game->render();
      }
      rollout_history.emplace_back(action, reward, game->getCurrentState());
    }

    if (verbose_) {
      const double final_reward = rollout_history.back().reward;
      if (final_reward == 1.0) {
        std::cout << "mcts won!" << std::endl;
      } else if (final_reward == -1.0) {
        std::cout << "opponent won!" << std::endl;
      } else {
        std::cout << "it's a draw!" << std::endl;
      }
    }

    if (!config.update_weights) {
      return rollout_history;
    }

    // 2. Go through the rollout history and update node values for each one.
    double total_rollout_reward = std::accumulate(
        rollout_history.begin(), rollout_history.end(), 0.0,
        [&](double a, const HistoryFrame &el) { return a + el.reward; });

    if (verbose_) {
      std::cout << "total reward: " << total_rollout_reward << std::endl;
    }
    auto updateNode = [&](Node *current) {
      current->num_rollouts_involved++;
      current->total_reward_from_here += total_rollout_reward;
    };

    Node *current = root_;
    updateNode(current);

    for (const auto &frame : rollout_history) {
      // Create the node if it doesn't exist.
      if (nodes_.find(frame.state) == nodes_.end()) {
        nodes_.insert(std::make_pair(frame.state, Node(frame.state)));
      }

      // Update the parent node to point to the newly created node, if it does
      // not already.
      const Action &action = frame.action;
      if (current->children.find(action) == current->children.end()) {
        current->children.insert(
            std::make_pair(action, &nodes_.at(frame.state)));
      }

      Node &next = nodes_.at(frame.state);
      updateNode(&next);
      current = &next;
    }

    // reset the game to be a good citizen :)
    game->reset();
    return rollout_history;
  }

  void renderTree(int max_depth) {
    // how to display the tree? maybe with a BFS
    std::queue<std::pair<int, const Node *>> queue;
    queue.push(std::make_pair(0, root_));
    while (!queue.empty()) {
      std::pair<int, const Node *> depth_top = queue.front();
      int depth = depth_top.first;
      const Node *top = depth_top.second;

      if (depth > max_depth) {
        break;
      }

      queue.pop();
      top->state.render();
      std::cout << "num rollouts: " << top->num_rollouts_involved << std::endl;
      std::cout << "reward: " << top->total_reward_from_here << std::endl;
      for (const auto &action_child : top->children) {
        const Node *child = action_child.second;
        queue.push(std::make_pair(depth + 1, child));
      }
    }
  }

  // Greedily choose "best" action
  Action act(const Game<State, Action> *game) override {
    assert(!game->isTerminal());
    std::vector<Action> valid_actions = game->getValidActions();
    assert(!valid_actions.empty());

    // eps_ fraction of the time, act randomly.
    float chance = distr_(eng_);
    if (chance < eps_) {
      // do something random!
      return random_policy_.act(game);
    } else {

      // find the "best" state
      int best_idx = getBestActionIdx(valid_actions, game);
      assert(best_idx >= 0);
      return valid_actions[best_idx];
    }
  };

  int getBestActionIdx(const std::vector<Action> &valid_actions,
                       const Game<State, Action> *game) {
    const State &current_state = game->getCurrentState();
    double best_value_seen = std::numeric_limits<double>::lowest();
    int best_idx = -1;
    for (int i = 0; i < valid_actions.size(); i++) {
      const Action &action = valid_actions[i];
      // TODO: should i be using the reward from the dry simulation here? Right
      // now I'm just using the estimated value from my value function.
      const std::pair<State, RewardMap> state_reward =
          game->simulateDry(current_state, action);
      double state_value = getExpectedReward(state_reward.first);
      if (verbose_) {
        std::cout << "Action " << action.toString()
                  << " has expected reward: " << state_value << std::endl;
      }
      if (state_value > best_value_seen) {
        best_idx = i;
        best_value_seen = state_value;
      }
    }
    return best_idx;
  };

  // For introspection
  const std::map<State, Node> &getNodes() { return nodes_; }

private:
  double getExpectedReward(const State &state) {
    if (nodes_.find(state) == nodes_.end()) {
      return UNEXPLORED_STATE_REWARD;
    }
    const Node &node = nodes_.at(state);
    // If a state gets constructed, it probably should have at least one
    // rollout.
    assert(node.num_rollouts_involved != 0);
    return (node.total_reward_from_here / node.num_rollouts_involved);
  }

  // When running this->act(), use eps-greedy policy.
  // TODO: don't like that eps_ is a stateful thing, let's remove this if
  // possible. same with verbose_.
  double eps_;
  bool verbose_;
  std::random_device rd_; // obtain a random number from hardware
  std::default_random_engine eng_;
  std::uniform_real_distribution<float> distr_;
  RandomValidPolicy<State, Action> random_policy_;

  std::map<State, Node> nodes_;
  Node *root_;
};

#endif // MCTS_MCTS
