#include <string>

class EpsilonScheduler {
public:
  virtual double getEpsilon() = 0;
  virtual std::string name() = 0;
};

class FixedEpsilonScheduler : public EpsilonScheduler {
public:
  FixedEpsilonScheduler(double eps) : eps_(eps) {}
  double getEpsilon() { return eps_; }
  std::string name() { return "fixed epsilon " + std::to_string(eps_); }

private:
  double eps_;
};

// Start at high learning rate of 1.0 and go down to 0.05 in 0.05 increments
class BasicEpsilonScheduler : public EpsilonScheduler {
  BasicEpsilonScheduler() : eps_(1.0) {}
  double getEpsilon() {
    double eps_copy = eps_;
    if (eps_ >= 0.05) {
      eps_ -= 0.05;
    }
    return eps_copy;
  }
  std::string name() { return "basic epsilon scheduler "; }

private:
  double eps_;
};
