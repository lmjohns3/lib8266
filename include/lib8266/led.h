#ifndef LIB8266__LED__H__
#define LIB8266__LED__H__

#include "lib8266/pwm.h"

typedef struct ahoy_led *ahoy_led_handle;

esp_err_t ahoy_led_set(const ahoy_led_handle led, uint32_t hex);

void ahoy_led_fade(const ahoy_led_handle led, uint32_t hex);

ahoy_led_rgbw_handle ahoy_led_init(const uint8_t *channels,
                                   uint16_t fade_ms,
                                   uint16_t fps,
                                   uint16_t queue_size);

#endif
