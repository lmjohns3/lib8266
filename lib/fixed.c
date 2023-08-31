#include "lib8266/fixed.h"

/* Compute the product of two fixed-point values. */
ahoy_fixed_t ahoy_fixed_mul(ahoy_fixed_t a, ahoy_fixed_t b) {
  return (a * b) >> AHOY_FIXED_FRACTION_BITS;
}

/* Compute the fixed-point base-e exponential function, exp(x) = e^x.
 *
 * We first scale x, dividing successively by 2, until x is in the range [-1, 1].
 * Then we can use the Taylor series to approximate e^x, and then we square the
 * result to scale back up, corresponding to the divisions by 2.
 */
ahoy_fixed_t ahoy_fixed_exp(ahoy_fixed_t x) {
  /* Check for underflow and overflow. */
  if (x < AHOY_TO_FIXED(-11.09f)) return AHOY_FIXED_SMALLEST_POSITIVE;
  if (x > AHOY_TO_FIXED(10.39f)) return AHOY_FIXED_LARGEST_POSITIVE;

  /* e^x = (e^(x/2))^2 */
  uint8_t halvings = 0;

  const ahoy_fixed_t _1_4 = AHOY_FIXED_1 >> 2;
  while (x < -_1_4 || x > _1_4) {
    ++halvings;
    x >>= 1;
  }

  /* At this point x \in [-1/4, 1/4]. Taylor series at x = 0 is
   * e^x = 1 + x + x^2/2 + x^3/6 + x^4/24 + x^5/120 ...
   *     = 1 + x (1 + x/2 + x^2/6 + x^3/24 + x^4/120 ...)
   *     = 1 + x (1 + x/2 (1 + x/3 + x^2/12 + x^3/60 ...))
   *     = 1 + x (1 + x/2 (1 + x/3 (1 + x/4 + x^2/20 ...)))
   *     = 1 + x (1 + x/2 (1 + x/3 (1 + x/4 (1 + x/5 ...))))
   */
  int32_t result = AHOY_FIXED_1;
  for (uint8_t i = 5; i > 0; --i) {
    result = AHOY_FIXED_1 + result * x / i / AHOY_FIXED_1;
  }

  while (halvings--) {
    result *= result;
  }

  return result;
}

/* Compute the fixed-point logarithm function in base-e, log(x) = ln x.
 *
 * We first scale x down successively by 2, until x is in the range [1/2, 2],
 * adding ln(2) for each scaling. Then we use the Taylor series approximation of
 * log(x) at x = 1 to compute the final value.
 */
ahoy_fixed_t ahoy_fixed_log(ahoy_fixed_t x) {
  if (x <= 0) return -AHOY_FIXED_1;

  ahoy_fixed_t result = 0;

  /* log x = log x/2 + log 2 */
  const ahoy_fixed_t _3_2 = AHOY_FIXED_1 * 3 / 2;
  while (x >= _3_2) {
    result += AHOY_FIXED_LOG2;
    x >>= 1;
  }

  /* log x = log 2x - log 2 */
  const ahoy_fixed_t _1_2 = AHOY_FIXED_1 >> 1;
  while (x <= _1_2) {
    result -= AHOY_FIXED_LOG2;
    x <<= 1;
  }

  /* At this point x \in (1/2, 3/2). Let u = x-1. Taylor series at x = 1 is
   * log x = u - u^2/2 + u^3/3 - u^4/4 + u^5/5 ...
   *       = u (1 - u/2 + u^2/3 - u^3/4 + u^4/5 ...)
   *       = u (1 - u (1/2 - u/3 + u^2/4 - u^3/5 ...))
   *       = u (1 - u (1/2 - u (1/3 - u/4 + u^2/5 ...)))
   *       = u (1 - u (1/2 - u (1/3 - u (1/4 - u/5 ...))))
   *       = u (1 - u (1/2 - u (1/3 - u (1/4 - u (1/5 ...)))))
   */
  const ahoy_fixed_t u = x - AHOY_FIXED_1;
  for (uint8_t i = 5; i > 0; --i) {
    result = AHOY_FIXED_1 / i - u * result / AHOY_FIXED_1;
  }

  return result;
}

/* Compute the fixed-point modulus, numer % denom.
 *
 * This returns the remainder after removing as many whole-number multiples of
 * denom from numer as possible.
 */
ahoy_fixed_t ahoy_fixed_mod(ahoy_fixed_t numer, ahoy_fixed_t denom) {
  if (denom == 0) return 0;
  denom = AHOY_ABS(denom);
  return numer - denom * (numer / denom);
}

/* Compute base^exponent using fixed-point representation.
 */
ahoy_fixed_t ahoy_fixed_pow(ahoy_fixed_t base, ahoy_fixed_t exponent) {
  /* b^e = exp(log(b^e)) = exp(e log b) */
  return ahoy_fixed_exp(exponent * ahoy_fixed_log(base) / AHOY_FIXED_1);
}
