/* WLAN Radio
 * Display Ansteuerung
 * + Empfang von Sendernamen per UART
 * + Verändern der Radiostation durch Tastendruck, übertragen per UART

 * Zusammenstellung von Friedemann Wulff-Woesten, www.wulff-woesten.de
 * die meisten Codefragmente stammen von www.mikrocontroller.net
 * - Danke an alle Jungs dort!

 * Display Ansteuerung getestet mit
 * + DISPLAYTECH 162A
 *   bestellt bei Reichelt
 * + LCD-Modul TC1602A-09 (sehr trendiges blau!)
 *   bestellt bei Pollin

 * verwendeter uController: Atmel ATMega8 - reicht vollkommen aus.
 * Compiler: AVR-GCC
 * erfolgreich getestet auf
 * Windows (WinAVR), Mac OS X (Crosspack), Ubuntu Linux 11.10 und 11.04
 */

// Hier die verwendete Taktfrequenz in Hz eintragen, wichtig!
// edit clock setting here! otherwise timing will be spoiled.
#ifndef F_CPU
#define F_CPU 3686400
#endif

// clock speed
#define FOSC F_CPU
// 115200 is default baud rate of OpenWRT - so it should work.
// Please use an external crystal oscillator!
// 3,6864 MHz works fine for me.
#define BAUD 115200
#define MYUBRR FOSC/16/BAUD-1
// longest character line to accept from serial port
#define SER_BUFF_LEN 80

#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/io.h>
#include <stdlib.h>
#include "lcd-routines.h"
#include "taster.h"
#include <string.h>
#include <stdio.h>

// serial buffer
unsigned char serbuffer[SER_BUFF_LEN];
// current tuner position
unsigned char tunerpos = 0;
// string with max. 16+1 chars
unsigned char Line[17];
unsigned char OldLine[17];
unsigned char flag = 0;

// UART stuff
void uart_init( unsigned int ubrr)
{
	// set baud rate
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
	// enable receiver and transmitter
	UCSRB = (1<<RXEN)|(1<<TXEN);
	// set frame format: 8data, 1stop bit
	UCSRC = (1<<URSEL)|(3<<UCSZ0);
	UCSRC &= ~(1<<USBS);
}

int uart_putc(unsigned char c)
{
	// wait until UART ready
	while (!(UCSRA & (1<<UDRE)))
	{
	}
	// send char
	UDR = c;
	return 0;
}

void uart_puts(char *s)
{
	while (*s)
	{
		// loop as long *s != '\0'
		uart_putc(*s);
		s++;
	}
}

int uart_getc(void)
{
	// wait until char is there
	while (!(UCSRA & (1<<RXC))) ;
	// return char to caller
	return UDR;
}

void uart_gets(char* Buffer, uint8_t MaxLen)
{
	uint8_t NextChar;
	uint8_t StringLen = 0;
	/* wait for and receive new char
	   collect chars, until
	   - \n came by
	   - array is full */
	NextChar = uart_getc();
	while ((NextChar != '\n') && (StringLen < MaxLen))
	{
		// do not collect \r char
		if (NextChar != '\r')
		{
			*Buffer++ = NextChar;
		}
		StringLen++;
		NextChar = uart_getc();
	}
	// append '\0' to produce proper c string
	*Buffer = '\0';
}


// push button stuff
/* 3,6864 MHz clock -> Timer0 produces overflow interrupt every 256µs
   we need get_taster() to be executed every 10ms
   so we call it every 144th time */

ISR(TIMER0_OVF_vect)
{
	static unsigned char count_ovl0;
	unsigned char ovl0 = count_ovl0+1;
	if (ovl0 >= 144) // if your clock differs from 3,6864 MHz, recalculate this value!
	{
		get_taster (0, PIND & (1<<PD2));
		get_taster (1, PIND & (1<<PD3));
		ovl0 = 0;
	}
	count_ovl0 = ovl0;
}

void ioinit()
{
  #if TASTER_LEVEL
	;
  #else
	PORTD |= 1 << PD2;
	PORTD |= 1 << PD3;
  #endif
	// Timer0 without prescaler
	TCCR0 = 1 << CS00;
	// activate Timer0 overflow interrupt
	TIMSK |= (1 << TOIE0);
}

int main(void)
{

	// initialize USART
	uart_init(MYUBRR);
	DDRB = 0xFF;
	DDRD = 0x00;
	// initialize buttons
	DDRD &= ~(1 << PD2);
	DDRD &= ~(1 << PD3);
	ioinit();

	// splash screen rendering
	lcd_init();
	lcd_string("** Wifi Radio **");
	lcd_setcursor( 0, 2 );
	lcd_string("wulff-woesten.de");
	_delay_ms(2000);
	lcd_clear();
	lcd_string("Idee:");
	lcd_setcursor( 0, 2 );
	lcd_string("Sebastian Paul");
	_delay_ms(2000);
	lcd_clear();
	lcd_string("Umsetzung:");
	lcd_setcursor( 0, 2 );
	lcd_string("Friedemann W.-W.");
	_delay_ms(2000);
	lcd_clear();
	lcd_string("Special thanks:");
	lcd_setcursor( 0, 2 );
	lcd_string("Karyna Krynko");
	_delay_ms(2000);
	lcd_clear();
	lcd_string(">>> Verbinde >>>");
	lcd_setcursor( 0, 2 );
	lcd_string("> Bitte warten >");
	do uart_gets( Line, sizeof( Line ) );
	while (!strstr(Line, "AVR Start"));
	lcd_clear();

	// set button mode
	tasten[0].mode = TM_LONG;
	tasten[1].mode = TM_SHORT;
	// enable interrupts
	sei();
	while(1)
	{
		signed char tast = taster;
		switch (tast)
		{
		default:
		case NO_TASTER:
			break;
		case 0:
			// if the button1 is pressed, send new position over serial port
			tunerpos++;
			sprintf(serbuffer, "tuner: %d\n", tunerpos);
			// transmit over serial link
			uart_puts(serbuffer);
			break;
		case 0+TASTER_LONG:
			// if button1 was pressed for a longer time, jump 2 stations forward
			tunerpos = tunerpos + 2;
			sprintf(serbuffer, "tuner: %d\n", tunerpos);
			// transmit over serial link
			uart_puts(serbuffer);
			break;
		case 1:
			// if button2 was pressed go back to first station in playlist
			tunerpos = 1;
			sprintf(serbuffer, "tuner: %d\n", tunerpos);
			// transmit over serial link
			uart_puts(serbuffer);
			break;
		}
		if (tast != NO_TASTER)
			taster = NO_TASTER;
		uart_gets(Line, sizeof(Line));
		if (strcmp(Line, OldLine))
		{
			lcd_clear();
			lcd_home();
			lcd_string("** Wifi Radio **");
			lcd_setcursor( 0, 2 );
			lcd_string(Line);
			strcpy(OldLine, Line);
		}
	}
	return 0;
}