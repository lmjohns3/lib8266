#ifndef LIB8266__BME280__H__
#define LIB8266__BME280__H__

#include "lib8266/fixed.h"

#include "driver/i2c.h"

typedef struct {
  uint16_t scl_pin;
  uint16_t sda_pin;
  uint32_t fine_temperature;
  uint16_t tc[4];
  uint16_t hc[4];
  uint16_t pc[4];
} ahoy_bme280_t;

esp_err_t ahoy_bme280_init(ahoy_bme280_t *bme280);

esp_err_t ahoy_bme280_read_once(ahoy_bme280_t *bme280,
                                ahoy_fixed_t *temperature_degc,
                                ahoy_fixed_t *pressure_mmhg,
                                ahoy_fixed_t *relative_humidity_pct);

#endif
