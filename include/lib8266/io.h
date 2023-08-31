#ifndef LIB8266__IO__H__
#define LIB8266__IO__H__

#include "lib8266/base.h"

#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp8266/gpio_struct.h"

esp_err_t ahoy_io_init_adc(uint8_t clock_div);
esp_err_t ahoy_io_init_inputs(uint32_t pin_bit_mask, bool enable_pullup);
esp_err_t ahoy_io_init_outputs(uint32_t pin_bit_mask);

// https://github.com/wdim0/esp8266_direct_gpio
// https://www.reddit.com/r/esp32/comments/f529hf

#define AHOY_GPIO_ENABLE_OUTPUT(pin) (GPIO.enable_w1ts |= BIT(pin))
#define AHOY_GPIO_DISABLE_OUTPUT(pin) (GPIO.enable_w1tc |= BIT(pin))

#define AHOY_GPIO_ENABLE_PULLUP gpio_pullup_en
#define AHOY_GPIO_DISABLE_PULLUP gpio_pullup_dis

#define AHOY_GPIO_READ(pin) ((GPIO.in >> pin) & 1)

#define AHOY_GPIO_WRITE(pin, value) ((value) ? AHOY_GPIO_WRITE_H(pin) : AHOY_GPIO_WRITE_L(pin))
#define AHOY_GPIO_WRITE_H(pin) (GPIO.out_w1ts |= BIT(pin))
#define AHOY_GPIO_WRITE_L(pin) (GPIO.out_w1tc |= BIT(pin))

#endif
