#ifndef PIRATE__NTP__H__
#define PIRATE__NTP__H__

#include "pirate/base.h"

esp_err_t ahoy_ntp_init(uint8_t max_retries);

void current_time(struct tm *timeinfo);

#endif
