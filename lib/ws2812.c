#include <math.h>
#include <string.h>

#include "esp8266/gpio_struct.h"

#include "pirate/base.h"
#include "pirate/ws2812.h"

static const char* TAG = "ws2812";

/* One "nop" assembler instruction runs in ~1 CPU tick. At 80MHz, 8 ticks runs
 * in 100ns, so 300 ns = 24 = 8 * 3 ticks and 800 ns = 64 = 8 * 8 ticks. */
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
  bit ? asm volatile (LONG_NOP) : asm volatile (SHORT_NOP);
  GPIO.out_w1tc = 1 << pin;
  asm volatile (LONG_NOP);
}

/* https://sub.nanona.fi/esp8266/timing-and-ticks.html */
static void check_timings(uint16_t pin) {
  uint32_t start, end;
  taskENTER_CRITICAL();
  ESP_LOGI(TAG, " ");
  ESP_LOGI(TAG, "Profiling CPU timings...");
  os_delay_us(10);
  asm volatile (LONG_NOP);
  start = soc_get_ccount(); asm volatile ("nop"); end = soc_get_ccount();
  ESP_LOGI(TAG, "asm nop = %d ticks (target 1)", end - start - 1);
  start = soc_get_ccount(); GPIO.out_w1tc = 1 << pin; end = soc_get_ccount();
  ESP_LOGI(TAG, "GPIO set low = %d ticks (target ~10)", end - start - 1);
  start = soc_get_ccount(); GPIO.out_w1ts = 1 << pin; end = soc_get_ccount();
  ESP_LOGI(TAG, "GPIO set high = %d ticks (target ~10)", end - start - 1);
  start = soc_get_ccount(); asm volatile (_SHORT); end = soc_get_ccount();
  ESP_LOGI(TAG, "short delay = %d ticks (target 24)", end - start - 1);
  start = soc_get_ccount(); asm volatile (_LONG); end = soc_get_ccount();
  ESP_LOGI(TAG, "long delay = %d ticks (target 64)", end - start - 1);
  start = soc_get_ccount(); write_bit(pin, 0); end = soc_get_ccount();
  ESP_LOGI(TAG, "0-bit = %d ticks (target ~100)", end - start - 1);
  start = soc_get_ccount(); write_bit(pin, 1); end = soc_get_ccount();
  ESP_LOGI(TAG, "1-bit = %d ticks (target ~150)", end - start - 1);
  start = soc_get_ccount(); os_delay_us(10); end = soc_get_ccount();
  ESP_LOGI(TAG, "10us delay = %d ticks (target ~800)", end - start - 1);
  taskEXIT_CRITICAL();
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
      write_word_grbw(pin, ws2812->words[i]);
    }
    taskEXIT_CRITICAL();
  } else if (ws2812->channels == AHOY_WS2812_GRB) {
    taskENTER_CRITICAL();
    for (int i = 0; i < count; ++i) {
      write_word_grb(pin, ws2812->words[i]);
    }
    taskEXIT_CRITICAL();
  }
  os_delay_us(10);
}
