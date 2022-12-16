#include "lwip/apps/sntp.h"

#include "lib8266/ntp.h"
#include "lib8266/wifi.h"

static const char *TAG = "ntp";

esp_err_t ahoy_ntp_init(uint8_t max_retries) {
  ahoy_wifi_wait_until_connected();
  ESP_LOGI(TAG, "Initializing SNTP");
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.org");
  sntp_init();
  time_t now = 0;
  struct tm timeinfo = {0};
  int retry = 0;
  while (timeinfo.tm_year < 100 && ++retry < max_retries) {
    ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, max_retries);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    time(&now);
    localtime_r(&now, &timeinfo);
  }
  return retry == max_retries ? ESP_ERR_TIMEOUT : ESP_OK;
}

void ahoy_now(struct tm *timeinfo) {
  time_t now = 0;
  time(&now);
  localtime_r(&now, timeinfo);
}
