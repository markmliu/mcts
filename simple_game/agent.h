#include "game.h"
#include <iostream>
#include <map>
#include <random>
#include <queue>

template <class State, class Action>
class RandomAgent{
public:
    RandomAgent() {
        gen = std::mt19937(rd()); // seed the generator
    }
    Action act(Game<State,Action>* game) {
        assert(!game->isTerminal());
        std::vector<Action> valid_actions = game->getValidActions();
        assert(!valid_actions.empty());

        // choose a random action in range
        std::uniform_int_distribution<> distr(0, valid_actions.size()-1);
        int random_idx = distr(gen);
        return valid_actions.at(random_idx);
    }
private:
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen;

};

template <class State, class Action>
class MCTS{
public:
    struct Node {
        Node() : num_rollouts_involved(0),
                 total_reward_from_here(0) {}
        int num_rollouts_involved;
        double total_reward_from_here;
        std::map<Action, Node> children;
    };

    void rollout(Game<State,Action>* game) {
        // Simulate a rollout with uniform random policy and likewise for opponent.
        // As we simulate, we want to update the game tree. Each node of the tree
        // stores:
        // - how many rollouts have passed through this node
        // - the total reward of games that have passed through this node.

        // let's assume for now that the reward will only come at a terminal state.

        // 1. Do a playthrough, keeping track of the actions that were played.
        game->reset();

        std::vector<std::pair<Action, double>> rollout_history;
        while (!game->isTerminal()) {
            Action action = [&]() {
                if (game->isOurTurn()) {
                    return us.act(game);
                } else {
                    return them.act(game);
                }
            }();

            double reward = game->simulate(action); // unused.
            game->render();
            rollout_history.push_back(std::make_pair(action, reward));
        }

        double total_rollout_reward = std::accumulate(rollout_history.begin(),
                                                      rollout_history.end(),
                                                      0.0,
                                                      [&](double a,
                                                          const std::pair<Action, double>& el) {
                                                          return a + el.second;
                                                      });
        std::cout << "calculating total reward: " << total_rollout_reward << std::endl;
        auto updateNode = [&](Node* current) {
            current->num_rollouts_involved++;
            current->total_reward_from_here += total_rollout_reward;
        };

        Node* current = &root;
        updateNode(current);

        // Go through the rollout history and update node values for each one.
        for (const auto& action_reward : rollout_history) {
            const Action& action = action_reward.first;
            // This should create a child if one didn't already exist.

            Node& next = current->children[action];
            updateNode(&next);
            current = &next;
        }
    }

    void renderTree() {
        // how to display the tree? maybe with a BFS
        std::queue<std::pair<int, const Node*>> queue;
        queue.push(std::make_pair(0, &root));
        while (!queue.empty()) {
            std::pair<int, const Node*> depth_top = queue.front();
            int depth = depth_top.first;
            const Node* top = depth_top.second;
            queue.pop();
            std::cout << "depth: " << depth << std::endl;
            std::cout << "num rollouts: " << top->num_rollouts_involved << std::endl;
            std::cout << "reward: " << top->total_reward_from_here << std::endl;
            for (const auto& action_child : top->children) {
                const Node* child = &action_child.second;
                queue.push(std::make_pair(depth+1, child));
            }
        }
    }

private:
    Node root;
    RandomAgent<State,Action> us;
    RandomAgent<State,Action> them;
};
