#ifndef LIB8266__FIXED__H__
#define LIB8266__FIXED__H__

#include "lib8266/base.h"

// We'll use signed 32-bit fixed-point values instead of floats, using 2^16 =
// 0x10000 = 65536 as our scale -- why the hell not. This gives a fractional
// resolution of ~2e-5, and 16 bits (+/- 32k) for values greater than 1. Seems
// like that oughta be enough for anybody, amirite?

typedef int32_t ahoy_fixed_t;

#define AHOY_FIXED_LARGEST_NEGATIVE 0x80000000
#define AHOY_FIXED_LARGEST_POSITIVE 0x7fffffff
#define AHOY_FIXED_SMALLEST_POSITIVE 1

#define AHOY_FIXED_FRACTION_BITS 16

#define AHOY_FIXED_1 (1 << AHOY_FIXED_FRACTION_BITS)

#define AHOY_TO_FIXED(x) ((ahoy_fixed_t)(AHOY_FIXED_1 * (x)))
#define AHOY_FIXED_TO_FLOAT(x) (((float)(x)) / ((float)AHOY_FIXED_1))

#define AHOY_FIXED_E (AHOY_TO_FIXED(2.71828183f))
#define AHOY_FIXED_PI (AHOY_TO_FIXED(3.14159265f))
#define AHOY_FIXED_LOG2 (AHOY_TO_FIXED(0.69314718f))

#define AHOY_FIXED_FRACTIONAL_MASK ((1 << AHOY_FIXED_FRACTION_BITS) - 1)
#define AHOY_FIXED_FRACTIONAL_PART(x) ((x) & AHOY_FIXED_FRACTIONAL_MASK)
#define AHOY_FIXED_INTEGRAL_PART(x) ((x) >> AHOY_FIXED_FRACTION_BITS)

#define AHOY_FIXED_TENTHS(x) (AHOY_FIXED_INTEGRAL_PART(10 * AHOY_FIXED_FRACTIONAL_PART(x)))
#define AHOY_FIXED_HUNDREDTHS(x) (AHOY_FIXED_INTEGRAL_PART(100 * AHOY_FIXED_FRACTIONAL_PART(x)))
#define AHOY_FIXED_THOUSANDTHS(x) (AHOY_FIXED_INTEGRAL_PART(1000 * AHOY_FIXED_FRACTIONAL_PART(x)))

#define AHOY_FIXED_CLAMP_01(x) ((x) < 0 ? 0 : (x) > AHOY_FIXED_1 ? AHOY_FIXED_1 : (x))

ahoy_fixed_t ahoy_fixed_mul(ahoy_fixed_t a, ahoy_fixed_t b);
ahoy_fixed_t ahoy_fixed_exp(ahoy_fixed_t x);
ahoy_fixed_t ahoy_fixed_log(ahoy_fixed_t x);
ahoy_fixed_t ahoy_fixed_mod(ahoy_fixed_t numer, ahoy_fixed_t denom);
ahoy_fixed_t ahoy_fixed_pow(ahoy_fixed_t base, ahoy_fixed_t exponent);

#endif
