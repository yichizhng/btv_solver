#include "dice_roller.h"

#include <array>
#include <unordered_map>

namespace {
  // Map from dice configs to CDFs
  std::unordered_map<unsigned int, cdf_t> memoization;
  // Indicates the reward config the current memoization is for.
  unsigned int current_reward_config = 257;
  void reset_memoization() {
    memoization.clear();
    memoization.reserve(1000000);
  }

  // Various utility functions regarding dice rolling.
  // They're kind of obfuscated because I was bored.

  // add one die to a CDF
  inline cdf_t add_die(int range, const cdf_t& v) {
    auto ret = cdf_t{{1}};
    double running_total = range;
    for (int i = 1; i < 71; ++i) {
      ret[i] = running_total / range;
      running_total -= (i >= range) ?
        v[i - range] : 1;
      running_total += v[i];
    }
    return ret;
  }

  // add one die to a CDF with 1 strength
  inline cdf_t add_s_die(int range, const cdf_t& v) {
    auto ret = cdf_t{{1, 1}};
    double running_total = range;
    for (int i = 2; i < 71; ++i) {
      ret[i] = running_total / range;
      running_total -= (i >= range) ? 2 * v[i - range] : 2;
      running_total += (i >= range - 1) ? v[i - (range - 1)] : 1;
      running_total += v[i - 1];
    }
    return ret;
  }
  inline void add_atm(unsigned int reward_config, cdf_t& v) {
    v = add_die(reward_config & 0x1 ? 5 : 4, v);
  }
  inline void add_dic(unsigned int reward_config, cdf_t& v) {
    v = reward_config & 0x4 ?
      add_s_die(reward_config & 0x2 ? 7 : 6, v) :
      add_die(reward_config & 0x2 ? 7 : 6, v);
  }
  inline void add_pre(unsigned int reward_config, cdf_t& v) {
    v = reward_config & 0x10 ?
      add_s_die(reward_config & 0x8 ? 9 : 8, v) :
      add_die(reward_config & 0x8 ? 9 : 8, v);
  }
  inline void add_cal(unsigned int reward_config, cdf_t& v) {
    v = reward_config & 0x20 ?
      add_s_die(10, v) :
      add_die(10, v);
  }
  inline void add_foc(unsigned int reward_config, cdf_t& v) {
    v = add_die(12, v);
  }
  inline void add_sty(unsigned int reward_config, cdf_t& v) {
    if (reward_config & 0x40) {
      cdf_t v2 = v, v3{{1}};
      v2 = add_die(18, v2);
      for (int i = 18; i < 71; ++i) {
        v3[i] = v[i - 18];
      }
      v3 = add_die(2, v3);
      v3 = add_die(20, v3);
      for (int i = 0; i < 71; ++i) {
        v[i] = 0.9 * v2[i] + 0.1 * v3[i];
      }
    }
    else {
      v = add_die(20, v);
    }
  }
  inline void add_rhy(unsigned int reward_config, cdf_t& v) {
    v = add_die(30, v);
  }
  inline void add_tim(unsigned int reward_config, cdf_t& v) {
    v = add_die(100, v);
  }
}

const cdf_t& cdf(unsigned int reward_config, unsigned int dice_config) {
  if (current_reward_config != reward_config) {
    reset_memoization();
    current_reward_config = reward_config;
  }
  if (memoization.count(dice_config)) {
    return memoization[dice_config];
  }

  // Base case: if the dice config is 0, then the answer is obviously 0
  if (dice_config == 0) {
    return memoization[0] = cdf_t {{1}};
  }

  // Special cases
  if (dice_config == 0x000001ff) {
    cdf_t rec_cdf = cdf(reward_config, 0x0000000f);
    add_atm(reward_config, rec_cdf);
    return memoization[dice_config] = rec_cdf;
  } else if (dice_config == 0x000003ff) {
    cdf_t rec_cdf = cdf(reward_config, 0x000000f0);
    add_dic(reward_config, rec_cdf);
    return memoization[dice_config] = rec_cdf;
  } else if (dice_config == 0x000007ff) {
    cdf_t rec_cdf = cdf(reward_config, 0x00000f00);
    add_pre(reward_config, rec_cdf);
    return memoization[dice_config] = rec_cdf;
  } else if (dice_config == 0x00000fff) {
    cdf_t rec_cdf = cdf(reward_config, 0x0000f000);
    add_cal(reward_config, rec_cdf);
    return memoization[dice_config] = rec_cdf;
  } else if (dice_config == 0x00001fff) {
    cdf_t rec_cdf = cdf(reward_config, 0x000f0000);
    add_foc(reward_config, rec_cdf);
    return memoization[dice_config] = rec_cdf;
  } else if (dice_config == 0x00003fff) {
    cdf_t rec_cdf = cdf(reward_config, 0x00f00000);
    add_sty(reward_config, rec_cdf);
    return memoization[dice_config] = rec_cdf;
  } else if (dice_config == 0x00007fff) {
    cdf_t rec_cdf = cdf(reward_config, 0x0f000000);
    add_rhy(reward_config, rec_cdf);
    return memoization[dice_config] = rec_cdf;
  } else if (dice_config == 0x0000ffff) {
    cdf_t rec_cdf = cdf(reward_config, 0xf0000000);
    add_tim(reward_config, rec_cdf);
    return memoization[dice_config] = rec_cdf;
  } else if (dice_config & 0x0000000f) {
    cdf_t rec_cdf = cdf(reward_config, dice_config - 0x00000001);
    add_atm(reward_config, rec_cdf);
    return memoization[dice_config] = rec_cdf;
  } else if (dice_config & 0x000000f0) {
    cdf_t rec_cdf = cdf(reward_config, dice_config - 0x00000010);
    add_dic(reward_config, rec_cdf);
    return memoization[dice_config] = rec_cdf;
  } else if (dice_config & 0x00000f00) {
    cdf_t rec_cdf = cdf(reward_config, dice_config - 0x00000100);
    add_pre(reward_config, rec_cdf);
    return memoization[dice_config] = rec_cdf;
  } else if (dice_config & 0x0000f000) {
    cdf_t rec_cdf = cdf(reward_config, dice_config - 0x00001000);
    add_cal(reward_config, rec_cdf);
    return memoization[dice_config] = rec_cdf;
  } else if (dice_config & 0x000f0000) {
    cdf_t rec_cdf = cdf(reward_config, dice_config - 0x00010000);
    add_foc(reward_config, rec_cdf);
    return memoization[dice_config] = rec_cdf;
  } else if (dice_config & 0x00f00000) {
    cdf_t rec_cdf = cdf(reward_config, dice_config - 0x00100000);
    add_sty(reward_config, rec_cdf);
    return memoization[dice_config] = rec_cdf;
  } else if (dice_config & 0x0f000000) {
    cdf_t rec_cdf = cdf(reward_config, dice_config - 0x01000000);
    add_rhy(reward_config, rec_cdf);
    return memoization[dice_config] = rec_cdf;
  } else if (dice_config & 0xf0000000) {
    cdf_t rec_cdf = cdf(reward_config, dice_config - 0x10000000);
    add_tim(reward_config, rec_cdf);
    return memoization[dice_config] = rec_cdf;
  }
  throw "Fatal logic error, go tell yichizhng that cdf is broken";
}