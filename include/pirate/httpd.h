#ifndef PIRATE__HTTPD__H__
#define PIRATE__HTTPD__H__

#include "esp_http_server.h"

#include "pirate/base.h"

esp_err_t ahoy_httpd_init(httpd_handle_t *server);

void ahoy_httpd_handle(const httpd_handle_t server, const httpd_uri_t *handler);

#endif
