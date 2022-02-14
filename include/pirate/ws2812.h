#ifndef PIRATE__WS2812__H__
#define PIRATE__WS2812__H__

#include "driver/gpio.h"

typedef enum {
  AHOY_WS2812_GRB,
  AHOY_WS2812_GRBW,
} ahoy_ws2812_channels_t;

typedef struct ahoy_ws2812 *ahoy_ws2812_handle;

ahoy_ws2812_handle ahoy_ws2812_init(uint16_t pin, uint32_t count, ahoy_ws2812_channels_t channels);

void ahoy_ws2812_set(ahoy_ws2812_handle ws2812, uint32_t count, uint32_t *locations, uint32_t *words);

esp_err_t ahoy_ws2812_render(const ahoy_ws2812_handle ws2812);

#endif
