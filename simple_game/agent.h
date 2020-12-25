#include "game.h"
#include <iostream>
#include <map>
#include <random>

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
        game->reset();
        Node* current = &root;
        while (!game->isTerminal()) {

            Action action = [&]() {
                if (game->isOurTurn()) {
                    return us.act(game);
                } else {
                    return them.act(game);
                }
            }();

            double reward = game->simulate(action);
            // current->num_rollouts_involved++;
            // how to backprop up the stream?
            // current->children
        }
    }

private:
    Node root;
    RandomAgent<State,Action> us;
    RandomAgent<State,Action> them;
};
