#ifndef LIB8266__HTTP__H__
#define LIB8266__HTTP__H__

#include "esp_http_client.h"

#include "lib8266/base.h"

typedef void (* ahoy_http_poll_cb)(esp_http_client_config_t *, esp_http_client_handle_t);

void ahoy_http_poll(uint32_t period_ms,
                    const char *url,
                    const char *cert,
                    ahoy_http_poll_cb callback);

#endif
