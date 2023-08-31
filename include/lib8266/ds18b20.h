#ifndef LIB8266__DS18B20__H__
#define LIB8266__DS18B20__H__

#include "lib8266/fixed.h"
#include "lib8266/onewire.h"

ahoy_fixed_t ahoy_ds18b20_read_degc(uint8_t pin);

#endif
