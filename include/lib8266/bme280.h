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

  int32_t t_fine;

  uint16_t dig_t1;
  int16_t dig_t2;
  int16_t dig_t3;

  uint16_t dig_p1;
  int16_t dig_p2;
  int16_t dig_p3;
  int16_t dig_p4;
  int16_t dig_p5;
  int16_t dig_p6;
  int16_t dig_p7;
  int16_t dig_p8;
  int16_t dig_p9;

  uint8_t dig_h1;
  int16_t dig_h2;
  uint8_t dig_h3;
  int16_t dig_h4;
  int16_t dig_h5;
  int8_t dig_h6;
} ahoy_bme280_device_t;

typedef struct {
  ahoy_fixed_t temperature_degc;
  uint32_t relative_humidity_pct;
  uint32_t pressure_pa;
} ahoy_bme280_measurements_t;

esp_err_t ahoy_bme280_init(ahoy_bme280_device_t *dev);

esp_err_t ahoy_bme280_read_once(ahoy_bme280_device_t *dev, ahoy_bme280_measurements_t *out);

#endif
