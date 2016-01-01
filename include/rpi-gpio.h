#pragma once

#include "rpi-base.h"


//
// Pin function
//
EXTERN_C void gpio_set_pin_function( rpi_gpio_pin_t pin, rpi_gpio_alt_function_t func );
EXTERN_C void gpio_set_input( rpi_gpio_pin_t pin );
EXTERN_C void gpio_set_output( rpi_gpio_pin_t pin );



//
// Pin value
//
EXTERN_C rpi_gpio_value_t gpio_get_pin_value( rpi_gpio_pin_t pin );
EXTERN_C void gpio_set_pin_value(rpi_gpio_pin_t pin, rpi_gpio_value_t value);
EXTERN_C void gpio_toggle_pin(rpi_gpio_pin_t pin);
EXTERN_C void gpio_set_pin_hi( rpi_gpio_pin_t pin );
EXTERN_C void gpio_set_pin_lo( rpi_gpio_pin_t pin );
