#include <avr/io.h>
#include <stdlib.h>
#include <avr/sfr_defs.h>
//#include <time.h>
#define F_CPU 16E6
#include <util/delay.h>
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



int main(void)
{
	uart_init();
	DDRB = 0xFF;
	DDRC = 0x00;
	uint8_t test = 0; //Jesse
	uint8_t var1 = 0; //Jesse
	_delay_ms(1000);
	while (1) {
		
		test = PINC0;//Jesse
		transmit(test);//Jesse
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