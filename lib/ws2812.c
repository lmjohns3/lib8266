#include <math.h>
#include <string.h>

#include "esp8266/gpio_struct.h"

#include "lib8266/base.h"
#include "lib8266/ws2812.h"

// One "nop" assembler instruction runs in ~1 CPU tick. At 80MHz, 8
// CPU ticks = 100ns, so 300ns = 3 * 100ns = 3 * (8 CPU ticks) = 24
// CPU ticks; similarly, 800 ns = 64 CPU ticks.
 
#define SHORT_NOP "nop;nop;nop;nop;nop;nop;nop;nop;" \
                  "nop;nop;nop;nop;nop;nop;nop;nop;" \
                  "nop;nop;nop;nop;nop;nop;nop;nop;"
#define LONG_NOP  "nop;nop;nop;nop;nop;nop;nop;nop;" \
                  "nop;nop;nop;nop;nop;nop;nop;nop;" \
                  "nop;nop;nop;nop;nop;nop;nop;nop;" \
                  "nop;nop;nop;nop;nop;nop;nop;nop;" \
                  "nop;nop;nop;nop;nop;nop;nop;nop;" \
                  "nop;nop;nop;nop;nop;nop;nop;nop;" \
                  "nop;nop;nop;nop;nop;nop;nop;nop;" \
                  "nop;nop;nop;nop;nop;nop;nop;nop;"

static inline void write_bit(uint16_t pin, uint32_t bit) {
  GPIO.out_w1ts = 1 << pin;
  if (bit) {
    asm volatile (LONG_NOP);
  } else {
    asm volatile (SHORT_NOP);
  }
  GPIO.out_w1tc = 1 << pin;
  asm volatile (LONG_NOP);
}

/* With a generic gamma value, we'd need to cast to float, divide by 255, call
 * pow, multiply by 255, and cast back to int. If we set gamma == 2 (which is a
 * ballpark correct value for many cases), we can simplify 255 * (b / 255)^2 to
 * ~ b^2 / 256 */
#define GAMMA2(b) (((b) * (b)) >> 8)

static uint32_t adjust_gamma2(uint32_t word) {
  return
     (GAMMA2((word & 0xff000000) >> 24) << 24) |
     (GAMMA2((word & 0x00ff0000) >> 16) << 16) |
     (GAMMA2((word & 0x0000ff00) >> 8) << 8) |
     (GAMMA2((word & 0x000000ff)));
}

struct ahoy_ws2812 {
  uint16_t pin;
  uint32_t count;
  ahoy_ws2812_channels_t channels;
  uint32_t *words;
};

ahoy_ws2812_handle ahoy_ws2812_init(uint16_t pin, uint32_t count, ahoy_ws2812_channels_t channels) {
  gpio_config_t cfg;
  cfg.intr_type = GPIO_INTR_DISABLE;
  cfg.mode = GPIO_MODE_OUTPUT;
  cfg.pin_bit_mask = 1ULL << pin;
  cfg.pull_down_en = 0;
  cfg.pull_up_en = 0;
  gpio_config(&cfg);

  ahoy_ws2812_handle ws2812 = malloc(sizeof(struct ahoy_ws2812));
  ws2812->pin = pin;
  ws2812->count = count;
  ws2812->channels = channels;
  ws2812->words = malloc(count * sizeof(uint32_t));
  return ws2812;
}

void ahoy_ws2812_set(ahoy_ws2812_handle ws2812, uint32_t count, uint32_t *locations, uint32_t *words) {
  for (int i = 0; i < count; ++i) {
    ws2812->words[locations[i]] = words[i];
  }
}

static inline void write_word_grb(uint16_t pin, uint32_t rgb) {
  /* green channel */
  write_bit(pin, rgb & 0x008000);
  write_bit(pin, rgb & 0x004000);
  write_bit(pin, rgb & 0x002000);
  write_bit(pin, rgb & 0x001000);
  write_bit(pin, rgb & 0x000800);
  write_bit(pin, rgb & 0x000400);
  write_bit(pin, rgb & 0x000200);
  write_bit(pin, rgb & 0x000100);
  /* red channel */
  write_bit(pin, rgb & 0x800000);
  write_bit(pin, rgb & 0x400000);
  write_bit(pin, rgb & 0x200000);
  write_bit(pin, rgb & 0x100000);
  write_bit(pin, rgb & 0x080000);
  write_bit(pin, rgb & 0x040000);
  write_bit(pin, rgb & 0x020000);
  write_bit(pin, rgb & 0x010000);
  /* blue channel */
  write_bit(pin, rgb & 0x000080);
  write_bit(pin, rgb & 0x000040);
  write_bit(pin, rgb & 0x000020);
  write_bit(pin, rgb & 0x000010);
  write_bit(pin, rgb & 0x000008);
  write_bit(pin, rgb & 0x000004);
  write_bit(pin, rgb & 0x000002);
  write_bit(pin, rgb & 0x000001);
}

static inline void write_word_grbw(uint16_t pin, uint32_t rgbw) {
  /* green channel */
  write_bit(pin, rgbw & 0x00800000);
  write_bit(pin, rgbw & 0x00400000);
  write_bit(pin, rgbw & 0x00200000);
  write_bit(pin, rgbw & 0x00100000);
  write_bit(pin, rgbw & 0x00080000);
  write_bit(pin, rgbw & 0x00040000);
  write_bit(pin, rgbw & 0x00020000);
  write_bit(pin, rgbw & 0x00010000);
  /* red channel */
  write_bit(pin, rgbw & 0x80000000);
  write_bit(pin, rgbw & 0x40000000);
  write_bit(pin, rgbw & 0x20000000);
  write_bit(pin, rgbw & 0x10000000);
  write_bit(pin, rgbw & 0x08000000);
  write_bit(pin, rgbw & 0x04000000);
  write_bit(pin, rgbw & 0x02000000);
  write_bit(pin, rgbw & 0x01000000);
  /* blue channel */
  write_bit(pin, rgbw & 0x00008000);
  write_bit(pin, rgbw & 0x00004000);
  write_bit(pin, rgbw & 0x00002000);
  write_bit(pin, rgbw & 0x00001000);
  write_bit(pin, rgbw & 0x00000800);
  write_bit(pin, rgbw & 0x00000400);
  write_bit(pin, rgbw & 0x00000200);
  write_bit(pin, rgbw & 0x00000100);
  /* white channel */
  write_bit(pin, rgbw & 0x00000080);
  write_bit(pin, rgbw & 0x00000040);
  write_bit(pin, rgbw & 0x00000020);
  write_bit(pin, rgbw & 0x00000010);
  write_bit(pin, rgbw & 0x00000008);
  write_bit(pin, rgbw & 0x00000004);
  write_bit(pin, rgbw & 0x00000002);
  write_bit(pin, rgbw & 0x00000001);
}

void ahoy_ws2812_render(const ahoy_ws2812_handle ws2812) {
  const uint16_t pin = ws2812->pin;
  const uint32_t count = ws2812->count;
  if (ws2812->channels == AHOY_WS2812_GRBW) {
    taskENTER_CRITICAL();
    for (int i = 0; i < count; ++i) {
      write_word_grbw(pin, adjust_gamma2(ws2812->words[i]));
    }
    taskEXIT_CRITICAL();
  } else if (ws2812->channels == AHOY_WS2812_GRB) {
    taskENTER_CRITICAL();
    for (int i = 0; i < count; ++i) {
      write_word_grb(pin, adjust_gamma2(ws2812->words[i]));
    }
    taskEXIT_CRITICAL();
  }
  os_delay_us(10);
}
