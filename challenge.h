#pragma once

#include "config_evaluator.h"

#include <map>
#include <string>
#include <vector>

using std::map;
using std::string;
using std::vector;

struct challenge {
  int target;
  int dice_limit;
  unsigned int reward;  // per dice_roller.h
  int original_idx;  // i.e. what position they were in on the screen
  bool operator<(const challenge& rhs) const { return original_idx < rhs.original_idx; }
};

struct solution_data {
  double cost;
  string output_string;  // output for BTV agent
};

// Returns cost needed
double findSolutionForOrder(const vector<challenge>& challenge_order,
  double pass_chance);

// Returns cost needed, stores solution and a bunch of closeby ones
double storeSolutionsForOrder(const vector<challenge>& challenge_order,
  double pass_chance, map<double, solution_data> *solutions_out);
