#include "lib8266/io.h"
#include "lib8266/bme280.h"

static const char *TAG = "⚓ bme280";

#define IGNORE_ACK 0
#define CHECK_ACK 1

#define SEND_ACK 0
#define SEND_NACK 1
#define SEND_NACK_LAST 2

#define ADDRESS 0x77


//esp_err_t ahoy_bme280_init(ahoy_bme280_handle_t config) {
//  i2c_config_t ic;
//  ic.mode = I2C_MODE_MASTER;
//  ic.sda_io_num = config.sda_pin;
//  ic.sda_pullup_en = GPIO_PULLUP_ENABLE;
//  ic.scl_io_num = config.scl_pin;
//  ic.scl_pullup_en = GPIO_PULLUP_ENABLE;
//  ic.clk_stretch_tick = config.clk_stretch_tick;
//  ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, ic.mode));
//  ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &ic));
//  return ESP_OK;
//}
//
//// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
//// t_fine carries fine temperature as global value
//int32_t compensate_temperature(ahoy_bme280_state_t state, int32_t raw) {
//  const int32_t r3 = raw >> 3;
//  const int32_t var1 = (((r3 - (state->comp.t[0] << 1))) * state->comp.t[1]) >> 11;
//  const int32_t r4 = raw >> 4;
//  const int32_t var2 = ((((r4 - state->comp.t[0]) * (r4 - state->comp.t[0])) >> 12) * state->comp.t[2]) >> 14;
//  state->fine_temperature = var1 + var2;
//  return (state->fine_temperature * 5 + 128) >> 8;
//}
//
//// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
//// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
//uint32_t compensate_pressure(int32_t raw) {
//  int64_t var1, var2, p;
//  var1 = ((BME280_S64_t)t_fine) - 128000;
//  var2 = var1 * var1 * (BME280_S64_t)dig_P6;
//  var2 = var2 + ((var1*(BME280_S64_t)dig_P5)<<17);
//  var2 = var2 + (((BME280_S64_t)dig_P4)<<35);
//  var1 = ((var1 * var1 * (BME280_S64_t)dig_P3)>>8) + ((var1 * (BME280_S64_t)dig_P2)<<12);
//  var1 = (((((BME280_S64_t)1)<<47)+var1))*((BME280_S64_t)dig_P1)>>33;
//  if (var1 == 0) return 0;
//  p = 1048576-adc_P;
//  p = (((p<<31)-var2)*3125)/var1;
//  var1 = (((BME280_S64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
//  var2 = (((BME280_S64_t)dig_P8) * p) >> 19;
//  p = ((p + var1 + var2) >> 8) + (((BME280_S64_t)dig_P7)<<4);
//  return (BME280_U32_t)p;
//}
//
//// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
//// Output value of “47445” represents 47445/1024 = 46.333 %RH
//uint32_t compensate_humidity(int32_t raw) {
//  int32_t v_x1_u32r;
//  v_x1_u32r = (t_fine - ((BME280_S32_t)76800));
//  v_x1_u32r = (((((adc_H << 14) - (((BME280_S32_t)dig_H4) << 20) -
//                  (((BME280_S32_t)dig_H5) * v_x1_u32r)) + ((BME280_S32_t)16384)) >> 15) * (((((((v_x1_u32r * ((BME280_S32_t)dig_H6)) >> 10) * (((v_x1_u32r * ((BME280_S32_t)dig_H3)) >> 11) + ((BME280_S32_t)32768))) >> 10) + ((BME280_S32_t)2097152)) * ((BME280_S32_t)dig_H2) + 8192) >> 14));
//  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((BME280_S32_t)dig_H1)) >> 4));
//  v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
//  v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
//  return (BME280_U32_t)(v_x1_u32r>>12);
//}
//
//esp_err_t ahoy_bme280_read_once(ahoy_fixed_t *temperature_degc, ahoy_fixed_t *relative_humidity_pct) {
//  const uint8_t *command = oneshot_stretch_commands[repeatability];
//  i2c_cmd_handle_t cmd;
//  esp_err_t ret;
//  uint8_t buf[6] = { 0, 0, 0, 0, 0, 0 };
//
//  /* Send the measurement instruction. */
//  cmd = i2c_cmd_link_create();
//  i2c_master_start(cmd);
//  i2c_master_write_byte(cmd, ADDRESS << 1 | I2C_MASTER_WRITE, CHECK_ACK);
//  i2c_master_write_byte(cmd, *command, CHECK_ACK);
//  i2c_master_write_byte(cmd, *(command+1), CHECK_ACK);
//  i2c_master_stop(cmd);
//  ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
//  i2c_cmd_link_delete(cmd);
//  if (ret != ESP_OK) return ret;
//
//  /* Wait until a measurement is ready. */
//  ahoy_busy_wait_us(2000);
//
//  cmd = i2c_cmd_link_create();
//  i2c_master_start(cmd);
//  i2c_master_write_byte(cmd, ADDRESS << 1 | I2C_MASTER_READ, CHECK_ACK);
//  ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
//  i2c_cmd_link_delete(cmd);
//  if (ret != ESP_OK) return ret;
//
//  AHOY_GPIO_DISABLE_OUTPUT(scl_pin);
//  const int64_t timeout = esp_timer_get_time() + 1000000;
//  while (AHOY_GPIO_READ(scl_pin) == 0) {
//    if (esp_timer_get_time() >= timeout) {
//      AHOY_GPIO_ENABLE_OUTPUT(scl_pin);
//      return ESP_ERR_TIMEOUT;
//    }
//  }
//  AHOY_GPIO_ENABLE_OUTPUT(scl_pin);
//  ahoy_busy_wait_us(1);
//
//  /* Read out the measurement bytes. */
//  cmd = i2c_cmd_link_create();
//  i2c_master_read(cmd, buf, 6, SEND_NACK_LAST);
//  i2c_master_stop(cmd);
//  ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
//  i2c_cmd_link_delete(cmd);
//  ESP_LOGI(TAG, "(%d) got measurement %02x %02x %02x %02x %02x %02x",
//           ret, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
//  if (ret != ESP_OK) return ret;
//
//  if (ahoy_sht3x_crc(buf[0], buf[1]) != buf[2]) {
//    ESP_LOGI(TAG, "crc check failed for temperature");
//  }
//
//  if (ahoy_sht3x_crc(buf[3], buf[4]) != buf[5]) {
//    ESP_LOGI(TAG, "crc check failed for humidity");
//  }
//
//  /* Convert raw values. The raw values from this sensor are scaled by
//     2^16, which happens to be the fixed-point resolution we're using.
//     The temperature conversion requires a linear adjustment (from the
//     datasheet) to get degrees C. */
//  *temperature_degc = (((uint32_t)buf[0] << 8) | buf[1]) * 175 - AHOY_TO_FIXED(45);
//  *relative_humidity_pct = (((uint32_t)buf[3] << 8) | buf[4]) * 100;
//  return ESP_OK;
//}
