#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/sfr_defs.h>
#define F_CPU 16E6
#include <util/delay.h>

#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
// output on USB = PD1 = board pin 1
// datasheet p.190; F_OSC = 16 MHz & baud rate = 19.200
#define UBBRVAL 51


void uart_init()
{
	// set the baud rate
	UBRR0H = 0; //UBRR0H = Baud Rate registers for HIGH (msb)
	UBRR0L = UBBRVAL; // Baud Rate register for LOW (lsb) set to UBBRVAL = 51
	// disable U2X mode
	UCSR0A = 0; //contains status data
	// enable transmitter by setting the UCsZ02 bit
	UCSR0B = _BV(RXEN0) | _BV(TXEN0); //TXEN0
	// set frame format : asynchronous, 8 data bits, 1 stop bit, no parity
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
	
	
}

void transmit(uint8_t data)
{
	// wait for an empty transmit buffer
	// UDRE is set when the transmit buffer is empty
	loop_until_bit_is_set(UCSR0A, UDRE0);
	// send the data
	UDR0 = data;
}

uint8_t receive()
{
	
	loop_until_bit_is_set(UCSR0A, RXC0);
	return UDR0;
	
}

int analogRead(uint8_t pin)
{
	uint8_t low, high;

	if (pin >= 14) pin -= 14; // allow for channel or pin numbers

	// set the analog reference (high two bits of ADMUX) and select the
	// channel (low 4 bits).  this also sets ADLAR (left-adjust result)
	// to 0 (the default).
	ADMUX = (1 << REFS0) | (pin & 0x07); //OPZOEKEN ADMUX!!! 
	ADMUX |= (1 << ADLAR);    // Right adjust for 8 bit resolution
	
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 128 prescale for 16Mhz
	ADCSRA |= (1 << ADEN);    // Enable the ADC
	
	// start the conversion
	//sbi(ADCSRA, ADSC);
	ADCSRA |= (1<<ADSC);
	
	// ADSC is cleared when the conversion finishes
	while (bit_is_set(ADCSRA, ADSC));

	// we have to read ADCL first; doing so locks both ADCL
	// and ADCH until ADCH is read.  reading ADCL second would
	// cause the results of each conversion to be discarded,
	// as ADCL and ADCH would be locked when it completed.
	low  = ADCL;
	high = ADCH;
	// combine the two bytes
	return (high << 8) | low;
	
}

int main(void)
{
	uart_init();
	
	//DDRB = 0xFF; //output
	//DDRC = 0x00; //input
	
	_delay_ms(1000);
	while (1) {
		int input = analogRead(0);//Jesse
		float voltage = input *5.0;
		voltage /= 1024.0;
		
		transmit(voltage);//Jesse
		_delay_ms(1000);//Jesse
		
		//transmit(0x33);  _delay_ms(1000);
		//transmit(0x77);  _delay_ms(1000);
		//transmit(0xbb);  _delay_ms(1000);
		
		//var1 = receive(); 
		//if(var1 > 0){
		//	PORTB = 0xFF;
		//	 _delay_ms(1000);
		//}
		//PORTB = 0;
	}
}
