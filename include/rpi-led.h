#pragma once

#include "rpi-base.h"

//
// Led functions
//
EXTERN_C void led_enable();
EXTERN_C void led_on();
EXTERN_C void led_off();
EXTERN_C void led_pulse(int microseconds);
EXTERN_C void led_blink(int blink_count, int microseconds);
EXTERN_C void led_error_pulse(int num);
