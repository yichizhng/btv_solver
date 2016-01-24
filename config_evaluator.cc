#include "config_evaluator.h"

#include <algorithm>
#include <unordered_map>

#include <cmath>

using std::vector;

namespace {
  struct evaluation {
    unsigned int dice_config;
    double effective_cost;
    double payoff;
  };

  std::unordered_map<int, convex_hull> eval_memoization;

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

  bool evalCompare(evaluation a, evaluation b) {
    return a.effective_cost < b.effective_cost;
  }

  const double MINIMUM_PROBABILITY_SUPPORTED = 0.02;

  vector<evaluation> getEvaluations(unsigned int reward_config, int max_dice, int target) {
    vector<evaluation> ret;
    for (unsigned int atm = 0; atm <= max_dice; ++atm) {
      for (unsigned int dic = 0; dic <= max_dice - atm; ++dic) {
        for (unsigned int pre = 0; pre <= max_dice - (atm + dic); ++pre) {
          for (unsigned int cal = 0; cal <= max_dice - (atm + dic + pre); ++cal) {
            for (unsigned int foc = 0; foc <= max_dice - (atm + dic + pre + cal); ++foc) {
              for (unsigned int sty = 0; sty <= max_dice - (atm + dic + pre + cal + foc); ++sty) {
                for (unsigned int rhy = 0; rhy <= max_dice - (atm + dic + pre + cal + foc + sty); ++rhy) {
                  for (unsigned int tim = 0; tim <= max_dice - (atm + dic + pre + cal + foc + sty + rhy); ++tim) {
                    double pass_probability;
                    evaluation e;
                    if (atm == 16) {
                      e.dice_config = 0x000001ff;
                    } else if (dic == 16) {
                      e.dice_config = 0x000003ff;
                    } else if (pre == 16) {
                      e.dice_config = 0x000007ff;
                    } else if (cal == 16) {
                      e.dice_config = 0x00000fff;
                    } else if (foc == 16) {
                      e.dice_config = 0x00001fff;
                    } else if (sty == 16) {
                      e.dice_config = 0x00003fff;
                    } else if (rhy == 16) {
                      e.dice_config = 0x00007fff;
                    } else if (tim == 16) {  // lol
                      e.dice_config = 0x0000ffff;
                    } else {
                      e.dice_config = atm ^ (dic << 4) ^ (pre << 8)
                        ^ (cal << 12) ^ (foc << 16) ^ (sty << 20) ^
                        (rhy << 24) ^ (tim << 28);
                    }
                    pass_probability = cdf(reward_config, e.dice_config)[target];
                    e.effective_cost = costFromDiceConfig(e.dice_config);
                    if (pass_probability >= 1) {
                      // (insert BS explanation about why - look, it works, okay?
                      // I know better than you. Kinda. Maybe.)
                      break;
                    }
                    if (pass_probability < MINIMUM_PROBABILITY_SUPPORTED) {
                      continue;
                    }
                    e.payoff = log10(pass_probability);
                    if (reward_config & 0x80) {
                      // Find the lowest die used
                      // Some math as to what happens: we assume that each
                      // atmosphere die (4 cost) is worth an average of 13
                      // on the finale; then the other effective paybacks are
                      // diction: (10 + 34/7) / (13/4) = 32/7
                      // precision: (10 + 53/9) / (13/4) = 44/9
                      // calmness: (10 + 32/5) / (13/4) = 328/65
                      // focus: (10 + 13/2) / (13/4) = 66/13
                      // style: (10 + 231/20) / (13/4) = 431/65
                      // rhythm: (10 + 31/2) / (13/4) = 102/13
                      // timing: (10 + 101/2) / (13/4) = 242/13
                      // Furthermore you gain a bonus of (effective payback - 4)
                      // due to the finale bonus
                      // (It should actually be a bit less, but it's hard to correct
                      // for that locally because it depends when you get this reward)
                      if (e.dice_config == 0x000001ff) {
                        e.effective_cost -= 0.11 * 4;
                      } else if (e.dice_config == 0x000003ff) {
                        e.effective_cost -= 0.22 * (32. / 7.) - 0.44;
                      } else if (e.dice_config == 0x000007ff) {
                        e.effective_cost -= 0.22 * (44. / 9.) - 0.44;
                      } else if (e.dice_config == 0x00000fff) {
                        e.effective_cost -= 0.22 * (328. / 65.) - 0.44;
                      } else if (e.dice_config == 0x00001fff) {
                        e.effective_cost -= 0.22 * (66. / 13.) - 0.44;
                      } else if (e.dice_config == 0x00003fff) {
                        e.effective_cost -= 0.22 * (431. / 65.) - 0.44;
                      } else if (e.dice_config == 0x00007fff) {
                        e.effective_cost -= 0.22 * (102. / 13.) - 0.44;
                      } else if (e.dice_config == 0x0000ffff) {
                        e.effective_cost -= 0.22 * (242. / 13.) - 0.44;
                      } else if (e.dice_config & 0x0000000f) {
                        e.effective_cost -= 0.11 * 4;
                      } else if (e.dice_config & 0x000000f0) {
                        e.effective_cost -= 0.22 * (32. / 7.) - 0.44;
                      } else if (e.dice_config & 0x00000f00) {
                        e.effective_cost -= 0.22 * (44. / 9.) - 0.44;
                      } else if (e.dice_config & 0x0000f000) {
                        e.effective_cost -= 0.22 * (328. / 65.) - 0.44;
                      } else if (e.dice_config & 0x000f0000) {
                        e.effective_cost -= 0.22 * (66. / 13.) - 0.44;
                      } else if (e.dice_config & 0x00f00000) {
                        e.effective_cost -= 0.22 * (431. / 65.) - 0.44;
                      } else if (e.dice_config & 0x0f000000) {
                        e.effective_cost -= 0.22 * (102. / 13.) - 0.44;
                      } else {
                        e.effective_cost -= 0.22 * (242. / 13.) - 0.44;
                      }
                    }
                    ret.push_back(e);
                  }
                }
              }
            }
          }
        }
      }
    }
    return ret;
  }
}

const convex_hull& getOptimalEvaluations(unsigned int reward_config,
  int max_dice, int target) {
  int memo_idx = reward_config ^ (max_dice << 8) ^ (target << 16);
  if (eval_memoization.count(memo_idx)) {
    return eval_memoization[memo_idx];
  }
  // It should be noted that this function could be serialized (by saving
  // eval_memoization), but this turns out not to save much runtime.
  vector<evaluation> evals = getEvaluations(reward_config, max_dice, target);
  std::sort(evals.begin(), evals.end(), evalCompare);
  vector<hull_evaluation> ret;
  // Build ret in two passes; the first to construct, second to find the slopes
  for (const auto& eval : evals) {
    hull_evaluation he;
    he.dice_config = eval.dice_config;
    he.effective_cost = eval.effective_cost;
    he.payoff = eval.payoff;

    if (ret.size() && ret[ret.size() - 1].effective_cost == he.effective_cost) {
      if (he.payoff > ret[ret.size() - 1].payoff) {
        ret[ret.size() - 1] = he;
      } else {
        continue;
      }
    } else {
      ret.push_back(he);
    }

  }
  if (ret.size() > 0) {
    ret[0].max_slope = 1000000;
    ret[ret.size() - 1].min_slope = 0;
    for (int i = 1; i < ret.size(); ++i) {
      double left_slope = (ret[i].payoff - ret[i - 1].payoff) /
        (ret[i].effective_cost - ret[i - 1].effective_cost);
      ret[i - 1].min_slope = left_slope;
      ret[i].max_slope = left_slope;
    }
  }
  // TODO: wut
  // This should really be done in the first pass but I can't be bothered to fix it
  for (int i = 1; i < ret.size() - 1; ++i) {
    while (i >= 1 && (i < ret.size() - 1) &&
      ret[i - 1].min_slope < ret[i].min_slope) {
      ret.erase(ret.begin() + i);
      double left_slope = (ret[i].payoff - ret[i - 1].payoff) /
        (ret[i].effective_cost - ret[i - 1].effective_cost);
      ret[i - 1].min_slope = left_slope;
      ret[i].max_slope = left_slope;
      i--;
    }
  }
  return eval_memoization[memo_idx] = ret;
}

int find_index(double slope, const convex_hull &opt_evals) {
  if (slope > opt_evals[1].max_slope) { return 0; }
  if (slope < opt_evals[opt_evals.size() - 2].min_slope) { return opt_evals.size() - 1; }
  int start = 0, end = opt_evals.size();
  while (end - start > 1) {
    int mid = (start + end) / 2;
    if (opt_evals[mid].max_slope > slope) {
      if (opt_evals[mid].min_slope <= slope) {
        return mid;
      }
      start = mid + 1;
    } else {
      end = mid;
    }
  }
  return start;
}
