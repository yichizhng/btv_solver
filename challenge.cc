#include "challenge.h"

#include "config_evaluator.h"

#include <string>

#include <cmath>

using std::string;
using std::to_string;
using std::vector;

namespace {
  int costFromDiceConfig(unsigned int dice_config) {
    switch (dice_config) {
    case 0x1ff:
      return 4 * 16;
      break;
    case 0x3ff:
      return 6 * 16;
      break;
    case 0x7ff:
      return 8 * 16;
      break;
    case 0xfff:
      return 10 * 16;
      break;
    case 0x1fff:
      return 12 * 16;
      break;
    case 0x3fff:
      return 20 * 16;
      break;
    case 0x7fff:
      return 30 * 16;
      break;
    case 0xffff:
      return 100 * 16;
      break;
    default:
      return
        4 * (dice_config & 0xf) +
        6 * ((dice_config & 0xf0) >> 4) +
        8 * ((dice_config & 0xf00) >> 8) +
        10 * ((dice_config & 0xf000) >> 12) +
        12 * ((dice_config & 0xf0000) >> 16) +
        20 * ((dice_config & 0xf00000) >> 20) +
        30 * ((dice_config & 0xf000000) >> 24) +
        100 * ((dice_config & 0xf0000000) >> 28);
      break;
    }
  }

  string ToBTVAgentString(const vector<challenge>& challenge_order,
    const vector<hull_evaluation>& evals,
    int *cost, double *effective_cost) {
    *cost = 0;
    *effective_cost = 0;
    string solution_string;
    for (int i = 0; i < 11; ++i) {
      solution_string += to_string(challenge_order[i].original_idx);
      unsigned int dice_config = evals[i].dice_config;
      *cost += costFromDiceConfig(dice_config);
      *effective_cost += evals[i].effective_cost;
      if ((dice_config & 0xff) == 0xff) {
        switch (dice_config) {
        case 0x1ff:
          solution_string += ",16,0,0,0,0,0,0,0\n";
          break;
        case 0x3ff:
          solution_string += ",0,16,0,0,0,0,0,0\n";
          break;
        case 0x7ff:
          solution_string += ",0,0,16,0,0,0,0,0\n";
          break;
        case 0xfff:
          solution_string += ",0,0,0,16,0,0,0,0\n";
          break;
        case 0x1fff:
          solution_string += ",0,0,0,0,16,0,0,0\n";
          break;
        case 0x3fff:
          solution_string += ",0,0,0,0,0,16,0,0\n";
          break;
        case 0x7fff:
          solution_string += ",0,0,0,0,0,0,16,0\n";
          break;
        case 0xffff:
          solution_string += ",0,0,0,0,0,0,0,16\n";
          break;
        }
      } else {
        solution_string += ',' + to_string(dice_config & 0xf) +
          ',' + to_string((dice_config >> 4) & 0xf) +
          ',' + to_string((dice_config >> 8) & 0xf) +
          ',' + to_string((dice_config >> 12) & 0xf) +
          ',' + to_string((dice_config >> 16) & 0xf) +
          ',' + to_string((dice_config >> 20) & 0xf) +
          ',' + to_string((dice_config >> 24) & 0xf) +
          ',' + to_string((dice_config >> 28) & 0xf) +
          '\n';
      }
    }
    return solution_string;
  }

  double solutionForOrder(const vector<challenge>& challenge_order, double slope,
    vector<hull_evaluation> *out) {
    out->clear();
    unsigned int current_reward = 0;
    double pass_chance = 1;
    for (const auto &a : challenge_order) {
      const auto &oe =
        getOptimalEvaluations(current_reward & 0xff,
        a.dice_limit + (!!(current_reward & 0x100)),
        a.target);
      const auto &he = oe.at(find_index(slope, oe));
      pass_chance *= pow(10, he.payoff);
      out->push_back(he);
      current_reward |= a.reward;
    }
    return pass_chance;
  }

  // stores the next solution in next and the previous solution in prev
  void solutionAndFriendsForOrder(const vector<challenge>& challenge_order, double slope,
    vector<hull_evaluation> *out,
    vector<hull_evaluation> *next,
    vector<hull_evaluation> *prev) {
    out->clear();
    next->clear();
    prev->clear();
    unsigned int current_reward = 0;
    for (const auto &a : challenge_order) {
      const auto &oe =
        getOptimalEvaluations(current_reward & 255,
        a.dice_limit + (!!(current_reward & 0x100)),
        a.target);
      int he_index = find_index(slope, oe);
      const auto &he = oe.at(he_index);
      out->push_back(he);
      if (he_index < oe.size() - 1) {
        next->push_back(oe.at(he_index + 1));
      } else {
        hull_evaluation dummy;
        dummy.effective_cost = -1;
        next->push_back(dummy);
      }
      if (he_index > 0) {
        prev->push_back(oe.at(he_index - 1));
      } else {
        hull_evaluation dummy;
        dummy.effective_cost = -1;
        prev->push_back(dummy);
      }
      current_reward |= a.reward;
    }
  }
}

double findSolutionForOrder(const vector<challenge>& challenge_order,
  double pass_chance) {
  vector<hull_evaluation> evals;
  double min_slope = 1;
  double this_pass_chance;
  do {
    min_slope /= 2;
    this_pass_chance = solutionForOrder(challenge_order, min_slope, &evals);
    // cout << this_pass_chance << endl;
  } while (this_pass_chance < pass_chance);
  double max_slope = min_slope * 2;
  while (max_slope - min_slope > 1e-9) {
    double mid_slope = (max_slope + min_slope) / 2;
    this_pass_chance = solutionForOrder(challenge_order, mid_slope, &evals);
    if (this_pass_chance >= pass_chance) {
      min_slope = mid_slope;
    } else {
      max_slope = mid_slope;
    }
  }
  this_pass_chance = solutionForOrder(challenge_order, min_slope, &evals);
  double overall_cost = 0;
  for (const auto& he : evals) {
    overall_cost += he.effective_cost;
  }
  return overall_cost;
}

double storeSolutionsForOrder(const vector<challenge>& challenge_order,
  double pass_chance,
  map<double, solution_data> *solutions_out) {
  vector<hull_evaluation> evals;
  double min_slope = 1;
  double this_pass_chance;
  do {
    min_slope /= 2;
    this_pass_chance = solutionForOrder(challenge_order, min_slope, &evals);
    // cout << this_pass_chance << endl;
  } while (this_pass_chance <= pass_chance);
  double max_slope = min_slope * 2;
  while (max_slope - min_slope > 1e-9) {
    double mid_slope = (max_slope + min_slope) / 2;
    this_pass_chance = solutionForOrder(challenge_order, mid_slope, &evals);
    if (this_pass_chance > pass_chance) {
      min_slope = mid_slope;
    } else {
      max_slope = mid_slope;
    }
  }
  this_pass_chance = solutionForOrder(challenge_order, min_slope, &evals);

  vector<hull_evaluation> next;
  vector<hull_evaluation> prev;
  solutionAndFriendsForOrder(challenge_order, min_slope, &evals, &next, &prev);

  double min_max_slope = 100000, max_min_slope = -100000;
  int min_max_slope_idx = -1, max_min_slope_idx = -1;
  for (int i = 0; i < 11; ++i) {
    if (evals[i].min_slope > max_min_slope) {
      max_min_slope = evals[i].min_slope;
      max_min_slope_idx = i;
    }
    if (evals[i].max_slope < min_max_slope) {
      min_max_slope = evals[i].max_slope;
      min_max_slope_idx = i;
    }
  }

  // swap with next
  for (int jj = 0; jj < 11; ++jj) {
    if (next[jj].effective_cost < 0) continue;
    double subopt_pass_chance = 0;
    std::swap(evals[jj], next[jj]);
    // calculate subopt_pass_chance
    for (int i = 0; i < 11; ++i) {
      subopt_pass_chance += evals[i].payoff;
    }
    subopt_pass_chance = pow(10, subopt_pass_chance);

    int cost = 0;
    double effective_cost = 0;
    string solution_string = "BTV agent output:\n" +
      ToBTVAgentString(challenge_order, evals, &cost, &effective_cost);

    solution_string += "Cost: " + to_string(cost) + ' ';
    solution_string += "Effective cost: " + to_string(effective_cost) + ' ';
    solution_string += "Chance to pass: " + to_string(subopt_pass_chance * 100)
      + "%\n\n";

    solution_data sd;
    sd.cost = effective_cost;
    sd.output_string = solution_string;

    if (solutions_out->count(subopt_pass_chance)) {
      // check if our solution is better
      if (effective_cost < solutions_out->at(subopt_pass_chance).cost) {
        (*solutions_out)[subopt_pass_chance] = sd;
      }
    } else {
      (*solutions_out)[subopt_pass_chance] = sd;
    }
    std::swap(evals[jj], next[jj]);
  }
  // swap with prev
  for (int jj = 0; jj < 11; ++jj) {
    if (prev[jj].effective_cost < 0) continue;
    double subopt_pass_chance = 0;
    std::swap(evals[jj], prev[jj]);
    // calculate subopt_pass_chance
    for (int i = 0; i < 11; ++i) {
      subopt_pass_chance += evals[i].payoff;
    }
    subopt_pass_chance = pow(10, subopt_pass_chance);

    int cost = 0;
    double effective_cost = 0;
    string solution_string = "BTV agent output:\n" +
      ToBTVAgentString(challenge_order, evals, &cost, &effective_cost);

    solution_string += "Cost: " + to_string(cost) + ' ';
    solution_string += "Effective cost: " + to_string(effective_cost) + ' ';
    solution_string += "Chance to pass: " + to_string(subopt_pass_chance * 100)
      + "%\n\n";

    solution_data sd;
    sd.cost = effective_cost;
    sd.output_string = solution_string;  // I hope the compiler elides that <3

    if (solutions_out->count(subopt_pass_chance)) {
      // check if our solution is better
      if (effective_cost < solutions_out->at(subopt_pass_chance).cost) {
        (*solutions_out)[subopt_pass_chance] = sd;
      }
    } else {
      (*solutions_out)[subopt_pass_chance] = sd;
    }
    std::swap(evals[jj], prev[jj]);
  }
  // goto next sol, then swap back
  {
    std::swap(evals[max_min_slope_idx], next[max_min_slope_idx]);
    for (int jj = 0; jj < 11; ++jj) {
      if (prev[jj].effective_cost < 0) continue;
      double subopt_pass_chance = 0;
      std::swap(evals[jj], prev[jj]);
      for (int i = 0; i < 11; ++i) {
        subopt_pass_chance += evals[i].payoff;
      }
      subopt_pass_chance = pow(10, subopt_pass_chance);

      int cost = 0;
      double effective_cost = 0;
      string solution_string = "BTV agent output:\n" +
        ToBTVAgentString(challenge_order, evals, &cost, &effective_cost);

      solution_string += "Cost: " + to_string(cost) + ' ';
      solution_string += "Effective cost: " + to_string(effective_cost) + ' ';
      solution_string += "Chance to pass: " + to_string(subopt_pass_chance * 100)
        + "%\n\n";

      solution_data sd;
      sd.cost = effective_cost;
      sd.output_string = solution_string;  // I hope the compiler elides that <3

      if (solutions_out->count(subopt_pass_chance)) {
        // check if our solution is better
        if (effective_cost < solutions_out->at(subopt_pass_chance).cost) {
          (*solutions_out)[subopt_pass_chance] = sd;
        }
      } else {
        (*solutions_out)[subopt_pass_chance] = sd;
      }
      std::swap(evals[jj], prev[jj]);
    }
    std::swap(evals[max_min_slope_idx], next[max_min_slope_idx]);
  }
  {
    std::swap(evals[min_max_slope_idx], prev[min_max_slope_idx]);
    for (int jj = 0; jj < 11; ++jj) {
      if (next[jj].effective_cost < 0) continue;
      double subopt_pass_chance = 0;
      std::swap(evals[jj], next[jj]);
      // calculate subopt_pass_chance
      for (int i = 0; i < 11; ++i) {
        subopt_pass_chance += evals[i].payoff;
      }
      subopt_pass_chance = pow(10, subopt_pass_chance);

      int cost = 0;
      double effective_cost = 0;
      string solution_string = "BTV agent output:\n" +
        ToBTVAgentString(challenge_order, evals, &cost, &effective_cost);

      solution_string += "Cost: " + to_string(cost) + ' ';
      solution_string += "Effective cost: " + to_string(effective_cost) + ' ';
      solution_string += "Chance to pass: " + to_string(subopt_pass_chance * 100)
        + "%\n\n";

      solution_data sd;
      sd.cost = effective_cost;
      sd.output_string = solution_string;  // I hope the compiler elides that <3

      if (solutions_out->count(subopt_pass_chance)) {
        // check if our solution is better
        if (effective_cost < solutions_out->at(subopt_pass_chance).cost) {
          (*solutions_out)[subopt_pass_chance] = sd;
        }
      } else {
        (*solutions_out)[subopt_pass_chance] = sd;
      }
      std::swap(evals[jj], next[jj]);
    }
    std::swap(evals[min_max_slope_idx], prev[min_max_slope_idx]);
  }

  return this_pass_chance;
}
