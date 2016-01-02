/*
	Licensed to the Apache Software Foundation (ASF) under one
	or more contributor license agreements.  See the NOTICE file
	distributed with this work for additional information
	regarding copyright ownership.  The ASF licenses this file
	to you under the Apache License, Version 2.0 (the
	"License"); you may not use this file except in compliance
	with the License.  You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing,
	software distributed under the License is distributed on an
	"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
	KIND, either express or implied.  See the License for the
	specific language governing permissions and limitations
	under the License.
*/
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
