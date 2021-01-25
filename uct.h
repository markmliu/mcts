#ifndef MCTS_UCT
#define MCTS_UCT

#include "game.h"
#include "policy.h"

#include <iostream>
#include <map>
#include <math.h>
#include <memory>
#include <optional>
#include <queue>

template <class State, class Action> class UCT {
public:
  // exploration param, approx sqrt(2)
  static constexpr double C = 1.41;

  // Node stores statistics of games played starting from a given state.
  // total_reward is relative to the turn in the state.
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
    HistoryFrame(std::optional<Action> action_, double reward_,
                 const State &state_)
        : action(action_), reward(reward_), state(state_) {}
    // Action that created this node or nullopt for root node.
    std::optional<Action> action;
    // Reward after taking action
    double reward;
    // State after taking action
    State state;
  };

  struct RolloutConfig {
    bool update_weights;
    bool opponent_goes_first;
    bool verbose = false;
  };

  UCT() {
    nodes_.insert(std::make_pair(State(), Node(State())));
    root_ = &(nodes_.at(State()));
  }

  // Create a node corresponding to the current game state and link it as a
  // child of parent_node
  Node &getOrCreateNode(const Game<State, Action> *const game,
                        const Action action, Node &parent_node) {
    const State &state = game->getCurrentState();
    if (nodes_.find(state) == nodes_.end()) {
      nodes_.emplace(state, Node(state));
    }

    if (parent_node.children.find(action) == parent_node.children.end()) {
      parent_node.children.insert(std::make_pair(action, &nodes_.at(state)));
    }

    return nodes_.at(state);
  }

  Node &getNode(const Game<State, Action> *const game) {
    // Should remove this assert once we are sure in logic.
    assert(nodes_.find(game->getCurrentState()) != nodes_.end());
    return nodes_.at(game->getCurrentState());
  }

  // Rolls out a game, playing both players.
  std::vector<HistoryFrame> rollout(Game<State, Action> *game) {
    game->reset();

    std::vector<HistoryFrame> rollout_history;
    // Start with the initial board in the rollout history always.
    rollout_history.emplace_back(std::nullopt, 0.0, game->getCurrentState());

    // 1. Selection - recursively choose best child node using UCB until we hit
    // a leaf node.
    // 2. Expansion - Since getBestActionIdx will return the first non-explored
    // child node, this does the expansion phase as well.
    Node &cur_node = *root_;
    while (!cur_node.children.empty()) {
      int selected_action_idx = getBestActionIdx(game, cur_node);
      const Action chosen_action =
          game->getValidActions().at(selected_action_idx);
      int player_turn = game->turn();
      double reward = game->simulate(chosen_action).at(player_turn);

      rollout_history.emplace_back(chosen_action, reward,
                                   game->getCurrentState());
      cur_node = getNode(game);
    }

    bool need_to_update_cur_node = !game->isTerminal();

    // 3. Simulation
    if (need_to_update_cur_node) {
      // Do a random simulation from here to terminal state to get a reward for
      // this node, without storing any of it in the rollout history. Could also
      // do a light playout instead.
      RandomValidPolicy<State, Action> random_policy;
      double total_reward_from_leaf = 0.0;
      while (!game->isTerminal()) {
        const Action action = random_policy.act(game);
        int player_turn = game->turn();
        double reward = game->simulate(action).at(player_turn);
        total_reward_from_leaf += reward;
      }
      // Use received reward as a proxy for the reward from the earlier leaf
      // node. Modify what we've stored in the rollout history to include what
      // we think we should receive from here on out.
      rollout_history.back().reward += total_reward_from_leaf;
    }

    // 4. Backpropagation.
    // Iterate through frame history backward to easily compute "total reward
    // received from this node onward".
    double reward_from_here_for_rollout = 0.0;
    for (auto rit = rollout_history.rbegin(); rit != rollout_history.rend();
         ++rit) {
      // All nodes should exist already
      const auto &frame = *rit;
      Node &node = nodes_.at(frame.state);
      node.num_rollouts_involved++;
      reward_from_here_for_rollout += frame.reward;
      node.total_reward_from_here += reward_from_here_for_rollout;
    }

    // reset the game to be a good citizen :)
    game->reset();
    return rollout_history;
  }

  // Should only be called if current_node has at least one child.
  // If it does, we should have at least one simulation going through here.
  // Goes through all possible actions and returns best UCB value, returning the
  // index of any unexplored actions first.
  int getBestActionIdx(const Game<State, Action> *game, Node &current_node) {

    const std::vector<Action> &valid_actions = game->getValidActions();
    assert(!valid_actions.empty());
    assert(current_node.num_rollouts_involved != 0);

    const State &current_state = game->getCurrentState();

    double best_ucb_so_far = std::numeric_limits<double>::lowest();
    double best_idx_so_far = -1;

    for (int i = 0; i < valid_actions.size(); i++) {
      const Action &action = valid_actions.at(i);
      const std::pair<State, RewardMap> state_reward =
          game->simulateDry(current_state, action);
      const Node &child_node = getOrCreateNode(game, action, current_node);
      double ucb = getUcb(child_node.total_reward_from_here,
                          child_node.num_rollouts_involved,
                          current_node.num_rollouts_involved);
      if (ucb > best_ucb_so_far) {
        best_ucb_so_far = ucb;
        best_idx_so_far = i;
        // Return early if we have infinity
        if (ucb == std::numeric_limits<double>::max()) {
          return i;
        }
      }
    }
    return best_idx_so_far;
  }

  // Used only for evaluation
  Action actGreedily(const Game<State, Action> *game) {
    const State &current_state = game->getCurrentState();
    const std::vector<Action> &valid_actions = game->getValidActions();
    assert(!valid_actions.empty());
    int best_idx = 0;
    double best_value = std::numeric_limits<double>::lowest();
    for (int i = 0; i < valid_actions.size(); i++) {
      const Action &action = valid_actions.at(i);
      const std::pair<State, RewardMap> state_reward =
          game->simulateDry(current_state, action);
      if (nodes_.find(state_reward.first) == nodes_.end()) {
        // Don't try anything we don't haven't tried before.
        continue;
      }
      const Node &child_node = nodes_.at(state_reward.first);
      // Shouldn't have any nodes created with zero rollouts
      assert(child_node.num_rollouts_involved != 0);
      double value = (child_node.total_reward_from_here /
                      child_node.num_rollouts_involved);
      if (value > best_value) {
        best_value = value;
        best_idx = i;
      }
    }
    return valid_actions.at(best_idx);
  }

  // Use this to play against the UCT
  void evaluate(Game<State, Action> *game,
                Policy<State, Action> *opponent_policy,
                bool opponent_goes_first) {

    const int player_num = opponent_goes_first ? 1 : 0;

    game->reset();
    double final_reward;
    while (!game->isTerminal()) {
      Action action = [&]() {
        if (game->turn() == player_num) {
          // Choose best action
          return actGreedily(game);
        } else {
          return opponent_policy->act(game);
        }
      }();

      // As opposed to normal rollout, here we keep track of reward for policy
      // on opponent turns..
      final_reward = game->simulate(action).at(player_num);
      game->render();
    }

    if (final_reward == 1.0) {
      std::cout << "mcts won!" << std::endl;
    } else if (final_reward == -1.0) {
      std::cout << "opponent won!" << std::endl;
    } else {
      std::cout << "it's a draw!" << std::endl;
    }

    game->reset();
  }

private:
  double getUcb(double child_total_reward, int child_num_rollouts,
                int parent_num_rollouts) {
    assert(parent_num_rollouts > 0);
    assert(child_num_rollouts <= parent_num_rollouts);

    if (child_num_rollouts == 0) {
      return std::numeric_limits<double>::max();
    }

    double expected_reward = child_total_reward / (double)child_num_rollouts;
    double exploration_term =
        C * sqrt(log((double)parent_num_rollouts) / (double)child_num_rollouts);

    return expected_reward + exploration_term;
  }

  std::map<State, Node> nodes_;
  Node *root_;
};

#endif // MCTS_UCT
