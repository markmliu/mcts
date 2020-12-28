#include "game.h"

#include <random>

template <class State, class Action>
class Policy {
public:
    virtual Action act(const Game<State, Action> * game) = 0;
    virtual ~Policy() = default;
};

template <class State, class Action> class RandomValidPolicy : public Policy<State, Action> {
public:
    RandomValidPolicy() {
        gen = std::mt19937(rd()); // seed the generator
    }
    Action act(const Game<State,Action>* game) override {
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
