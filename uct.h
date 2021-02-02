#ifndef MCTS_UCT
#define MCTS_UCT

#include "debug_logger.h"
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
  // total_reward stores the reward for each player for all games starting from
  // here
  struct Node {
    Node(const State &state_)
        : num_rollouts_involved(0),
          total_reward_from_here({{0, 0.0}, {1, 0.0}}), state(state_) {}
    int num_rollouts_involved;
    RewardMap total_reward_from_here;
    std::map<Action, Node *> children;
    // let's store the board in the node as well for visualization.
    State state;
  };

  // Vector of these can be used to store history of a rollout.
  struct HistoryFrame {
    HistoryFrame(std::optional<Action> action_, RewardMap reward_,
                 const State &state_, int player_num_)
        : action(action_), reward(reward_), state(state_),
          player_num(player_num_) {}
    // Action that created this node or nullopt for root node.
    std::optional<Action> action;
    // Reward after taking action
    RewardMap reward;
    // State after taking action
    State state;
    // player number who took the action
    int player_num;
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
  Node &getOrCreateNode(const State &state, const Action action,
                        Node &parent_node) {
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
  // For each rollout, we first do selection of nodes using UCB until we hit a
  // node that we haven't explored before.
  //
  // Once we've hit such a node,
  //
  // simulation_policy is used to simulate both players once we reach simulation
  // stage. a random policy is usually fine for this.
  std::vector<HistoryFrame> rollout(Game<State, Action> *game,
                                    Policy<State, Action> *simulation_policy,
                                    bool verbose = false) {
    game->reset();

    DebugLogger logger(verbose);

    std::vector<HistoryFrame> rollout_history;
    // Start with the initial board in the rollout history always.
    rollout_history.emplace_back(std::nullopt, TwoPlayerNobodyWinsReward,
                                 game->getCurrentState(), 0);

    // 1. Selection - recursively choose best child node using UCB until we hit
    // a leaf node.
    // 2. Expansion - Since getBestActionIdx will return the first non-explored
    // child node, this does the expansion phase as well.
    Node &cur_node = *root_;
    logger << "Selection phase: " << std::endl;
    while (!cur_node.children.empty()) {
      logger << "current node: " << cur_node.state.render() << std::endl;
      ;
      int selected_action_idx = getBestActionIdx(game, cur_node);
      const Action chosen_action =
          game->getValidActions().at(selected_action_idx);
      int player_turn = game->turn();
      logger << "selected action: " << chosen_action.toString()
             << " for turn: " << player_turn << std::endl;
      RewardMap reward = game->simulate(chosen_action);

      rollout_history.emplace_back(chosen_action, reward,
                                   game->getCurrentState(), player_turn);
      cur_node = getNode(game);
    }

    bool need_to_update_cur_node = !game->isTerminal();

    // 3. Simulation
    if (need_to_update_cur_node) {
      // For simulation, we're keeping track of the reward for just the
      // simulated node, so accumulate the reward for just that player.
      const int player_turn = game->turn();

      logger << "Last explored node was not terminal, need to do a simulation "
                "from here to end of game starting from player turn: "
             << player_turn << std::endl;

      // Since cur_node has no children, pick one of the children to expand.
      {
        const Action action = simulation_policy->act(game);
        const RewardMap reward = game->simulate(action);
        const Node &child_node =
            getOrCreateNode(game->getCurrentState(), action, cur_node);
        rollout_history.emplace_back(action, reward, game->getCurrentState(),
                                     player_turn);
        logger << "simulation action: " << action.toString()
               << " receives reward " << reward.at(player_turn)
               << " resulting in board state: " << std::endl
               << rollout_history.back().state.render() << std::endl;
      }

      // We've created a child node, and we need to do a random simulation from
      // here to terminal state to get a reward for this node, without storing
      // any of it in the rollout history. Could also do a light playout
      // instead.
      int simulated_player = game->turn();

      while (!game->isTerminal()) {
        const Action action = simulation_policy->act(game);
        RewardMap reward = game->simulate(action);
        logger << "simulation action: " << action.toString()
               << " receives reward " << reward.at(simulated_player)
               << " resulting in board state: " << std::endl
               << game->getCurrentState().render() << std::endl;
        // Use received reward as a proxy for the reward from the earlier leaf
        // node. Modify what we've stored in the rollout history to include what
        // we think we should receive from here on out.
        rollout_history.back().reward += reward;
      }
    }

    // 4. Backpropagation.
    // Now our rollout_history buffer is a vector of frames, where each frame
    // contains:
    // - A state that was played through
    // - The action taken to get to this state, if any
    // - The reward received for taking that action
    // For example, after first rollout, we should have two frames.
    // First frame will be root state.
    // Second frame will be some child state
    //
    logger << "Backprop!" << std::endl;
    RewardMap reward_from_here_for_rollout = TwoPlayerNobodyWinsReward;
    for (auto rit = rollout_history.rbegin(); rit != rollout_history.rend();
         ++rit) {
      // All nodes should exist already
      const auto &frame = *rit;
      Node &node = nodes_.at(frame.state);
      node.num_rollouts_involved++;
      reward_from_here_for_rollout += frame.reward;
      logger << "update node with state: " << std::endl << frame.state.render() << " with reward map: " << reward_from_here_for_rollout.toString() << std::endl;
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

  // what's the rationale of choosing the child with the highest expected
  // reward?
  int getBestActionIdx(const Game<State, Action> *game, Node &current_node) {

    const std::vector<Action> &valid_actions = game->getValidActions();
    assert(!valid_actions.empty());
    assert(current_node.num_rollouts_involved != 0);

    const State &current_state = game->getCurrentState();
    const int current_node_turn = current_state.getTurn();

    double best_ucb_so_far = std::numeric_limits<double>::lowest();
    double best_idx_so_far = -1;

    for (int i = 0; i < valid_actions.size(); i++) {
      const Action &action = valid_actions.at(i);
      const std::pair<State, RewardMap> state_reward =
          game->simulateDry(current_state, action);
      const Node &child_node =
          getOrCreateNode(state_reward.first, action, current_node);
      double ucb = getUcb(
          child_node.total_reward_from_here.at(current_node_turn),
          child_node.num_rollouts_involved, current_node.num_rollouts_involved);
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
    const int current_turn = current_state.getTurn();
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
      double value = (child_node.total_reward_from_here.at(current_turn) /
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
      std::cout << game->render();
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

  const std::map<State, Node> &getNodes() { return nodes_; }

private:
  double getUcb(double child_total_reward, int child_num_rollouts,
                int parent_num_rollouts) {
    assert(parent_num_rollouts > 0);
    std::cout << "parent num rollouts: " << parent_num_rollouts << std::endl;
    std::cout << "child num rollouts: " << child_num_rollouts << std::endl;
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
