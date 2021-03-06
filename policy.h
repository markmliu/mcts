#ifndef MCTS_POLICY
#define MCTS_POLICY

#include "game.h"

#include <iostream>
#include <random>

// TODO: it might not really make that much sense to have Policy be a separate
// interface from the class that learns the value function, since a normal
// policy will use the value function to some degree.

template <class State, class Action> class Policy {
public:
  virtual Action act(const Game<State, Action> *game) = 0;
  virtual ~Policy() = default;
};

// RandomValidPolicy: Select a random "valid" policy
template <class State, class Action>
class RandomValidPolicy : public Policy<State, Action> {
public:
  RandomValidPolicy() {
    gen = std::mt19937(rd()); // seed the generator
  }
  Action act(const Game<State, Action> *game) override {
    assert(!game->isTerminal());
    std::vector<Action> valid_actions = game->getValidActions();
    assert(!valid_actions.empty());

    // choose a random action in range
    std::uniform_int_distribution<> distr(0, valid_actions.size() - 1);
    int random_idx = distr(gen);
    return valid_actions.at(random_idx);
  }

private:
  std::random_device rd; // obtain a random number from hardware
  std::mt19937 gen;
};

// UserInputPolicy: Tells user the list of valid inputs, and prompts user for a
// choice.
template <class State, class Action>
class UserInputPolicy : public Policy<State, Action> {
public:
  Action act(const Game<State, Action> *game) override {
    assert(!game->isTerminal());
    std::vector<Action> valid_actions = game->getValidActions();
    assert(!valid_actions.empty());

    // Display valid actions to user
    for (int i = 0; i < valid_actions.size(); ++i) {
      std::cout << i << ": " << valid_actions[i].toString() << std::endl;
    }
    int user_selected_index = -1;
    std::cin >> user_selected_index;
    while (user_selected_index < 0 ||
           user_selected_index >= valid_actions.size()) {
      std::cout << "User input out of range, try again. " << std::endl;
      std::cin.ignore();
      std::cin.clear();
      std::cin >> user_selected_index;
    }
    return valid_actions[user_selected_index];
  }
};

#endif // MCTS_POLICY
