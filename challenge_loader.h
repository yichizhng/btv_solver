#pragma once

#include "challenge.h"
#include <vector>

// Loads challenges from the file "challenges.txt", which is expected to exist
// in this directory. If it does not, we will have problems <3
std::vector<challenge> loadChallenges();