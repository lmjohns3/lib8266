#include <sys/time.h>
#include <time.h>

#include "lib8266/http.h"
#include "lib8266/time.h"

#define ISO_DATETIME_LENGTH 20
#define D(i, pow) ((iso[(i)] - '0') * (pow))

static const char* TAG = "⚓ time";

static void update(esp_http_client_config_t *http, esp_http_client_handle_t client) {
  char iso[ISO_DATETIME_LENGTH];
  if (esp_http_client_read(client, iso, ISO_DATETIME_LENGTH) == ISO_DATETIME_LENGTH) {
    struct tm tm = {
        .tm_year = D(0, 1000) + D(1, 100) + D(2, 10) + D(3, 1) - 1900,
        .tm_mon = D(5, 10) + D(6, 1),
        .tm_mday = D(8, 10) + D(9, 1),
        .tm_hour = D(11, 10) + D(12, 1),
        .tm_min = D(14, 10) + D(15, 1),
        .tm_sec = D(17, 10) + D(18, 1),
    };
    ESP_LOGI(TAG, "setting time: %s", asctime(&tm));
    //struct timeval now = { .tv_sec = mktime(&tm) };
    //settimeofday(&now, NULL);
  }
}

void ahoy_time_poll(uint32_t period_ms, const char *url) {
  ahoy_http_poll(period_ms, url, "", update);
}
