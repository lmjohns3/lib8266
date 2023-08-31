#include "FreeRTOS.h"
#include "queue.h"

#include "lib8266/color.h"
#include "lib8266/led.h"

static const char *TAG = "⚓ led";

struct ahoy_led {
  uint8_t channels[4];
  uint16_t frames;
  TickType_t period;
  QueueHandle_t targets_queue;
};

esp_err_t ahoy_led_set(const ahoy_led_handle led, uint32_t hex) {
  ahoy_fixed_t hsi[3], rgbw[4];
  ahoy_color_rgb_to_hsi(hex, hsi);
  ahoy_color_hsi_to_rgbw(hsi, rgbw);
  return ahoy_pwm_set(4, led->channels, rgbw);
}

void ahoy_led_fade(ahoy_led_handle led, uint32_t hex) {
  xQueueSend(led->targets_queue, (void *) hex, (TickType_t) 0);
}

static void fader(void *params) {
  ahoy_led_handle led = params;
  ahoy_fixed_t rgbw[4], cur_hsi[3], tgt_hsi[3];
  uint32_t hex;
  while (xQueueReceive(led->targets_queue, (void *) &hex, portMAX_DELAY)) {
    ESP_LOGI(TAG, "new color target: %08x", hex);
    ahoy_color_rgb_to_hsi(hex, tgt_hsi);
    const ahoy_fixed_t hue_diff = tgt_hsi[0] - cur_hsi[0],
      dhue = (hue_diff < -(AHOY_FIXED_1 >> 1) ? (hue_diff + AHOY_FIXED_1) :
              hue_diff > AHOY_FIXED_1 >> 1 ? (hue_diff - AHOY_FIXED_1) :
              hue_diff) / led->frames,
      dsat = (tgt_hsi[1] - cur_hsi[1]) / led->frames,
      dint = (tgt_hsi[2] - cur_hsi[2]) / led->frames;
    TickType_t last_wake_time = xTaskGetTickCount();
    for (uint16_t i = 0; i < led->frames; ++i) {
      ESP_LOGD(TAG, "fading HSI(%f, %f, %f) --> HSI(%f, %f, %f)",
               AHOY_FIXED_TO_FLOAT(cur_hsi[0]),
               AHOY_FIXED_TO_FLOAT(cur_hsi[1]),
               AHOY_FIXED_TO_FLOAT(cur_hsi[2]),
               AHOY_FIXED_TO_FLOAT(tgt_hsi[0]),
               AHOY_FIXED_TO_FLOAT(tgt_hsi[1]),
               AHOY_FIXED_TO_FLOAT(tgt_hsi[2]));
      cur_hsi[0] = ahoy_fixed_mod(cur_hsi[0] + dhue, AHOY_FIXED_1);
      cur_hsi[1] = AHOY_FIXED_CLAMP_01(cur_hsi[1] + dsat);
      cur_hsi[2] = AHOY_FIXED_CLAMP_01(cur_hsi[2] + dint);
      ahoy_color_hsi_to_rgbw(cur_hsi, rgbw);
      ahoy_pwm_set(4, led->channels, rgbw);
      vTaskDelayUntil(&last_wake_time, led->period);
    }
  }
  vQueueDelete(led->targets_queue);
  free(led);
  vTaskDelete(NULL);
}

ahoy_led_handle ahoy_led_init(const uint8_t *channels,
                              uint16_t fade_ms,
                              uint16_t fps,
                              uint16_t queue_size) {
  ahoy_led_handle led = malloc(sizeof(struct ahoy_led));
  led->channels[0] = channels[0];
  led->channels[1] = channels[1];
  led->channels[2] = channels[2];
  led->channels[3] = channels[3];
  led->frames = fps * fade_ms / 1000;
  led->period = 1000 / fps / portTICK_PERIOD_MS;
  led->targets_queue = xQueueCreate(queue_size, sizeof(uint32_t));
  xTaskCreate(fader, "fader", 1024, (void*) led, 10, NULL);
  return led;
}
