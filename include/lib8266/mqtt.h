#ifndef LIB8266__MQTT__H__
#define LIB8266__MQTT__H__

#include "lib8266/base.h"

#include "mqtt_client.h"

#define AHOY_MQTT_JSON_LENGTH 200
#define AHOY_MQTT_TOPIC_LENGTH 50

typedef struct ahoy_mqtt_t* ahoy_mqtt_handle_t;

ahoy_mqtt_handle_t ahoy_mqtt_init(const char *server, uint16_t queue_size);

esp_err_t ahoy_mqtt_periodic_heartbeat(ahoy_mqtt_handle_t mqtt,
                                       uint32_t period_s,
                                       uint32_t priority,
                                       const char *topic);

esp_err_t ahoy_mqtt_enqueue(ahoy_mqtt_handle_t mqtt, const char *topic, const char *json);

esp_err_t ahoy_mqtt_publish_queued(ahoy_mqtt_handle_t mqtt);

esp_err_t ahoy_mqtt_periodic_publish(ahoy_mqtt_handle_t mqtt,
                                     uint32_t period_s,
                                     uint32_t timeout_s,
                                     uint32_t priority);

#endif
