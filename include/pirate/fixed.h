#ifndef PIRATE__FIXED__H__
#define PIRATE__FIXED__H__

#include "pirate/base.h"

// We'll use signed 32-bit fixed-point values instead of floats, using 2^16 =
// 0x10000 = 65536 as our scale -- why the hell not. This gives a fractional
// resolution of ~2e-5, and 16 bits (+/- 32k) for values greater than 1. Seems
// like that oughta be enough for anybody, amirite?

typedef int32_t ahoy_fixed_t;

#define AHOY_FIXED_LARGEST_NEGATIVE 0x80000000
#define AHOY_FIXED_LARGEST_POSITIVE 0x7fffffff
#define AHOY_FIXED_SMALLEST_POSITIVE 1

#define AHOY_FIXED_1 65536
#define AHOY_FIXED_E 178145
#define AHOY_FIXED_PI 205887
#define AHOY_FIXED_LOG2 45426

#define AHOY_TO_FIXED(x) ((ahoy_fixed_t)(AHOY_FIXED_1 * (x)))
#define AHOY_FIXED_TO_FLOAT(x) (((float)(x)) / ((float)AHOY_FIXED_1))
#define AHOY_FIXED_TO_INT(x)  (((x) & 0xffff0000) >> 16)

#define AHOY_FIXED_ABS_FRACTION(x) ((x) & 0x0000ffff)

#define AHOY_FIXED_CLAMP_01(x) ((x) < 0 ? 0 : (x) > AHOY_FIXED_1 ? AHOY_FIXED_1 : (x))

ahoy_fixed_t ahoy_fixed_exp(ahoy_fixed_t x);
ahoy_fixed_t ahoy_fixed_log(ahoy_fixed_t x);
ahoy_fixed_t ahoy_fixed_mod(ahoy_fixed_t numer, ahoy_fixed_t denom);
ahoy_fixed_t ahoy_fixed_pow(ahoy_fixed_t base, ahoy_fixed_t exponent);

#endif
