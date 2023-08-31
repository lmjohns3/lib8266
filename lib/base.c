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
