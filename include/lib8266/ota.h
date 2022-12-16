#ifndef LIB8266__OTA__H__
#define LIB8266__OTA__H__

#include "lib8266/base.h"

void ahoy_ota_poll(uint32_t interval_sec, const char *url, const char *cert);

#endif
