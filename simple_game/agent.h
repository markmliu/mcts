#include "game.h"
#include <iostream>

template <class State, class Action, class Outcome>
class Agent{
public:
    void play(Game<State,Action,Outcome>* game) {
        while (!game->isTerminal()) {
            std::vector<Action> valid_actions = game->getValidActions();
            // choose the first action.
            assert(!valid_actions.empty());
            game->simulate(valid_actions.at(0));
            game->render();
        }
        std::cout << "game ended!" << std::endl;
    }

};
