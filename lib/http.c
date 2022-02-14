#include "pirate/http.h"
#include "pirate/wifi.h"

static const char *TAG = "http";

static esp_err_t logging_event_handler(esp_http_client_event_t *event) {
  switch(event->event_id) {
  case HTTP_EVENT_ERROR:
    ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
    break;
  case HTTP_EVENT_ON_CONNECTED:
    ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
    break;
  case HTTP_EVENT_HEADER_SENT:
    ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
    break;
  case HTTP_EVENT_ON_HEADER:
    ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s",
             event->header_key, event->header_value);
    break;
  case HTTP_EVENT_ON_DATA:
    ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", event->data_len);
    break;
  case HTTP_EVENT_ON_FINISH:
    ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
    break;
  case HTTP_EVENT_DISCONNECTED:
    ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
    break;
  }
  return ESP_OK;
}

typedef struct {
  TickType_t period;
  ahoy_http_poll_cb callback;
  esp_http_client_config_t *http;
} params;

static void poll(void *arg) {
  params *p = arg;
  TickType_t last_wake_time = xTaskGetTickCount();
  for (;;) {
    ahoy_wifi_wait_until_connected();
    esp_http_client_handle_t client = esp_http_client_init(p->http);
    ESP_LOGI("http-poll", "Checking for update from %s...", p->http->url);
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
      ESP_LOGI("http-poll", "GET status %d length %d",
               esp_http_client_get_status_code(client),
               esp_http_client_get_content_length(client));
      p->callback(p->http, client);
    } else {
      ESP_LOGE("http-poll", "GET request failed: %d", err);
    }
    esp_http_client_cleanup(client);
    vTaskDelayUntil(&last_wake_time, p->period);
  }
  free(p->http);
  free(p);
  vTaskDelete(NULL);
}

void ahoy_http_poll(uint32_t period_ms,
                    const char *url,
                    const char *cert,
                    ahoy_http_poll_cb callback) {
  params *p = malloc(sizeof(params));
  p->period = period_ms / portTICK_PERIOD_MS;
  p->callback = callback;
  p->http = (esp_http_client_config_t *) malloc(sizeof(esp_http_client_config_t));
  memset(p->http, 0, sizeof(esp_http_client_config_t));
  p->http->url = calloc(strlen(url), 1);
  strncpy((char*) p->http->url, url, strlen(url));
  p->http->cert_pem = calloc(strlen(cert), 1);
  strncpy((char*) p->http->cert_pem, cert, strlen(cert));
  p->http->event_handler = logging_event_handler;
  xTaskCreate(poll, "http-poll", 8192, (void *) p, 0, NULL);
}
