#ifndef PIRATE__OTA__H__
#define PIRATE__OTA__H__

#include "pirate/base.h"

void ahoy_ota_poll(uint32_t interval_sec, const char *url, const char *cert);

#endif
