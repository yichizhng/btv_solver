// reward_config is a bitmap indicating the (relevant) bonuses which have been
// gotten from the previous challenges.

#pragma once

#include <array>

typedef std::array<double, 71> cdf_t;

// 0x1: +1 atmosphere range
// 0x2: +1 diction range
// 0x4: +1 diction strength
// 0x8: +1 precision range
// 0x10: +1 precision strength
// 0x20: +1 calmness strength
// 0x40: +1 style attempt on style roll of 19 or 20
// 0x80: 11% to get back lowest ability used
// 0x100: +1 ability (ignored)

// dice_config encodes the abilities to use on the challenge. (Bitmask: ability)
// 0x0000000f: Atmosphere
// 0x000000f0: Diction
// 0x00000f00: Precision
// 0x0000f000: Calmness
// 0x000f0000: Focus
// 0x00f00000: Style
// 0x0f000000: Rhythm
// 0xf0000000: Timing
// The observant will notice that there is a case this doesn't handle: 16 of
// a single ability. This is handled by some dummy values which obviously use
// too many abilties.
// 0x000001ff: Atmosphere
// 0x000003ff: Diction
// 0x000007ff: Precision
// 0x00000fff: Calmness
// 0x00001fff: Focus
// 0x00003fff: Style
// 0x00007fff: Rhythm
// 0x0000ffff: Timing

// Returns an array indicating the probability that the the roll
// is at least that number. i.e. cdf(r, d)[i] is the chance that the roll
// is at least i
const cdf_t& cdf(unsigned int reward_config, unsigned int dice_config);
