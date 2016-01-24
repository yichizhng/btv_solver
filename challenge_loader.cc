#include "challenge_loader.h"

#include <fstream>
#include <iostream>

using std::cerr;
using std::endl;
using std::ifstream;
using std::string;
using std::vector;

vector<challenge> loadChallenges() {
  vector<challenge> ret;
  ifstream challenge_file("challenges.txt", std::ios::in);
  string token;
  if (!challenge_file) {
    cerr << "challenges.txt cannot be opened (does it exist?)" << endl;
    exit(-1);
  }
  do {
    challenge c;
    challenge_file >> token;
    if (token == "Target:") {
      challenge_file >> c.target;
    } else if (token == "Abilities:") {
      challenge_file >> c.dice_limit;
    } else if (token == "Reward:") {
      challenge_file >> token;
      if (token == "+1") {
        challenge_file >> token;
        if (token == "Atmosphere") {
          c.reward = 0x1;
        } else if (token == "Diction") {
          challenge_file >> token;
          if (token == "Range") {
            c.reward = 0x2;
          } else {
            c.reward = 0x4;
          }
        } else if (token == "Precision") {
          challenge_file >> token;
          if (token == "Range") {
            c.reward = 0x8;
          } else {
            c.reward = 0x10;
          }
        } else if (token == "Calmness") {
          c.reward = 0x20;
        } else if (token == "Style") {
          c.reward = 0x40;
        } else if (token == "Ability") {
          c.reward = 0x100;
        }
      } else if (token == "+11%") {
        c.reward = 0x80;
      } else {
        // the others ("+10" and "One") are finale powers
        c.reward = 0;
      }
      c.original_idx = ret.size();
      ret.push_back(c);
    }
  } while (challenge_file);
  if (ret.size() != 11) {
    // Note that I do not bother validating all of your input
    // if you manage to input an impossible die count or something
    // that sucks for you
    cerr << "challenges.txt is wrong" << endl;
    system("pause");
    exit(-1);
  }
  // Move the two 0-rewards to the back and the 0x100 reward to the front
  for (int i = 0; i < 9; ++i) {
    while (ret[i].reward == 0) {
      challenge c = ret[i];
      ret.erase(ret.begin() + i);
      ret.push_back(c);
    }
  }
  for (int i = 0; i < 11; ++i) {
    if (ret[i].reward == 0x100) {
      challenge c = ret[i];
      ret.erase(ret.begin() + i);
      ret.insert(ret.begin(), c);
    }
  }
  return ret;
}
