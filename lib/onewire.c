#include "lib8266/onewire.h"

#include "esp_timer.h"

static const char* TAG = "⚓ onewire";

bool ahoy_onewire_reset(uint8_t pin) {
  portENTER_CRITICAL();

  const int32_t mark = ahoy_ccount();

  /* Pull low for at least 480us. */
  AHOY_GPIO_ENABLE_OUTPUT(pin);
  AHOY_GPIO_WRITE_L(pin);
  ahoy_busy_wait_us(480);

  const int32_t elapsed0 = ahoy_ccount() - mark;

  /* Wait 20us for pin to go high. */
  AHOY_GPIO_DISABLE_OUTPUT(pin);
  ahoy_busy_wait_us(20);

  const int32_t elapsed1 = ahoy_ccount() - mark - elapsed0;

  /* Read pin for 480us. If it goes low, a device is present. */
  bool device_present = 0;
  const int64_t clock = esp_timer_get_time();
  while (esp_timer_get_time() - clock < 480)
    device_present |= AHOY_GPIO_READ(pin) == 0;

  const int32_t elapsed2 = ahoy_ccount() - mark - elapsed0 - elapsed1;

  portEXIT_CRITICAL();

  ESP_LOGD(TAG, "reset! %d + %d + %d us", elapsed0 / 160, elapsed1 / 160, elapsed2 / 160);

  return device_present;
}

bool ahoy_onewire_read_bit(uint8_t pin) {
  portENTER_CRITICAL();

  const int32_t mark = ahoy_ccount();

  AHOY_GPIO_ENABLE_OUTPUT(pin);
  AHOY_GPIO_WRITE_L(pin);
  AHOY_GPIO_DISABLE_OUTPUT(pin);

  const int32_t elapsed0 = ahoy_ccount() - mark;

  /* We then sample the bus after a short delay. */
  ahoy_busy_wait_us(1);
  const bool value = AHOY_GPIO_READ(pin);

  const int32_t elapsed1 = ahoy_ccount() - mark - elapsed0;

  portEXIT_CRITICAL();

  /* Wait another 60us to finish the read time slot. */
  ahoy_busy_wait_us(60);

  ESP_LOGD(TAG, "read %d in %d + %d us", value, elapsed0, elapsed1);

  return value;
}

uint8_t ahoy_onewire_read_byte(uint8_t pin) {
  uint8_t bits = 0;
  if (ahoy_onewire_read_bit(pin)) bits |= 0b00000001;
  vTaskDelay(1);
  if (ahoy_onewire_read_bit(pin)) bits |= 0b00000010;
  vTaskDelay(1);
  if (ahoy_onewire_read_bit(pin)) bits |= 0b00000100;
  vTaskDelay(1);
  if (ahoy_onewire_read_bit(pin)) bits |= 0b00001000;
  vTaskDelay(1);
  if (ahoy_onewire_read_bit(pin)) bits |= 0b00010000;
  vTaskDelay(1);
  if (ahoy_onewire_read_bit(pin)) bits |= 0b00100000;
  vTaskDelay(1);
  if (ahoy_onewire_read_bit(pin)) bits |= 0b01000000;
  vTaskDelay(1);
  if (ahoy_onewire_read_bit(pin)) bits |= 0b10000000;
  vTaskDelay(1);
  return bits;
}

void ahoy_onewire_write_bit(uint8_t pin, bool value) {
  portENTER_CRITICAL();

  const int64_t clock = ahoy_ccount();

  AHOY_GPIO_ENABLE_OUTPUT(pin);
  AHOY_GPIO_WRITE_L(pin);

  /* To write a 1, the master holds the bus low for at most 15us. To
     write a 0, the master holds the bus low for at least 60us. */
  if (value == 0) ahoy_busy_wait_us(60);
  AHOY_GPIO_DISABLE_OUTPUT(pin);

  const int32_t elapsed0 = ahoy_ccount() - clock;

  /* Each write time slot must total at least 60us. We'll make it 70. */
  ahoy_busy_wait_us(value ? 60 : 5);

  const int32_t elapsed1 = ahoy_ccount() - clock - elapsed0;

  portEXIT_CRITICAL();

  ESP_LOGD(TAG, "write %d | %d + %d us", value, elapsed0 / 160, elapsed1 / 160);
}

void ahoy_onewire_write_byte(uint8_t pin, uint8_t bits) {
  ahoy_onewire_write_bit(pin, bits & 0b00000001);
  vTaskDelay(1);
  ahoy_onewire_write_bit(pin, bits & 0b00000010);
  vTaskDelay(1);
  ahoy_onewire_write_bit(pin, bits & 0b00000100);
  vTaskDelay(1);
  ahoy_onewire_write_bit(pin, bits & 0b00001000);
  vTaskDelay(1);
  ahoy_onewire_write_bit(pin, bits & 0b00010000);
  vTaskDelay(1);
  ahoy_onewire_write_bit(pin, bits & 0b00100000);
  vTaskDelay(1);
  ahoy_onewire_write_bit(pin, bits & 0b01000000);
  vTaskDelay(1);
  ahoy_onewire_write_bit(pin, bits & 0b10000000);
  vTaskDelay(1);
}

uint8_t ahoy_onewire_crc_8bytes(uint8_t *bytes) {
  uint8_t crc = 0;
  for (uint8_t b = 0; b < 8; ++b) {
    uint8_t bits = bytes[b];
    for (uint8_t i = 8; i; i--) {
      uint8_t mix = (crc ^ bits) & 0b1;
      crc >>= 1;
      if (mix) crc ^= 0x8C;
      bits >>= 1;
    }
  }
  return crc;
}
