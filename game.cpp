#include "game.h"

double &RewardMap::at(int turn) { return data.at(turn); }

const double &RewardMap::at(int turn) const { return data.at(turn); }

RewardMap &RewardMap::operator+=(const RewardMap &rhs) {
  for (const auto &el : rhs.data) {
    auto it = data.find(el.first);
    assert(it != data.end());
    it->second += el.second;
  }
  return *this;
}

RewardMap operator+(RewardMap lhs, const RewardMap &rhs) {
  // Key-wise add every element of rhs to lhs.
  lhs += rhs;
  return lhs;
}
