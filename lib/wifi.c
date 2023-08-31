#include <string.h>

#include "esp_event_loop.h"

#include "lib8266/base.h"
#include "lib8266/wifi.h"

#define AHOY_WIFI_CONNECTED_BIT BIT0

static const char *TAG = "⚓ wifi";

static EventGroupHandle_t wifi_event_group;

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data) {
  if (event_base == IP_EVENT) {
    ESP_LOGI(TAG, "STA_GOT_IP");
    xEventGroupSetBits(wifi_event_group, AHOY_WIFI_CONNECTED_BIT);
    return;
  }

  switch (event_id) {
  case WIFI_EVENT_WIFI_READY: {
    ESP_LOGI(TAG, "WIFI_READY");
    break;
  }
  case WIFI_EVENT_SCAN_DONE: {
    wifi_event_sta_scan_done_t *data = (wifi_event_sta_scan_done_t *)event_data;
    ESP_LOGI(TAG, "SCAN_DONE -- %s, %d results", data->status ? "failure" : "success", data->number);
    break;
  }
  case WIFI_EVENT_STA_START: {
    ESP_LOGI(TAG, "STA_START");
    break;
  }
  case WIFI_EVENT_STA_STOP: {
    ESP_LOGI(TAG, "STA_STOP");
    break;
  }
  case WIFI_EVENT_STA_CONNECTED: {
    wifi_event_sta_connected_t *data = (wifi_event_sta_connected_t *)event_data;
    data->ssid[data->ssid_len] = '\0';
    ESP_LOGI(TAG, "STA_CONNECTED -- ssid %s bssid %c%c%c%c%c%c channel %d",
             data->ssid,
             data->bssid[0] || '_',
             data->bssid[1] || '_',
             data->bssid[2] || '_',
             data->bssid[3] || '_',
             data->bssid[4] || '_',
             data->bssid[5] || '_',
             data->channel);
    break;
  }
  case WIFI_EVENT_STA_DISCONNECTED: {
    wifi_event_sta_disconnected_t *data = (wifi_event_sta_disconnected_t *)event_data;
    data->ssid[data->ssid_len] = '\0';
    ESP_LOGI(TAG, "STA_DISCONNECTED -- ssid %s bssid %c%c%c%c%c%c reason %d %s",
             data->ssid,
             data->bssid[0] || '_',
             data->bssid[1] || '_',
             data->bssid[2] || '_',
             data->bssid[3] || '_',
             data->bssid[4] || '_',
             data->bssid[5] || '_',
             data->reason,
             data->reason == 2 ? "AUTH_EXPIRE" :
             data->reason == 4 ? "ASSOC_EXPIRE" :
             data->reason == 8 ? "ASSOC_LEAVE" :
             data->reason == 202 ? "AUTH_FAIL" :
             data->reason == 203 ? "ASSOC_FAIL" :
             data->reason == 205 ? "CONNECTION_FAIL" :
             data->reason == 207 ? "BASIC_RATE_NOT_SUPPORT" :
             "");
    xEventGroupClearBits(wifi_event_group, AHOY_WIFI_CONNECTED_BIT);
    if (data->reason != 8) esp_wifi_connect();
    break;
  }
  case WIFI_EVENT_STA_AUTHMODE_CHANGE: {
    wifi_event_sta_authmode_change_t *data = (wifi_event_sta_authmode_change_t *)event_data;
    ESP_LOGI(TAG, "STA_AUTHMODE_CHANGE -- %d --> %d", data->old_mode, data->new_mode);
    break;
  }
  case WIFI_EVENT_STA_BSS_RSSI_LOW: {
    ESP_LOGI(TAG, "STA_BSS_RSSI_LOW");
    break;
  }
  case WIFI_EVENT_STA_WPS_ER_SUCCESS: {
    ESP_LOGI(TAG, "STA_WPS_ER_SUCCESS");
    break;
  }
  case WIFI_EVENT_STA_WPS_ER_FAILED: {
    ESP_LOGI(TAG, "STA_WPS_ER_FAILED");
    break;
  }
  case WIFI_EVENT_STA_WPS_ER_TIMEOUT: {
    ESP_LOGI(TAG, "STA_WPS_ER_TIMEOUT");
    break;
  }
  case WIFI_EVENT_STA_WPS_ER_PIN: {
    wifi_event_sta_wps_er_pin_t *data = (wifi_event_sta_wps_er_pin_t *)event_data;
    ESP_LOGI(TAG, "STA_WPS_ER_PIN -- %c%c%c%c%c%c%c%c",
             data->pin_code[0] || '_',
             data->pin_code[1] || '_',
             data->pin_code[2] || '_',
             data->pin_code[3] || '_',
             data->pin_code[4] || '_',
             data->pin_code[5] || '_',
             data->pin_code[6] || '_',
             data->pin_code[7] || '_');
    break;
  }
  case WIFI_EVENT_AP_START: {
    ESP_LOGI(TAG, "AP_START");
    break;
  }
  case WIFI_EVENT_AP_STOP: {
    ESP_LOGI(TAG, "AP_STOP");
    break;
  }
  case WIFI_EVENT_AP_STACONNECTED: {
    wifi_event_ap_staconnected_t *data = (wifi_event_ap_staconnected_t *)event_data;
    ESP_LOGI(TAG, "AP_STACONNECTED -- %c%c%c%c%c%c AID %d",
             data->mac[0] || '_',
             data->mac[1] || '_',
             data->mac[2] || '_',
             data->mac[3] || '_',
             data->mac[4] || '_',
             data->mac[5] || '_',
             data->aid);
    break;
  }
  case WIFI_EVENT_AP_STADISCONNECTED: {
    wifi_event_ap_stadisconnected_t *data = (wifi_event_ap_stadisconnected_t *)event_data;
    ESP_LOGI(TAG, "AP_STADISCONNECTED -- %c%c%c%c%c%c AID %d",
             data->mac[0] || '_',
             data->mac[1] || '_',
             data->mac[2] || '_',
             data->mac[3] || '_',
             data->mac[4] || '_',
             data->mac[5] || '_',
             data->aid);
    break;
  }
  case WIFI_EVENT_AP_PROBEREQRECVED: {
    wifi_event_ap_probe_req_rx_t *data = (wifi_event_ap_probe_req_rx_t *)event_data;
    ESP_LOGI(TAG, "AP_PROBEREQRECVED -- %c%c%c%c%c%c RSSI %d",
             data->mac[0] || '_',
             data->mac[1] || '_',
             data->mac[2] || '_',
             data->mac[3] || '_',
             data->mac[4] || '_',
             data->mac[5] || '_',
             data->rssi);
    break;
  }
  default:
    ESP_LOGW(TAG, "unknown event id %d", event_id);
  }
}

esp_err_t common_init() {
  wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_event_handler_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

  return ESP_OK;
}

esp_err_t ahoy_wifi_init_sta(const char *ssid, const char *pass) {
  ESP_ERROR_CHECK(common_init());

  wifi_config_t cfg;
  memset(&cfg, 0, sizeof(wifi_config_t));
  strncpy((char *) cfg.sta.ssid, ssid, 32);
  strncpy((char *) cfg.sta.password, pass, 64);
  if (strlen(pass)) cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg));

  return ESP_OK;
}

esp_err_t ahoy_wifi_init_ap(const char *ssid, const char *pass) {
  ESP_ERROR_CHECK(common_init());

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

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &cfg));

  return ESP_OK;
}

void ahoy_wifi_wait_until_connected() {
  xEventGroupWaitBits(wifi_event_group, AHOY_WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
}

esp_err_t ahoy_wifi_deinit() {
  esp_wifi_deinit();

  vEventGroupDelete(wifi_event_group);

  ESP_ERROR_CHECK(esp_event_handler_unregister(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler));
  ESP_ERROR_CHECK(esp_event_handler_unregister(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler));

  return ESP_OK;
}
