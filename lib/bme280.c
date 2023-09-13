#include "lib8266/io.h"
#include "lib8266/bme280.h"

static const char *TAG = "⚓ bme280";

#define IGNORE_ACK 0
#define CHECK_ACK 1

#define SEND_ACK 0
#define SEND_NACK 1
#define SEND_NACK_LAST 2

#define ADDRESS 0x77


static esp_err_t install(ahoy_bme280_t *bme280) {
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = bme280->sda_pin;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = bme280->scl_pin;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.clk_stretch_tick = bme280->clk_stretch_tick;
  ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode));
  ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
  return ESP_OK;
}

static esp_err_t read_compensation_values(ahoy_bme280_t *bme280) {
  uint8_t buf[25];
  i2c_cmd_handle_t cmd;
  esp_err_t ret;

  /* Compensation values are stored in two blocks of registers. */

  /* Read the first block. */
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ADDRESS << 1 | I2C_MASTER_WRITE, CHECK_ACK);
  i2c_master_write_byte(cmd, 0x88, CHECK_ACK);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ADDRESS << 1 | I2C_MASTER_READ, CHECK_ACK);
  i2c_master_read(cmd, buf, 25, SEND_NACK_LAST);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
  i2c_cmd_link_delete(cmd);
  if (ret != ESP_OK) return ret;

  bme280->dig_t1 = ((uint16_t)buf[1] << 8) | buf[0];
  bme280->dig_t2 = ((int16_t)buf[3] << 8) | buf[2];
  bme280->dig_t3 = ((int16_t)buf[5] << 8) | buf[4];

  bme280->dig_p1 = ((uint16_t)buf[7] << 8) | buf[6];
  bme280->dig_p2 = ((int16_t)buf[9] << 8) | buf[8];
  bme280->dig_p3 = ((int16_t)buf[11] << 8) | buf[10];
  bme280->dig_p4 = ((int16_t)buf[13] << 8) | buf[12];
  bme280->dig_p5 = ((int16_t)buf[15] << 8) | buf[14];
  bme280->dig_p6 = ((int16_t)buf[17] << 8) | buf[16];
  bme280->dig_p7 = ((int16_t)buf[19] << 8) | buf[18];
  bme280->dig_p8 = ((int16_t)buf[21] << 8) | buf[20];
  bme280->dig_p9 = ((int16_t)buf[23] << 8) | buf[22];

  bme280->dig_h1 = buf[24];

  /* Read the second block. */
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ADDRESS << 1 | I2C_MASTER_WRITE, CHECK_ACK);
  i2c_master_write_byte(cmd, 0xE1, CHECK_ACK);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ADDRESS << 1 | I2C_MASTER_READ, CHECK_ACK);
  i2c_master_read(cmd, buf, 6, SEND_NACK_LAST);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
  i2c_cmd_link_delete(cmd);
  if (ret != ESP_OK) return ret;

  bme280->dig_h2 = ((int16_t)buf[1] << 8) | buf[0];
  bme280->dig_h3 = buf[2];
  bme280->dig_h4 = ((uint16_t)buf[3] << 4) | (buf[4] & 0b00001111);
  bme280->dig_h5 = ((uint16_t)buf[5] << 4) | ((buf[4] & 0b11110000) >> 4);
  
  return ESP_OK;
}

esp_err_t ahoy_bme280_init(ahoy_bme280_t *bme280) {
  ESP_ERROR_CHECK(install(bme280));
  ESP_ERROR_CHECK(read_compensation_values(bme280));
  return ESP_OK;
}
  
/* Returns "t_fine" for computing pressure and humidity compensation.
   Temperature in degC = t_fine / 5120. */
static int32_t compensate_t_fine(const ahoy_bme280_t *bme280, int32_t raw) {
  const int32_t v1 = (raw >> 3) - (bme280->dig_t1 << 1);
  const int32_t v2 = ((raw >> 4) - bme280->dig_t1) >> 6;
  return ((v1 * bme280->dig_t2) >> 11) + ((v2 * v2 * bme280->dig_t3) >> 14);
}

// Returns pressure in Pa.
static uint32_t compensate_pressure(const ahoy_bme280_t *bme280, int32_t raw) {
  int32_t v1 = (bme280->t_fine >> 1) - 64000;
  int32_t v2 = (((((v1 >> 2) * (v1 >> 2)) >> 11) * bme280->dig_p6 +
                 (v1 * bme280->dig_p5 << 1)) >> 2) + (bme280->dig_p4 << 16);
  v1 = ((32768 + ((((bme280->dig_p3 * (((v1 >> 2) * (v1 >> 2)) >> 13)) >> 3) +
                   ((bme280->dig_p2 * v1) >> 1)) >> 18)) * bme280->dig_p1) >> 15;
  if (v1 == 0) return 0;
  uint32_t p = (((uint32_t)(1048576 - raw) - (v2 >> 12))) * 3125;
  p = p & 0x80000000 ? (p / (uint32_t)v1) << 1 : (p << 1) / (uint32_t)v1;
  return (uint32_t)((int32_t)p + ((((bme280->dig_p9 * ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12) +
                                   ((((int32_t)(p >> 2)) * bme280->dig_p8) >> 13) +
                                   bme280->dig_p7) >> 4));
}

// Returns humidity in %RH, a value from 0 to 100.
static int8_t compensate_humidity(const ahoy_bme280_t *bme280, int32_t raw) {
  int32_t v = bme280->t_fine - 76800;
  v =
    ((((raw << 14) - (bme280->dig_h4 << 20) - bme280->dig_h5 * v) + 16384) >> 15) *
    (((((((v * bme280->dig_h6) >> 10) *
         (((v * bme280->dig_h3) >> 11) + 32768)) >> 10) + 2097152) * bme280->dig_h2 + 8192) >> 14);
  v -= ((((v >> 15) * (v >> 15)) >> 7) * bme280->dig_h1) >> 4;
  /* The code from the datasheet shifts right 12 bits to return a fixed-point value in
     22.10 format. The sensor itself is only accurate to +/- 1% or so, so we'll shift
     those bits off and return an integer value. */
  return v < 0 ? 0 : v >> 22;
}

esp_err_t ahoy_bme280_read_once(ahoy_bme280_t *bme280,
                                int8_t *temperature_degc,
                                uint32_t *pressure_pa,
                                int8_t *relative_humidity_pct) {
  i2c_cmd_handle_t cmd;
  esp_err_t ret;
  uint8_t buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

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

  bme280->t_fine = compensate_t_fine(bme280, ((uint32_t)buf[3] << 12) | ((uint32_t)buf[4] << 4) | (buf[5] >> 4));
  *temperature_degc = (bme280->t_fine * 5 + 128) >> 8;
  *pressure_pa = compensate_pressure(bme280, ((uint32_t)buf[0] << 12) | ((uint32_t)buf[1] << 4) | (buf[2] >> 4));
  *relative_humidity_pct = compensate_humidity(bme280, ((uint32_t)buf[6] << 8) | buf[7]);

  return ESP_OK;
}
