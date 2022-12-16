#ifndef LIB8266__NTP__H__
#define LIB8266__NTP__H__

#include "lib8266/base.h"

esp_err_t ahoy_ntp_init(uint8_t max_retries);

void ahoy_now(struct tm *timeinfo);

#endif
