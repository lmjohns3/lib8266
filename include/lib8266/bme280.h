#ifndef LIB8266__BME280__H__
#define LIB8266__BME280__H__

#include "lib8266/fixed.h"

#include "driver/i2c.h"

typedef enum {
  OVERSAMPLING_DISABLED = 0b000,
  OVERSAMPLING_1X = 0b001,
  OVERSAMPLING_2X = 0b010,
  OVERSAMPLING_4X = 0b011,
  OVERSAMPLING_8X = 0b100,
  OVERSAMPLING_16X = 0b101,
} ahoy_bme280_oversampling_t;

typedef enum {
  IIR_FILTER_DISABLED = 0b000,
  IIR_FILTER_2 = 0b001,
  IIR_FILTER_4 = 0b010,
  IIR_FILTER_8 = 0b011,
  IIR_FILTER_16 = 0b100,
} ahoy_bme280_iir_filter_t;

typedef struct {
  /* Configuration for the I²C interface. */

  int16_t scl_pin;
  int16_t sda_pin;
  int16_t clk_stretch_tick;

  /* Sensing configuration. */

  ahoy_bme280_oversampling_t temperature_oversampling;
  ahoy_bme280_oversampling_t humidity_oversampling;
  ahoy_bme280_oversampling_t pressure_oversampling;
  ahoy_bme280_iir_filter_t iir_filter;

  /* Values for compensating raw sensor readings. */

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
  ahoy_fixed_t relative_humidity_pct;
  uint32_t pressure_pa;
} ahoy_bme280_measurements_t;

esp_err_t ahoy_bme280_init(ahoy_bme280_device_t *dev);

esp_err_t ahoy_bme280_read_once(ahoy_bme280_device_t *dev, ahoy_bme280_measurements_t *out);

#endif
