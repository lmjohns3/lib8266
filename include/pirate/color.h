#ifndef PIRATE__COLOR__H__
#define PIRATE__COLOR__H__

#include "pirate/base.h"
#include "pirate/fixed.h"

void ahoy_color_rgb_to_hsi(uint32_t rgb, ahoy_fixed_t *hsi);

uint32_t ahoy_color_hsi_to_rgbw(const ahoy_fixed_t *hsi);

#endif
