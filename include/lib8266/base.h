#ifndef LIB8266__BASE__H__
#define LIB8266__BASE__H__

#include "esp_log.h"
#include "esp_system.h"
#include "FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#define AHOY_MAX(a, b) ((a) > (b) ? (a) : (b))
#define AHOY_MIN(a, b) ((a) < (b) ? (a) : (b))
#define AHOY_ABS(x) ((x) < 0 ? (0-(x)) : (x))

#define AHOY_RETURN_IF_NOT_OK(x) ({                                     \
      esp_err_t __err__ = (x);                                          \
      if (__err__ != ESP_OK) {                                          \
        _esp_error_check_failed_without_abort(                          \
            __err__, __ESP_FILE__, __LINE__, __ASSERT_FUNC, #x);        \
        return __err__;                                                 \
      }                                                                 \
      __err__;                                                          \
    })

#define AHOY_RETURN_IF_NOT_PASS(x) ({                                   \
      BaseType_t __out__ = (x);                                         \
      if (__out__ != pdPASS) {                                          \
        _esp_error_check_failed_without_abort(                          \
            __out__, __ESP_FILE__, __LINE__, __ASSERT_FUNC, #x);        \
        return __out__;                                                 \
      }                                                                 \
      __out__;                                                          \
    })

#endif
