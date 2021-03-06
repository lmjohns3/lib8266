#ifndef PIRATE__WIFI__H__
#define PIRATE__WIFI__H__

#include "pirate/base.h"

esp_err_t ahoy_wifi_init_sta(const char *ssid, const char *pass);

esp_err_t ahoy_wifi_init_ap(const char *ssid, const char *pass);

void ahoy_wifi_wait_until_connected();

#endif
