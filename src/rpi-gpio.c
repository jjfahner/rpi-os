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
#include <stdint.h>
#include "rpi-gpio.h"
#include "rpi-systimer.h"



void gpio_set_pin_function( rpi_gpio_pin_t pin, rpi_gpio_alt_function_t func )
{
    rpi_reg_rw_t* fsel_reg = &((rpi_reg_rw_t*)rpi_gpio)[pin / 10 ];
    rpi_reg_rw_t fsel_copy = *fsel_reg;
    fsel_copy &= ~( FS_MASK << ( (pin % 10 ) * 3 ) );
    fsel_copy |= (func << ( (pin % 10 ) * 3 ) );
    *fsel_reg = fsel_copy;
}



void gpio_set_input(rpi_gpio_pin_t pin)
{
	gpio_set_pin_function(pin, FS_INPUT);
}




void gpio_set_output( rpi_gpio_pin_t pin )
{
	gpio_set_pin_function(pin, FS_OUTPUT);
}




rpi_gpio_value_t gpio_get_pin_value( rpi_gpio_pin_t pin )
{
    rpi_gpio_value_t result = RPI_IO_UNKNOWN;

    switch( pin / 32 )
    {
        case 0:
            result = rpi_gpio->GPLEV0 & ( 1 << pin );
            break;

        case 1:
            result = rpi_gpio->GPLEV1 & ( 1 << ( pin - 32 ) );
            break;

        default:
            break;
    }

    if( result != RPI_IO_UNKNOWN )
    {
        if( result )
            result = RPI_IO_HI;
    }

    return result;
}



void gpio_toggle_pin( rpi_gpio_pin_t pin )
{
    if( gpio_get_pin_value( pin ) )
        gpio_set_pin_lo( pin );
    else
        gpio_set_pin_hi( pin );
}



void gpio_set_pin_hi( rpi_gpio_pin_t pin )
{
    switch( pin / 32 )
    {
        case 0:
            rpi_gpio->GPSET0 = ( 1 << pin );
            break;

        case 1:
            rpi_gpio->GPSET1 = ( 1 << ( pin - 32 ) );
            break;

        default:
            break;
    }
}


void gpio_set_pin_lo( rpi_gpio_pin_t pin )
{
    switch( pin / 32 )
    {
        case 0:
            rpi_gpio->GPCLR0 = ( 1 << pin );
            break;

        case 1:
            rpi_gpio->GPCLR1 = ( 1 << ( pin - 32 ) );
            break;

        default:
            break;
    }
}


void gpio_set_pin_value( rpi_gpio_pin_t pin, rpi_gpio_value_t value )
{
    if( ( value == RPI_IO_LO ) || ( value == RPI_IO_OFF ) )
        gpio_set_pin_lo( pin );
    else if( ( value == RPI_IO_HI ) || ( value == RPI_IO_ON ) )
        gpio_set_pin_hi( pin );
}



