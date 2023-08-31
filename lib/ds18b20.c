#include "lib8266/ds18b20.h"

static const char *TAG = "⚓ ds18b20";

ahoy_fixed_t ahoy_ds18b20_read_degc(uint8_t pin) {
  if (!ahoy_onewire_reset(pin)) return AHOY_TO_FIXED(-999.f);

  ahoy_onewire_write_byte(pin, 0xCC); // skip rom.
  ahoy_onewire_write_byte(pin, 0x44); // initiate temperature conversion.

  while (ahoy_onewire_read_bit(pin) == 0) vTaskDelay(1);

  ahoy_onewire_reset(pin);
  ahoy_onewire_write_byte(pin, 0xCC); // skip rom.
  ahoy_onewire_write_byte(pin, 0xBE); // read scratchpad.

  uint8_t scratch[9];
  for (int i = 0; i < 9; ++i) {
    vTaskDelay(1);
    scratch[i] = ahoy_onewire_read_byte(pin);
    ESP_LOGD(TAG, "%d: %x", i, scratch[i]);
  }

  const uint8_t crc = ahoy_onewire_crc_8bytes(scratch);
  if (scratch[8] != crc) {
    ESP_LOGW(TAG, "payload CRC (%02x) != computed CRC (%02x)", scratch[8], crc);
    return AHOY_TO_FIXED(-9999.f);
  }

  // Scale measurement directly to fixed, without converting to float.
  // - Fixed uses 16 bits for fractional part.
  // - Temperature reading uses 4 bits for fractional part.
  return (((int32_t)scratch[1] << 8) | scratch[0]) << 12;
}
