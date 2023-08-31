#ifndef LIB8266__ONEWIRE__H__
#define LIB8266__ONEWIRE__H__

#include "lib8266/io.h"

bool ahoy_onewire_reset(uint8_t pin);

bool ahoy_onewire_read_bit(uint8_t pin);

uint8_t ahoy_onewire_read_byte(uint8_t pin);

void ahoy_onewire_write_bit(uint8_t pin, bool bit);

void ahoy_onewire_write_byte(uint8_t pin, uint8_t bits);

uint8_t ahoy_onewire_crc_8bytes(uint8_t *bytes);

#endif
