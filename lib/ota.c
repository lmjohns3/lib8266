#include "esp_https_ota.h"

#include "lib8266/http.h"
#include "lib8266/ota.h"
#include "lib8266/wifi.h"

static void update(esp_http_client_config_t *http, esp_http_client_handle_t client) {
  if (esp_https_ota(http) == ESP_OK) esp_restart();
}

void ahoy_ota_poll(uint32_t period_ms, const char *url, const char *cert) {
  ahoy_http_poll(period_ms, url, cert, update);
}
