#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/sfr_defs.h>
#define F_CPU 16000000UL
#define BAUD 19200
#include <util/setbaud.h>
#include <util/delay.h>

#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
// output on USB = PD1 = board pin 1
// datasheet p.190; F_OSC = 16 MHz & baud rate = 19.200
#define UBBRVAL 51

//red: 5v
//gray: gnd
//yellow: A1


void uart_init()
{
	// set the baud rate
	UBRR0H = UBRRH_VALUE; //UBRR0H = Baud Rate registers for HIGH (msb)
	UBRR0L = UBRRL_VALUE;// Baud Rate register for LOW (lsb) set to UBBRVAL = 51
	// disable U2X mode
	//UCSR0A = 0; //contains status data
	// set frame format : asynchronous, 8 data bits, 1 stop bit, no parity
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
	// enable transmitter by setting the UCsZ02 bit
	UCSR0B = _BV(RXEN0) | _BV(TXEN0); //TXEN0

	
	
}

static int uart_sendchar(char letter, FILE *stream);
static int uart_readchar(FILE *stream);

FILE uart_output = FDEV_SETUP_STREAM(uart_sendchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_readchar, _FDEV_SETUP_READ);

FILE uart_io = FDEV_SETUP_STREAM(uart_sendchar, uart_readchar, _FDEV_SETUP_RW);

int uart_sendchar(char letter, FILE *stream) {
	if (letter == '\n') {
		uart_sendchar('\r', stream);
	}
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = letter;
	return 0;
}

int uart_readchar(FILE *stream) {
	loop_until_bit_is_set(UCSR0A, RXC0); // Wait until data exists. 
	return UDR0;
}




void init_analogRead()
{


	// set the analog reference (high two bits of ADMUX) and select the
	// channel (low 4 bits).  this also sets ADLAR (left-adjust result)
	// to 0 (the default).
//OPZOEKEN ADMUX!!! 
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0)| (1 << ADEN); // 128 prescale for 16Mhz & enable adc
}
uint16_t analogRead(uint8_t pin)
{
	
	_delay_ms(10);

	uint8_t low, high;
	if (pin >= 14) pin -= 14; // allow for channel or pin numbersuint8_t low, high;
	
	ADMUX = (1 << REFS0) | (pin & 0x07);
	// start the conversion
	//sbi(ADCSRA, ADSC);
	ADCSRA |= (1<<ADSC);
	
	// ADSC is cleared when the conversion finishes
	while (bit_is_set(ADCSRA, ADSC));

	// we have to read ADCL first; doing so locks both ADCL
	// and ADCH until ADCH is read.  reading ADCL second would
	// cause the results of each conversion to be discarded,
	// as ADCL and ADCH would be locked when it completed.
	// combine the two bytes
	low  = ADCL;
	high = ADCH;
	
	//return (high << 8) | low;
	return ADC;
}

int main(void)
{
	uart_init();
	stdout = &uart_output;
	stdin  = &uart_input;
	
	init_analogRead();
	
	_delay_ms(1000);
	while (1) {
		
		int input = analogRead(0);
		float voltage = input * 5.0;
		voltage /= 1024;
		
		float temperature;
		temperature = (voltage - 0.5) * 100 ;
		int inputLigth = analogRead(1);
		float voltageLight = inputLigth * 5.0;
		voltageLight /= 1024;
		float rldr = (10*voltageLight)/(5-voltageLight);
		float lux = 500/rldr;
		
		printf("L %.1f\n", lux);
		printf("T %.1f\n", temperature);
		//printf("voltagePoort1 %.1f\n", voltageLight);
	
	
		_delay_ms(3000);
	}
}