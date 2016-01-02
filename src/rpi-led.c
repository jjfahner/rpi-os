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
#include "rpi-led.h"
#include "rpi-gpio.h"
#include "rpi-systimer.h"



//
// Enable the on-board LED
//
void led_enable()
{
	TRACE("Enabling LED on pin %u", LED_GPFBIT);
	rpi_gpio->LED_GPFSEL |= LED_GPFBIT;
}



//
// Switch the LED on
//
void led_on()
{
#if defined( RPIBPLUS ) || defined( RPI2 )
	rpi_gpio->LED_GPSET = (1 << LED_GPIO_BIT);
#else
	rpi_gpio->LED_GPCLR = (1 << LED_GPIO_BIT);
#endif
}



//
// Switch the LED off
//
void led_off()
{
#if defined( RPIBPLUS ) || defined( RPI2 )
	rpi_gpio->LED_GPCLR = (1 << LED_GPIO_BIT);
#else
	rpi_gpio->LED_GPSET = (1 << LED_GPIO_BIT);
#endif
}



//
// Switch LED on, wait n us, then switch LED off
//
void led_pulse(int microseconds)
{
	led_on();
	sys_timer_wait_usec(microseconds);
	led_off();
}



//
// Blink the led n times, for m us
//
void led_blink(int blink_count, int microseconds)
{
	while (blink_count--)
	{
		led_on();
		sys_timer_wait_usec(microseconds);
		led_off();
		sys_timer_wait_usec(microseconds);
	}
}



//
// Pulse led num times, then wait, used for error reporting
//
void led_error_pulse(int num)
{
	while (1)
	{
		led_blink(num, 500000);
		sys_timer_wait_usec(1000000);
	}
}
