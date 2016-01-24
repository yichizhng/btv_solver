#include "challenge.h"
#include "challenge_loader.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <cmath>
#include <ctime>

using std::cerr;
using std::cout;
using std::endl;
using std::ostream;
using std::ofstream;
using std::string;

static double minPassChance = 0.02, maxPassChance = 0.8;

namespace {
  string dateOfLastThursday() {
    // Get the current time
    auto time = std::chrono::system_clock::now();
    char buf[100];
    auto time_as_time_t = std::chrono::system_clock::to_time_t(time);
    auto local_time = std::localtime(&time_as_time_t);
    std::strftime(buf, 100, "%w", local_time);
    int days_past_thursday = (buf[0] + 7 - '4') % 7;
    // No, that was not a typo, '4'.
    std::chrono::hours offset{-days_past_thursday * 24};
    auto last_thursday = time + offset;
    auto last_thursday_as_time_t =
      std::chrono::system_clock::to_time_t(last_thursday);
    auto last_thursday_local = std::localtime(&last_thursday_as_time_t);
    std::strftime(buf, 100, "%Y_%m_%d", last_thursday_local);
    return string(buf);
  }
}


int main(int argc, char** argv) {
  setenv("TZ", "std6", 1);
  ostream* out;
  string out_name = "BTV_strats_" + dateOfLastThursday() + ".txt";
  ofstream file_out(out_name);
  if (file_out) {
    cout << "Outputting to " << out_name << endl;
    out = &file_out;
  } else {
    cerr << "Error opening file " << out_name << " for writing" << endl
      << "Outputting to standard output" << endl;
    out = &cout;
  }
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();
  {
    map<double, solution_data> solutions;
    vector<challenge> challenge_order = loadChallenges();
    for (double probability_to_use = minPassChance;
      probability_to_use < maxPassChance;) {
      vector<challenge> best_challenge_order;
      double best_cost = 10000;
      for (int i = 0; i < 40320; ++i) {

        double this_cost = findSolutionForOrder(challenge_order, probability_to_use);
        if (this_cost < best_cost) {
          best_cost = this_cost;
          best_challenge_order = challenge_order;
        }
        // try doing the first challenge second too - worth a shot
    {
      std::swap(challenge_order[0], challenge_order[1]);
      double this_cost = findSolutionForOrder(challenge_order, probability_to_use);
      if (this_cost < best_cost) {
        best_cost = this_cost;
        best_challenge_order = challenge_order;
      }
      std::swap(challenge_order[0], challenge_order[1]);
    }

        if (!std::next_permutation(challenge_order.begin() + 1,
          challenge_order.end() - 2)) {
          //cout << i + 1 << " permutations checked" << endl;
          //break;
        }
      }
      vector<vector<challenge>> orders_to_check;
      // int orders_to_check = 0;
      for (int i = 0; i < 40320; ++i) {

        double this_cost = findSolutionForOrder(challenge_order, probability_to_use);
        if (this_cost - best_cost < 2) {
          orders_to_check.push_back(challenge_order);
        }
        // try doing the first challenge second too - worth a shot
    {
      std::swap(challenge_order[0], challenge_order[1]);
      this_cost = findSolutionForOrder(challenge_order, probability_to_use);
      if (this_cost - best_cost < 2) {
        orders_to_check.push_back(challenge_order);
      }
      std::swap(challenge_order[0], challenge_order[1]);
    }

        if (!std::next_permutation(challenge_order.begin() + 1,
          challenge_order.end() - 2)) {
        }
      }
      for (const auto& challenge_order : orders_to_check) {
        storeSolutionsForOrder(challenge_order,
          probability_to_use, &solutions);
      }
      probability_to_use = storeSolutionsForOrder(best_challenge_order,
        probability_to_use, &solutions);
    }
    vector<double> chances_to_pass;
    vector<solution_data> filtered_solutions;
    for (const auto& it : solutions) {
      // check if this solution is better than the last one in the vector

      while (filtered_solutions.size() &&
        (it.second.cost -
        filtered_solutions[filtered_solutions.size() - 1].cost)
        < (it.first - chances_to_pass[chances_to_pass.size() - 1])) {

        filtered_solutions.pop_back();
        chances_to_pass.pop_back();
      }
      if (filtered_solutions.size()) {
        double slope = (it.first - *chances_to_pass.rbegin()) /
          (it.second.cost - filtered_solutions.rbegin()->cost);
        if (slope < 1e-4) continue;
      }
      filtered_solutions.push_back(it.second);
      chances_to_pass.push_back(it.first);

    }
    for (const auto& it : filtered_solutions) {
      (*out) << it.output_string;
    }
  }
  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  cout << "Finished calculating in " << ((int)elapsed_seconds.count()) / 60 << " minutes "
    << fmod(elapsed_seconds.count(), 60) << " seconds" << endl;

  return 0;
}
