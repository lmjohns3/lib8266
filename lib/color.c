#include "lib8266/color.h"

void ahoy_color_rgb_to_hsi(uint32_t rgb, ahoy_fixed_t *hsi) {
  const ahoy_fixed_t
    _255 = AHOY_TO_FIXED(255),
    r = AHOY_TO_FIXED((rgb >> 16) & 0xff) / _255,
    g = AHOY_TO_FIXED((rgb >>  8) & 0xff) / _255,
    b = AHOY_TO_FIXED((rgb      ) & 0xff) / _255,
    intensity = (r + g + b) / AHOY_TO_FIXED(3),
    min = r < g ? (b < r ? b : r) : (b < g ? b : g),
    max = r > g ? (b > r ? b : r) : (b > g ? b : g),
    saturation = intensity == 0 ? 0 : AHOY_FIXED_1 - min / intensity,
    radius = AHOY_TO_FIXED(6) * (max - min),
    _1_3 = AHOY_FIXED_1 / 3,
    _2_3 = AHOY_FIXED_1 * 2 / 3,
    hue = AHOY_FIXED_FRACTIONAL_PART(max == min ?                     0 :
                                     max == r ?        (g - b) / radius :
                                     max == g ? _1_3 + (b - r) / radius :
                                                _2_3 + (r - g) / radius);
  hsi[0] = hue < 0 ? hue + AHOY_FIXED_1 : hue;
  hsi[1] = saturation;
  hsi[2] = intensity;
}

void ahoy_color_hsi_to_rgbw(const ahoy_fixed_t *hsi, ahoy_fixed_t *rgbw) {
  /* https://en.wikipedia.org/wiki/HSL_and_HSV#HSI_to_RGB */
  const ahoy_fixed_t
    hue = ahoy_fixed_mod(hsi[0], AHOY_FIXED_1),
    saturation = AHOY_FIXED_CLAMP_01(hsi[1]),
    intensity = AHOY_FIXED_CLAMP_01(hsi[2]),
    segment = AHOY_TO_FIXED(6) * hue,
    diff = ahoy_fixed_mod(segment, AHOY_TO_FIXED(2)) - AHOY_FIXED_1,
    z = AHOY_FIXED_1 - (diff < 0 ? -diff : diff),
    chroma = AHOY_TO_FIXED(3) * intensity * saturation / (AHOY_FIXED_1 + z);
  rgbw[0] = (segment < AHOY_FIXED_1 ? chroma :
             segment < AHOY_TO_FIXED(2) ? chroma * z :
             segment < AHOY_TO_FIXED(4) ? 0 :
             segment < AHOY_TO_FIXED(5) ? chroma * z : chroma);
  rgbw[1] = (segment < AHOY_FIXED_1 ? chroma * z :
             segment < AHOY_TO_FIXED(3) ? chroma :
             segment < AHOY_TO_FIXED(4) ? chroma * z : 0);
  rgbw[2] = (segment < AHOY_TO_FIXED(2) ? 0 :
             segment < AHOY_TO_FIXED(3) ? chroma * z :
             segment < AHOY_TO_FIXED(5) ? chroma : chroma * z);
  rgbw[3] = (AHOY_FIXED_1 - saturation) * intensity;
}
