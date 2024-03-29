#include "lib8266/httpd.h"
#include "lib8266/wifi.h"

static const char *TAG = "⚓ httpd";

esp_err_t ahoy_httpd_init(httpd_handle_t *server) {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  ahoy_wifi_wait_until_connected(1 << 31);
  ESP_LOGI(TAG, "Starting HTTP server on port %d", config.server_port);
  return httpd_start(server, &config);
}

void ahoy_httpd_handle(const httpd_handle_t server, const httpd_uri_t *handler) {
  ESP_LOGI(TAG, "Registering HTTP URI handler");
  httpd_register_uri_handler(server, handler);
}
