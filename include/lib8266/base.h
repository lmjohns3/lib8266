#ifndef LIB8266__BASE__H__
#define LIB8266__BASE__H__

#include "esp_clk.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define AHOY_MAX(a, b) ((a) > (b) ? (a) : (b))
#define AHOY_MIN(a, b) ((a) < (b) ? (a) : (b))
#define AHOY_ABS(x) ((x) < 0 ? (0-(x)) : (x))

/* Get the current unix timestamp. */
time_t ahoy_now();

/* Get the clock counter. */
uint32_t ahoy_ccount();

/* Busy-wait (at least) the given number of microseconds or clock ticks. */
void ahoy_busy_wait_us(uint32_t wait_us);
void ahoy_busy_wait_ticks(uint32_t ticks);

uint8_t ahoy_crc8(uint8_t polynomial, uint8_t *bytes, uint8_t num_bytes);

#endif
