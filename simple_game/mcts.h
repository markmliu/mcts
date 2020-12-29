#include "game.h"
#include "policy.h"

#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <queue>

template <class State, class Action> class MCTS {
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

  MCTS() {
    nodes_.insert(std::make_pair(State(), Node(State())));
    root_ = &(nodes_.at(State()));
  }

  void train(Game<State, Action> *game) {
    auto self_policy = std::make_unique<RandomValidPolicy<State, Action>>();
    auto opponent_policy = std::make_unique<RandomValidPolicy<State, Action>>();
    rollout(game, self_policy.get(), opponent_policy.get());
  }

  void rollout(Game<State, Action> *game, Policy<State, Action> *self_policy,
               Policy<State, Action> *opponent_policy) {
    // Simulate a rollout with uniform random policy and likewise for opponent.
    // As we simulate, we want to update the game tree. Each node of the tree
    // stores:
    // - how many rollouts have passed through this node
    // - the total reward of games that have passed through this node.

    // let's assume for now that the reward will only come at a terminal state.

    // 1. Do a playthrough, keeping track of the actions that were played.
    game->reset();

    struct HistoryBuffer {
      HistoryBuffer(Action action_, double reward_, const State &state_)
          : action(action_), reward(reward_), state(state_) {}
      // Action that created this node
      Action action;
      // Reward after taking action
      double reward;
      // State after taking action
      State state;
    };

    std::vector<HistoryBuffer> rollout_history;

    while (!game->isTerminal()) {
      Action action = [&]() {
        if (game->isOurTurn()) {
          return self_policy->act(game);
        } else {
          return opponent_policy->act(game);
        }
      }();

      double reward = game->simulate(action); // unused.
      game->render();
      rollout_history.emplace_back(action, reward, game->getCurrentState());
    }

    double total_rollout_reward = std::accumulate(
        rollout_history.begin(), rollout_history.end(), 0.0,
        [&](double a, const HistoryBuffer &el) { return a + el.reward; });
    std::cout << "calculating total reward: " << total_rollout_reward
              << std::endl;
    auto updateNode = [&](Node *current) {
      current->num_rollouts_involved++;
      current->total_reward_from_here += total_rollout_reward;
    };

    Node *current = root_;
    updateNode(current);

    // Go through the rollout history and update node values for each one.
    for (const auto &buffer : rollout_history) {
      // Create the node if it doesn't exist.
      if (nodes_.find(buffer.state) == nodes_.end()) {
        nodes_.insert(std::make_pair(buffer.state, Node(buffer.state)));
      }

      // Update the parent node to point to the newly created node, if it does
      // not already.
      const Action &action = buffer.action;
      if (current->children.find(action) == current->children.end()) {
        current->children.insert(
            std::make_pair(action, &nodes_.at(buffer.state)));
      }

      Node &next = nodes_.at(buffer.state);
      updateNode(&next);
      current = &next;
    }
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

private:
  std::map<State, Node> nodes_;
  Node *root_;
};
