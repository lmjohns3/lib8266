#ifndef LIB8266__HTTPD__H__
#define LIB8266__HTTPD__H__

#include "esp_http_server.h"

#include "lib8266/base.h"

esp_err_t ahoy_httpd_init(httpd_handle_t *server);

void ahoy_httpd_handle(const httpd_handle_t server, const httpd_uri_t *handler);

#endif
