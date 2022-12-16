#ifndef LIB8266__COLOR__H__
#define LIB8266__COLOR__H__

#include "lib8266/base.h"
#include "lib8266/fixed.h"

void ahoy_color_rgb_to_hsi(uint32_t rgb, ahoy_fixed_t *hsi);

uint32_t ahoy_color_hsi_to_rgbw(const ahoy_fixed_t *hsi);

#endif
