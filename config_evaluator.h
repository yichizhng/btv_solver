#pragma once

#include "dice_roller.h"
#include <vector>

struct hull_evaluation {
  unsigned int dice_config;
  double effective_cost;
  double payoff;  // log base 10 of the chance of passing the check
  double max_slope;  // internal details
  double min_slope;  // internal details
};

typedef std::vector<hull_evaluation> convex_hull;

// Computes the convex hull of the possible abilities to use on the challenge.
// The most expensive function in the program :(
// It could be serialized, but that doesn't make it much faster.
const convex_hull& getOptimalEvaluations(unsigned int reward_config,
  int max_dice, int target);

// Binary searches the convex hull for the appropriate solution for
// an overall slope.
int find_index(double slope, const convex_hull& opt_evals);
