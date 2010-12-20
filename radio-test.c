/* Clockfort, 2010-12-20.
 * Code to test cheap wireless radios.
 */

/* Simple example for Teensy USB Development Board
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2008 PJRC.COM, LLC
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <util/delay.h>
#include "usb_serial.h"
#include "uart.h"

#define LED_CONFIG	(DDRD |= (1<<6))
#define LED_ON		(PORTD &= ~(1<<6))
#define LED_OFF		(PORTD |= (1<<6))
#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
#define BAUD_RATE	9600

volatile uint8_t proc_timer;

ISR(TIMER0_OVF_vect){
                ++proc_timer;
		while(uart_available()){
			usb_serial_putchar_nowait(uart_getchar());
			LED_ON;
			proc_timer = 0;
		}
                if(proc_timer>30){
                        LED_OFF; //Keep the LED on for a human-perceivable amount of time when UART is active
                }
}

int main(void)
{
	uint16_t c;

	// set for 16 MHz clock, set LED pin mode
	CPU_PRESCALE(0);
	LED_CONFIG;

	//Setup USB
	usb_init();
	while (!usb_configured()) /* wait */ ;
	_delay_ms(1000);


	//Setup interrupts
        //Set CPU clock divider to ~61Hz
        TCCR0B |= _BV(CS02) | _BV(CS00);

        //Interrupt on timer0 overflow
        TIMSK0 |= _BV(TOIE0);

        proc_timer=0;
        TCNT0 = 0;

        //Enable interrupts
        sei();

	//setup UART
	uart_init(BAUD_RATE);

	while (1) {
		// wait for the user to run their terminal emulator program
		// which sets DTR to indicate it is ready to receive.
		while (!(usb_serial_get_control() & USB_SERIAL_DTR)) /* wait */ ;

		// discard anything that was received prior.  Sometimes the
		// operating system or other software will send a modem
		// "AT command", which can still be buffered.
		usb_serial_flush_input();

		// Do pseudo-interactive I/O with the USB serial port/uart.
		while (1) {
			c = usb_serial_getchar();
			uart_putchar(c);
		}
	}
}
