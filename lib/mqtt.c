#include "lib8266/mqtt.h"
#include "lib8266/wifi.h"

static const char *TAG = "⚓ mqtt";

struct ahoy_mqtt_t {
  QueueHandle_t queue;
  esp_mqtt_client_config_t config;
};

typedef struct {
  char topic[AHOY_MQTT_TOPIC_LENGTH];
  char json[AHOY_MQTT_JSON_LENGTH];
  int64_t created_us;
} ahoy_mqtt_message_t;

typedef struct {
  uint32_t period_s;
  uint32_t period_jitter_s;
  char topic[AHOY_MQTT_TOPIC_LENGTH];
  ahoy_mqtt_handle_t mqtt;
} heartbeat_params_t;

typedef struct {
  uint32_t period_s;
  uint32_t wifi_timeout_s;
  ahoy_mqtt_handle_t mqtt;
} publish_params_t;


ahoy_mqtt_handle_t ahoy_mqtt_init(const char *server, uint16_t queue_size) {
  struct ahoy_mqtt_t *mqtt = malloc(sizeof(struct ahoy_mqtt_t));
  memset(mqtt, 0, sizeof(struct ahoy_mqtt_t));
  mqtt->config.uri = server;
  mqtt->queue = xQueueCreate(queue_size, sizeof(ahoy_mqtt_message_t *));
  return mqtt;
}


esp_err_t ahoy_mqtt_enqueue(ahoy_mqtt_handle_t mqtt,
                            const char *topic,
                            const char *json) {
  ahoy_mqtt_message_t *msg = malloc(sizeof(ahoy_mqtt_message_t));
  if (msg == NULL) return ESP_FAIL;
  memset(msg, 0, sizeof(ahoy_mqtt_message_t));
  msg->created_us = esp_timer_get_time();  /* record time of message enqueue. */
  memcpy(msg->topic, topic, AHOY_MQTT_TOPIC_LENGTH);
  memcpy(msg->json, json, AHOY_MQTT_JSON_LENGTH);
  xQueueSend(mqtt->queue, &msg, 0);
  return ESP_OK;
}


static void heartbeat(void *params) {
  heartbeat_params_t *p = (heartbeat_params_t *)params;
  char json[AHOY_MQTT_JSON_LENGTH];
  TickType_t last_wake_time = xTaskGetTickCount();
  for (;;) {
    const uint32_t jitter_ms = esp_random() % (1000 * p->period_jitter_s);
    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(1000 * p->period_s + jitter_ms));
    snprintf(json, AHOY_MQTT_JSON_LENGTH,
             "{\"heap\":%u,\"queued\":%u}",
             esp_get_free_heap_size(),
             uxQueueMessagesWaiting(p->mqtt->queue));
    ESP_LOGI(TAG, "🫀 %s", json);
    ESP_ERROR_CHECK(ahoy_mqtt_enqueue(p->mqtt, p->topic, json));
  }
}


esp_err_t ahoy_mqtt_periodic_heartbeat(ahoy_mqtt_handle_t mqtt,
                                       uint32_t period_s,
                                       uint32_t period_jitter_s,
                                       uint32_t priority,
                                       const char *topic) {
  heartbeat_params_t *p = malloc(sizeof(heartbeat_params_t));
  if (p == NULL) return ESP_FAIL;
  p->period_s = period_s;
  p->period_jitter_s = period_jitter_s;
  p->mqtt = mqtt;
  memcpy(p->topic, topic, AHOY_MQTT_TOPIC_LENGTH);
  xTaskCreate(heartbeat, "heartbeat", 2000, (void *)p, priority, NULL);
  return ESP_OK;
}


esp_err_t ahoy_mqtt_publish_queued(ahoy_mqtt_handle_t mqtt) {
  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&(mqtt->config));

  esp_err_t ret = esp_mqtt_client_start(client);
  if (ret != ESP_OK) return ret;

  const uint16_t size = AHOY_MQTT_JSON_LENGTH + 30;
  char buf[size];
  ahoy_mqtt_message_t *msg;
  while (xQueueReceive(mqtt->queue, &msg, 0) == pdTRUE) {
    memset(buf, 0, size);
    snprintf(buf, size, "{\"age_ms\":%u,\"json\":%s}",
             (uint32_t)(esp_timer_get_time() - msg->created_us) / 1000, msg->json);
    int res = esp_mqtt_client_publish(client, msg->topic, buf, 0, 1, 0);
    free(msg);
    if (res == -1) break;
  }

  ESP_LOGI(TAG, "queue has %d messages after publishing", uxQueueMessagesWaiting(mqtt->queue));
  return esp_mqtt_client_stop(client);
}


static void publish(void *params) {
  publish_params_t *p = (publish_params_t *)params;
  TickType_t last_wake_time = xTaskGetTickCount();
  for (;;) {
    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(1000 * p->period_s));
    esp_wifi_start();
    esp_wifi_connect();
    if (ahoy_wifi_wait_until_connected(1000 * p->wifi_timeout_s))
      ahoy_mqtt_publish_queued(p->mqtt);
    esp_wifi_disconnect();
    esp_wifi_stop();
  }
}


esp_err_t ahoy_mqtt_periodic_publish(ahoy_mqtt_handle_t mqtt,
                                     uint32_t period_s,
                                     uint32_t wifi_timeout_s,
                                     uint32_t priority) {
  publish_params_t *p = malloc(sizeof(publish_params_t));
  if (p == NULL) return ESP_FAIL;
  p->period_s = period_s;
  p->wifi_timeout_s = wifi_timeout_s;
  p->mqtt = mqtt;
  xTaskCreate(publish, "publish", 2000, (void *)p, priority, NULL);
  return ESP_OK;
}
