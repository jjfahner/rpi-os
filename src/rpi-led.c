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
	sys_timer_wait_us(microseconds);
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
		sys_timer_wait_us(microseconds);
		led_off();
		sys_timer_wait_us(microseconds);
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
		sys_timer_wait_us(1000000);
	}
}
