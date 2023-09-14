#include "lib8266/io.h"
#include "lib8266/bme280.h"

static const char *TAG = "⚓ bme280";

#define IGNORE_ACK 0
#define CHECK_ACK 1

#define SEND_ACK 0
#define SEND_NACK 1
#define SEND_NACK_LAST 2

#define ADDRESS 0x77

/* Datasheet: https://cdn-learn.adafruit.com/assets/assets/000/115/588/original/bst-bme280-ds002.pdf */

/* Official code: https://github.com/boschsensortec/BME280_driver */

/* Third-party implementation: https://github.com/malokhvii-eduard/arduino-bme280 */

/* Helpful debugging page with example compensation values from 2 chips:
   https://community.bosch-sensortec.com/t5/MEMS-sensors-forum/Several-BME280-sensors-giving-invalid-values/td-p/11903 */


static esp_err_t install(ahoy_bme280_device_t *dev) {
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = dev->sda_pin;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = dev->scl_pin;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.clk_stretch_tick = dev->clk_stretch_tick;
  ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode));
  ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
  ESP_LOGI(TAG, "installed i2c driver");
  return ESP_OK;
}

static esp_err_t read_compensation_values(ahoy_bme280_device_t *dev) {
  uint8_t buf[26];
  i2c_cmd_handle_t cmd;
  esp_err_t ret;

  /* Read the first block of compensation values. */
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ADDRESS << 1 | I2C_MASTER_WRITE, CHECK_ACK);
  i2c_master_write_byte(cmd, 0x88, CHECK_ACK);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ADDRESS << 1 | I2C_MASTER_READ, CHECK_ACK);
  i2c_master_read(cmd, buf, 26, SEND_NACK_LAST);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
  i2c_cmd_link_delete(cmd);
  if (ret != ESP_OK) {
    ESP_LOGW(TAG, "error %d reading first calibration block", ret);
    return ret;
  }

  dev->dig_t1 = (buf[1] << 8) | buf[0];
  dev->dig_t2 = ((uint16_t)buf[3] << 8) | (uint16_t)buf[2];
  dev->dig_t3 = ((uint16_t)buf[5] << 8) | (uint16_t)buf[4];

  dev->dig_p1 = ((uint16_t)buf[7] << 8) | (uint16_t)buf[6];
  dev->dig_p2 = ((uint16_t)buf[9] << 8) | (uint16_t)buf[8];
  dev->dig_p3 = ((uint16_t)buf[11] << 8) | (uint16_t)buf[10];
  dev->dig_p4 = ((uint16_t)buf[13] << 8) | (uint16_t)buf[12];
  dev->dig_p5 = ((uint16_t)buf[15] << 8) | (uint16_t)buf[14];
  dev->dig_p6 = ((uint16_t)buf[17] << 8) | (uint16_t)buf[16];
  dev->dig_p7 = ((uint16_t)buf[19] << 8) | (uint16_t)buf[18];
  dev->dig_p8 = ((uint16_t)buf[21] << 8) | (uint16_t)buf[20];
  dev->dig_p9 = ((uint16_t)buf[23] << 8) | (uint16_t)buf[22];

  dev->dig_h1 = buf[25];  /* buf[24] (address 0xA0) is unused. */

  /* Read the second block. */
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ADDRESS << 1 | I2C_MASTER_WRITE, CHECK_ACK);
  i2c_master_write_byte(cmd, 0xE1, CHECK_ACK);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ADDRESS << 1 | I2C_MASTER_READ, CHECK_ACK);
  i2c_master_read(cmd, buf, 7, SEND_NACK_LAST);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
  i2c_cmd_link_delete(cmd);
  if (ret != ESP_OK) {
    ESP_LOGW(TAG, "error %d reading second calibration block", ret);
    return ret;
  }

  dev->dig_h2 = ((uint16_t)buf[1] << 8) | (uint16_t)buf[0];
  dev->dig_h3 = buf[2];
  dev->dig_h4 = ((uint16_t)buf[3] << 4) | ((uint16_t)buf[4] & 0b1111);
  dev->dig_h5 = ((uint16_t)buf[5] << 4) | ((uint16_t)buf[4] >> 4);
  dev->dig_h6 = buf[6];
  
  ESP_LOGI(TAG, "read compensation values:");
  ESP_LOGI(TAG, "T %u %d %d", dev->dig_t1, dev->dig_t2, dev->dig_t3);
  ESP_LOGI(TAG, "P %u %d %d %d %d %d %d %d %d",
           dev->dig_p1, dev->dig_p2, dev->dig_p3,
           dev->dig_p4, dev->dig_p5, dev->dig_p6,
           dev->dig_p7, dev->dig_p8, dev->dig_p9);
  ESP_LOGI(TAG, "H %u %d %u %d %d %u", dev->dig_h1, dev->dig_h2,
           dev->dig_h3, dev->dig_h4, dev->dig_h5, dev->dig_h6);

  return ESP_OK;
}

esp_err_t ahoy_bme280_init(ahoy_bme280_device_t *dev) {
  ESP_ERROR_CHECK(install(dev));
  ESP_ERROR_CHECK(read_compensation_values(dev));
  return ESP_OK;
}
  
/* Returns "t_fine" for pressure and humidity compensation. degC = t_fine / 5120. */
static int32_t compensate_t_fine(const ahoy_bme280_device_t *dev, int32_t raw) {
  int32_t var1 = (((int32_t)((raw / 8) - ((int32_t)dev->dig_t1 * 2))) * ((int32_t)dev->dig_t2)) / 2048;
  int32_t var2 = (int32_t)((raw / 16) - ((int32_t)dev->dig_t1));
  var2 = (((var2 * var2) / 4096) * ((int32_t)dev->dig_t3)) / 16384;
  return var1 + var2;
}

// Returns pressure in Pa; atmospheric pressure at sea level is about 100_000 Pa.
static uint32_t compensate_pressure(const ahoy_bme280_device_t *dev, int32_t raw) {
  int32_t var1 = (((int32_t)dev->t_fine) / 2) - (int32_t)64000;
  int32_t var2 = (((var1 / 4) * (var1 / 4)) / 2048) * ((int32_t)dev->dig_p6);
  var2 = var2 + ((var1 * ((int32_t)dev->dig_p5)) * 2);
  var2 = (var2 / 4) + (((int32_t)dev->dig_p4) * 65536);
  int32_t var3 = (dev->dig_p3 * (((var1 / 4) * (var1 / 4)) / 8192)) / 8;
  int32_t var4 = (((int32_t)dev->dig_p2) * var1) / 2;
  var1 = (var3 + var4) / 262144;
  var1 = (((32768 + var1)) * ((int32_t)dev->dig_p1)) / 32768;
  if (!var1) return 0;
  uint32_t var5 = (uint32_t)((uint32_t)1048576) - raw;
  uint32_t pressure = ((uint32_t)(var5 - (uint32_t)(var2 / 4096))) * 3125;
  pressure = (pressure < 0x80000000) ? (pressure << 1) / ((uint32_t)var1)
                                     : (pressure / (uint32_t)var1) * 2;
  var1 = (((int32_t)dev->dig_p9) * ((int32_t)(((pressure / 8) * (pressure / 8)) / 8192))) / 4096;
  var2 = (((int32_t)(pressure / 4)) * ((int32_t)dev->dig_p8)) / 8192;
  return (uint32_t)((int32_t)pressure + ((var1 + var2 + dev->dig_p7) / 16));
}

// Returns humidity in 1000 * %RH, a value from 0_000 to 100_000.
static uint32_t compensate_humidity(const ahoy_bme280_device_t *dev, int32_t raw) {
  int32_t var1 = dev->t_fine - ((int32_t)76800);
  int32_t var2 = (int32_t)(raw * 16384);
  int32_t var3 = (int32_t)(((int32_t)dev->dig_h4) * 1048576);
  int32_t var4 = ((int32_t)dev->dig_h5) * var1;
  int32_t var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;
  var2 = (var1 * ((int32_t)dev->dig_h6)) / 1024;
  var3 = (var1 * ((int32_t)dev->dig_h3)) / 2048;
  var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;
  var2 = ((var4 * ((int32_t)dev->dig_h2)) + 8192) / 16384;
  var3 = var5 * var2;
  var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
  var5 = var3 - ((var4 * ((int32_t)dev->dig_h1)) / 16);
  return (uint32_t)(var5 < 0 ? 0 : var5 > 409600000 ? 100000 : var5 / 4096);
  }

esp_err_t ahoy_bme280_read_once(ahoy_bme280_device_t *dev, ahoy_bme280_measurements_t *out) {
  i2c_cmd_handle_t cmd;
  esp_err_t ret;
  uint8_t buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  /* Set up measurements -- hard-coded oversampling and filter settings for now. */
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ADDRESS << 1 | I2C_MASTER_WRITE, CHECK_ACK);
  i2c_master_write_byte(cmd, 0xF2, CHECK_ACK);
  i2c_master_write_byte(cmd, 0b00000001, CHECK_ACK);
  i2c_master_write_byte(cmd, 0xF4, CHECK_ACK);
  i2c_master_write_byte(cmd, 0b00100101, CHECK_ACK);
  i2c_master_write_byte(cmd, 0xF5, CHECK_ACK);
  i2c_master_write_byte(cmd, 0b00000000, CHECK_ACK);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
  i2c_cmd_link_delete(cmd);
  if (ret != ESP_OK) return ret;

  /* Poll the "measuring" status bit on the sensor until it returns to 0. */
  for (;;) {
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ADDRESS << 1 | I2C_MASTER_WRITE, CHECK_ACK);
    i2c_master_write_byte(cmd, 0xF3, CHECK_ACK);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ADDRESS << 1 | I2C_MASTER_READ, CHECK_ACK);
    i2c_master_read(cmd, buf, 1, SEND_NACK_LAST);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) return ret;
    if ((buf[0] & 0b1000) == 0) break;
  }

  /* Read out the measurement data. */
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ADDRESS << 1 | I2C_MASTER_WRITE, CHECK_ACK);
  i2c_master_write_byte(cmd, 0xF7, CHECK_ACK);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ADDRESS << 1 | I2C_MASTER_READ, CHECK_ACK);
  i2c_master_read(cmd, buf, 8, SEND_NACK_LAST);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
  i2c_cmd_link_delete(cmd);
  if (ret != ESP_OK) return ret;

  ESP_LOGD(TAG, "read sensor values: "
           "P [%02x %02x %02x ==> %u] "
           "T [%02x %02x %02x ==> %u] "
           "H [%02x %02x ==> %u]",
           buf[0], buf[1], buf[2], (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4),
           buf[3], buf[4], buf[5], (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4),
           buf[6], buf[7], (buf[6] << 8) | buf[7]);

  dev->t_fine = compensate_t_fine(dev, ((((buf[3] << 8) | buf[4]) << 8) | buf[5]) >> 4);

  out->temperature_degc =
    AHOY_TO_FIXED((dev->t_fine * 5 + 128) / 256) / 100;
  out->pressure_pa =
    compensate_pressure(dev, ((((buf[0] << 8) | buf[1]) << 8) | buf[2]) >> 4);
  out->relative_humidity_pct =
    AHOY_TO_FIXED(compensate_humidity(dev, (buf[6] << 8) | buf[7]) / 10) / 100;

  return ESP_OK;
}
