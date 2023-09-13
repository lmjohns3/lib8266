#ifndef LIB8266__BME280__H__
#define LIB8266__BME280__H__

#include "lib8266/fixed.h"

#include "driver/i2c.h"

typedef struct {
  uint16_t scl_pin;
  uint16_t sda_pin;
  uint16_t clk_stretch_tick;

  /* Values for compensating raw sensor readings. Unless otherwise
     noted, each value actually holds a signed 16-bit value; the
     arithmetic for compensation requires larger containers to avoid
     losing precision. */

  int32_t t_fine;  // signed 32.

  int32_t dig_t1;  // unsigned 16.
  int32_t dig_t2;
  int32_t dig_t3;

  int64_t dig_p1;  // unsigned 16.
  int64_t dig_p2;
  int64_t dig_p3;
  int64_t dig_p4;
  int64_t dig_p5;
  int64_t dig_p6;
  int64_t dig_p7;
  int64_t dig_p8;
  int64_t dig_p9;

  int32_t dig_h1;  // unsigned 8.
  int32_t dig_h2;
  int32_t dig_h3;  // unsigned 8.
  int32_t dig_h4;
  int32_t dig_h5;
  int32_t dig_h6;  // signed 8.
} ahoy_bme280_t;

esp_err_t ahoy_bme280_init(ahoy_bme280_t *bme280);

esp_err_t ahoy_bme280_read_once(ahoy_bme280_t *bme280,
                                int8_t *temperature_degc,
                                uint32_t *pressure_pa,
                                int8_t *relative_humidity_pct);

#endif
