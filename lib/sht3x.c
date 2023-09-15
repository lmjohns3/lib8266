#include "lib8266/io.h"
#include "lib8266/sht3x.h"

static const char *TAG = "⚓ sht3x";

#define IGNORE_ACK 0
#define CHECK_ACK 1

#define SEND_ACK 0
#define SEND_NACK 1
#define SEND_NACK_LAST 2

#define ADDRESS 0x44

static const uint8_t oneshot_commands_with_stretching[3][2] = { { 0x2c, 0x06 }, { 0x2c, 0x0d }, { 0x2c, 0x10 } };
static const uint8_t oneshot_commands_without_stretching[3][2] = { { 0x24, 0x00 }, { 0x24, 0x0b }, { 0x24, 0x16 } };

static esp_err_t wait_for_measurement(const ahoy_sht3x_handle_t sht3x) {
  i2c_cmd_handle_t cmd;
  esp_err_t ret;

  ahoy_busy_wait_us(1000);

  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ADDRESS << 1 | I2C_MASTER_READ, CHECK_ACK);
  ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
  i2c_cmd_link_delete(cmd);
  if (ret != ESP_OK) return ret;

  AHOY_GPIO_DISABLE_OUTPUT(sht3x->scl_pin);
  const int64_t mark = esp_timer_get_time() + 1000 * sht3x->read_timeout_ms;
  while (AHOY_GPIO_READ(sht3x->scl_pin) == 0) {
    if (esp_timer_get_time() >= mark) {
      AHOY_GPIO_ENABLE_OUTPUT(sht3x->scl_pin);
      return ESP_ERR_TIMEOUT;
    }
  }

  AHOY_GPIO_ENABLE_OUTPUT(sht3x->scl_pin);
  ahoy_busy_wait_us(10);
  return ESP_OK;
}

esp_err_t ahoy_sht3x_init(const ahoy_sht3x_handle_t sht3x) {
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = sht3x->sda_pin;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = sht3x->scl_pin;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.clk_stretch_tick = sht3x->clk_stretch_tick;
  ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode));
  ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
  return ESP_OK;
}

esp_err_t ahoy_sht3x_read_once(const ahoy_sht3x_handle_t sht3x,
                               ahoy_fixed_t *temperature_degc,
                               ahoy_fixed_t *relative_humidity_pct) {
  const uint8_t *command = sht3x->read_with_clock_stretch
    ? oneshot_commands_with_stretching[sht3x->repeatability]
    : oneshot_commands_without_stretching[sht3x->repeatability];

  i2c_cmd_handle_t cmd;
  esp_err_t ret;
  uint8_t buf[6] = { 0, 0, 0, 0, 0, 0 };

  /* Send the measurement instruction. */
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ADDRESS << 1 | I2C_MASTER_WRITE, CHECK_ACK);
  i2c_master_write_byte(cmd, *command, CHECK_ACK);
  i2c_master_write_byte(cmd, *(command+1), CHECK_ACK);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
  i2c_cmd_link_delete(cmd);
  if (ret != ESP_OK) return ret;

  ret = wait_for_measurement(sht3x);
  if (ret != ESP_OK) return ret;

  /* Read out the measurement bytes. */
  cmd = i2c_cmd_link_create();
  i2c_master_read(cmd, buf, 6, SEND_NACK_LAST);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
  i2c_cmd_link_delete(cmd);
  if (ret != ESP_OK) return ret;

  ESP_LOGI(TAG, "(%d) got measurement %02x %02x %02x %02x %02x %02x",
           ret, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

  if (ahoy_crc8(0x31, buf, 2) != buf[2]) ESP_LOGI(TAG, "crc check failed for temperature");
  if (ahoy_crc8(0x31, buf+3, 2) != buf[5]) ESP_LOGI(TAG, "crc check failed for humidity");

  /* Convert raw values. The raw values from this sensor are scaled by
     2^16, which happens to be the fixed-point resolution we're using.
     The temperature conversion requires a linear adjustment (from the
     datasheet) to get degrees C. */
  *temperature_degc = (((uint32_t)buf[0] << 8) | buf[1]) * 175 - AHOY_TO_FIXED(45);
  *relative_humidity_pct = (((uint32_t)buf[3] << 8) | buf[4]) * 100;
  return ESP_OK;
}
