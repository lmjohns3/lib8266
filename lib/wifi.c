#include <string.h>

#include "esp_event_loop.h"
#include "esp_wifi.h"

#include "pirate/base.h"
#include "pirate/wifi.h"

#define AHOY_WIFI_CONNECTED_BIT BIT0

static const char *TAG = "wifi";

static EventGroupHandle_t connection_event_group;

static esp_err_t event_handler(void *ctx, system_event_t *event) {
  system_event_info_t *info = &event->event_info;
  switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
      esp_wifi_connect();
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      ESP_LOGI("wifi", "got ip %s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
      xEventGroupSetBits(connection_event_group, AHOY_BIT_WIFI_CONNECTED);
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      ESP_LOGE("wifi", "disconnect: %d", info->disconnected.reason);
      if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
          esp_wifi_set_protocol(ESP_IF_WIFI_STA,
                                WIFI_PROTOCAL_11B |
                                WIFI_PROTOCAL_11G |
                                WIFI_PROTOCAL_11N);
      }
      esp_wifi_connect();
      xEventGroupClearBits(connection_event_group, AHOY_BIT_WIFI_CONNECTED);
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        ESP_LOGI("wifi", "station:" MACSTR " join, AID=%d",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGI("wifi", "station:" MACSTR " leave, AID=%d",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);
        break;
    default:
      break;
  }

  return ESP_OK;
}

esp_err_t pre_init() {
  tcpip_adapter_init();
  connection_event_group = xEventGroupCreate();
  AHOY_RETURN_IF_ERROR(esp_event_loop_init(event_handler, NULL));
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  AHOY_RETURN_IF_ERROR(esp_wifi_init(&cfg));
  AHOY_RETURN_IF_ERROR(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  return ESP_OK;
}

esp_err_t ahoy_wifi_init_sta(const char *ssid, const char *pass) {
  AHOY_RETURN_IF_ERROR(pre_init());
  wifi_config_t cfg;
  memset(&cfg, 0, sizeof(wifi_config_t));
  strncpy((char *) cfg.sta.ssid, ssid, 32);
  strncpy((char *) cfg.sta.password, pass, 64);
  ESP_LOGI(TAG, "Connecting to WiFi SSID %s...", cfg.sta.ssid);
  AHOY_RETURN_IF_ERROR(esp_wifi_set_mode(WIFI_MODE_STA));
  AHOY_RETURN_IF_ERROR(esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg));
  AHOY_RETURN_IF_ERROR(esp_wifi_start());
  return ESP_OK;
}

esp_err_t ahoy_wifi_init_ap(const char *ssid, const char *pass) {
  AHOY_RETURN_IF_ERROR(pre_init());
  wifi_config_t cfg;
  strncpy((char *) cfg.ap.ssid, ssid, 32);
  cfg.ap.ssid_len = strlen(ssid);
  cfg.ap.max_connection = 2;
  if (strlen(pass) == 0) {
    strncpy((char *) cfg.ap.password, "", 64);
    cfg.ap.authmode = WIFI_AUTH_OPEN;
  } else {
    strncpy((char *) cfg.ap.password, pass, 64);
    cfg.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
  }
  ESP_LOGI(TAG, "Starting WiFi AP with SSID %s...", cfg.ap.ssid);
  AHOY_RETURN_IF_ERROR(esp_wifi_set_mode(WIFI_MODE_AP));
  AHOY_RETURN_IF_ERROR(esp_wifi_set_config(ESP_IF_WIFI_AP, &cfg));
  AHOY_RETURN_IF_ERROR(esp_wifi_start());
  return ESP_OK;
}

void ahoy_wifi_wait_until_connected() {
  xEventGroupWaitBits(connection_event_group, AHOY_BIT_WIFI_CONNECTED, false, true, portMAX_DELAY);
}
