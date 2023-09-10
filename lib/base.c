#include "lib8266/base.h"

#include "esp_clk.h"

time_t ahoy_now() {
  time_t now = 0;
  time(&now);
  return now;
}

inline uint32_t ahoy_ccount() {
  uint32_t r;
  asm volatile ("rsr %0, ccount" : "=r"(r));
  return r;
}

inline void ahoy_busy_wait_us(uint32_t wait_us) {
  ahoy_busy_wait_ticks(wait_us * (esp_clk_cpu_freq() / 1000000));
}

inline void ahoy_busy_wait_ticks(uint32_t ticks) {
  const uint32_t start = ahoy_ccount();
  const uint32_t stop = start + ticks;
  if (stop < start) { while (ahoy_ccount() > stop) ;; }
  while (ahoy_ccount() < stop) ;;
}

/* https://stackoverflow.com/questions/51752284/how-to-calculate-crc8-in-c */
uint8_t ahoy_crc8(uint8_t polynomial, uint8_t *bytes, uint8_t num_bytes) {
  uint16_t i = 0, j = 0, crc = 0xff;
  for (i = 0; i < num_bytes; ++i) {
    crc ^= bytes[i];
    crc = crc & 0x80 ? (crc << 1) ^ polynomial : crc << 1;
    crc = crc & 0x80 ? (crc << 1) ^ polynomial : crc << 1;
    crc = crc & 0x80 ? (crc << 1) ^ polynomial : crc << 1;
    crc = crc & 0x80 ? (crc << 1) ^ polynomial : crc << 1;
    crc = crc & 0x80 ? (crc << 1) ^ polynomial : crc << 1;
    crc = crc & 0x80 ? (crc << 1) ^ polynomial : crc << 1;
    crc = crc & 0x80 ? (crc << 1) ^ polynomial : crc << 1;
    crc = crc & 0x80 ? (crc << 1) ^ polynomial : crc << 1;
  }
  return crc & 0xff;
}
