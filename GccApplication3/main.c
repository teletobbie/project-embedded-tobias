#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/sfr_defs.h>
#define F_CPU 16000000UL
#define BAUD 19200
#include <util/setbaud.h>
#include <util/delay.h>

#include "AVR_TTC_scheduler.h"
#include <avr/interrupt.h>

static	float rollout_temp = 21.5;
static	float rollout_light = 500;

int rolloutDistance = 160;
uint8_t rolloutFlag = 0;

// The array of tasks
sTask SCH_tasks_G[SCH_MAX_TASKS];

volatile uint16_t gv_counter = 0; // 16 bit counter value
volatile uint8_t gv_echo = 0; // a flag

volatile float avgTemperature;
volatile int cTemperature=1;

volatile float avgLux;
volatile int cLux=1;



/*------------------------------------------------------------------*-

  SCH_Dispatch_Tasks()

  This is the 'dispatcher' function.  When a task (function)
  is due to run, SCH_Dispatch_Tasks() will run it.
  This function must be called (repeatedly) from the main loop.

-*------------------------------------------------------------------*/
void SCH_Dispatch_Tasks(void)
{
   unsigned char Index;

   // Dispatches (runs) the next task (if one is ready)
   for(Index = 0; Index < SCH_MAX_TASKS; Index++)
   {
      if((SCH_tasks_G[Index].RunMe > 0) && (SCH_tasks_G[Index].pTask != 0))
      {
         (*SCH_tasks_G[Index].pTask)();  // Run the task
         SCH_tasks_G[Index].RunMe -= 1;   // Reset / reduce RunMe flag

         // Periodic tasks will automatically run again
         // - if this is a 'one shot' task, remove it from the array
         if(SCH_tasks_G[Index].Period == 0)
         {
            SCH_Delete_Task(Index);
         }
      }
   }
}
/*------------------------------------------------------------------*-

  SCH_Add_Task()

  Causes a task (function) to be executed at regular intervals 
  or after a user-defined delay

  pFunction - The name of the function which is to be scheduled.
              NOTE: All scheduled functions must be 'void, void' -
              that is, they must take no parameters, and have 
              a void return type. 
                   
  DELAY     - The interval (TICKS) before the task is first executed

  PERIOD    - If 'PERIOD' is 0, the function is only called once,
              at the time determined by 'DELAY'.  If PERIOD is non-zero,
              then the function is called repeatedly at an interval
              determined by the value of PERIOD (see below for examples
              which should help clarify this).


  RETURN VALUE:  

  Returns the position in the task array at which the task has been 
  added.  If the return value is SCH_MAX_TASKS then the task could 
  not be added to the array (there was insufficient space).  If the
  return value is < SCH_MAX_TASKS, then the task was added 
  successfully.  

  Note: this return value may be required, if a task is
  to be subsequently deleted - see SCH_Delete_Task().

  EXAMPLES:

  Task_ID = SCH_Add_Task(Do_X,1000,0);
  Causes the function Do_X() to be executed once after 1000 sch ticks.            

  Task_ID = SCH_Add_Task(Do_X,0,1000);
  Causes the function Do_X() to be executed regularly, every 1000 sch ticks.            

  Task_ID = SCH_Add_Task(Do_X,300,1000);
  Causes the function Do_X() to be executed regularly, every 1000 ticks.
  Task will be first executed at T = 300 ticks, then 1300, 2300, etc.            
 
-*------------------------------------------------------------------*/
unsigned char SCH_Add_Task(void (*pFunction)(), const unsigned int DELAY, const unsigned int PERIOD)
{
   unsigned char Index = 0;

   // First find a gap in the array (if there is one)
   while((SCH_tasks_G[Index].pTask != 0) && (Index < SCH_MAX_TASKS))
   {
      Index++;
   }

   // Have we reached the end of the list?   
   if(Index == SCH_MAX_TASKS)
   {
      // Task list is full, return an error code
      return SCH_MAX_TASKS;  
   }

   // If we're here, there is a space in the task array
   SCH_tasks_G[Index].pTask = pFunction;
   SCH_tasks_G[Index].Delay =DELAY;
   SCH_tasks_G[Index].Period = PERIOD;
   SCH_tasks_G[Index].RunMe = 0;

   // return position of task (to allow later deletion)
   return Index;
}
/*------------------------------------------------------------------*-

  SCH_Delete_Task()

  Removes a task from the scheduler.  Note that this does
  *not* delete the associated function from memory: 
  it simply means that it is no longer called by the scheduler. 
 
  TASK_INDEX - The task index.  Provided by SCH_Add_Task(). 

  RETURN VALUE:  RETURN_ERROR or RETURN_NORMAL

-*------------------------------------------------------------------*/
unsigned char SCH_Delete_Task(const unsigned char TASK_INDEX)
{
   // Return_code can be used for error reporting, NOT USED HERE THOUGH!
   unsigned char Return_code = 0;

   SCH_tasks_G[TASK_INDEX].pTask = 0;
   SCH_tasks_G[TASK_INDEX].Delay = 0;
   SCH_tasks_G[TASK_INDEX].Period = 0;
   SCH_tasks_G[TASK_INDEX].RunMe = 0;

   return Return_code;
}
/*------------------------------------------------------------------*-

  SCH_Init_T1()

  Scheduler initialisation function.  Prepares scheduler
  data structures and sets up timer interrupts at required rate.
  You must call this function before using the scheduler.  

-*------------------------------------------------------------------*/
void SCH_Init_T2(void)
{
   unsigned char i;

   for(i = 0; i < SCH_MAX_TASKS; i++)
   {
      SCH_Delete_Task(i);
   }

   // Set up Timer 1
   // Values for 1ms and 10ms ticks are provided for various crystals

   // Hier moet de timer periode worden aangepast ....!
   OCR2A = (uint8_t)625;   		     // 10ms = (256/16.000.000) * 625
   TCCR2B = (1 << CS12) | (1 << WGM12);  // prescale op 64, top counter = value OCR1A (CTC mode)
   TIMSK2 = 1 << OCIE2A;   		     // Timer 1 Output Compare A Match Interrupt Enable
}
/*------------------------------------------------------------------*-

  SCH_Start()

  Starts the scheduler, by enabling interrupts.

  NOTE: Usually called after all regular tasks are added,
  to keep the tasks synchronised.

  NOTE: ONLY THE SCHEDULER INTERRUPT SHOULD BE ENABLED!!! 
 
-*------------------------------------------------------------------*/
void SCH_Start(void)
{
      sei();
}
/*------------------------------------------------------------------*-

  SCH_Update

  This is the scheduler ISR.  It is called at a rate 
  determined by the timer settings in SCH_Init_T1().

-*------------------------------------------------------------------*/
ISR(TIMER2_COMPA_vect)
{
   unsigned char Index;
   for(Index = 0; Index < SCH_MAX_TASKS; Index++)
   {
      // Check if there is a task at this location
      if(SCH_tasks_G[Index].pTask)
      {
         if(SCH_tasks_G[Index].Delay == 0)
         {
            // The task is due to run, Inc. the 'RunMe' flag
            SCH_tasks_G[Index].RunMe += 1;

            if(SCH_tasks_G[Index].Period)
            {
               // Schedule periodic tasks to run again
               SCH_tasks_G[Index].Delay = SCH_tasks_G[Index].Period;
               SCH_tasks_G[Index].Delay -= 1;
            }
         }
         else
         {
            // Not yet ready to run: just decrement the delay
            SCH_tasks_G[Index].Delay -= 1;
         }
      }
   }
}

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
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0)| (1 << ADEN); // 128 prescale for 16Mhz & enable adc
}
uint16_t analogRead(uint8_t pin)
{
	
	_delay_ms(10);

	if (pin >= 14) pin -= 14; // allow for channel or pin numbersuint8_t low, high;
	
	ADMUX = (1 << REFS0) | (pin & 0x07);
	// start the conversion
	ADCSRA |= (1<<ADSC);
	
	// ADSC is cleared when the conversion finishes
	while (bit_is_set(ADCSRA, ADSC));
	
	return ADC;
}

void init_timerUltrasoon()
{
	TCCR1A = 0;
	TCCR1B = 0;
}

void init_ext_int()
{
	// any change triggers ext interrupt 1
	EICRA = (1 << ISC10);
	EIMSK = (1 << INT1);
}

void init_ultrasoon(void)
{
	DDRD |= _BV(DDB4); // set port D4 as output
	DDRD &= ~_BV(DDB3); // set port D3 as input
	
	init_timerUltrasoon();
	init_ext_int();
}

void trigger_ultrasoon(void)
{
	PORTD|=(1<<PIND4);
	_delay_ms(10);
	PORTD &= ~(1<<PIND4);
}

uint16_t get_distance()
{
	trigger_ultrasoon();
	
	if (gv_counter <= 46400){
		
		uint16_t distance = gv_counter*0.5;
		distance /=58;
	
		return distance;
	}
	else
	{
		uint16_t distance = 400;
		return distance;
	}
} 

float get_lux(int in){
		int input = in;
		
		float voltageLight = input * 5.0;
		voltageLight /= 1024;
		
		float rldr = (10*voltageLight)/(5-voltageLight);
		float lux = 500/rldr;
		return lux;
}

float get_temp(int in){
			int input = in;
			float voltage = input * 5.0;
			voltage /= 1024;
			
			float temperature;
			temperature = (voltage - 0.5) * 100 ;
			return temperature;
}

void init_timerPWM()
{
		// Phase Correct PWM 8 Bit, Clear OCA0 on Compare Match
		// Set on TOP
		TCCR0A = (1 << WGM00) | (1 << COM0A1);
		// init PWM value
		OCR0A = 0;
}

void rollout(){
	uint8_t pwm = 0;
	int increase = 5;
	PORTB &= ~(1<<PINB1);
	PORTB|=(1<<PINB0);
	TCCR0B = (1 << CS01) | (1 << CS00);
	while (get_distance() < rolloutDistance)
	{
		pwm += increase;
		OCR0A = pwm;
			
		if (pwm == 0 || pwm == 255)
		{
			increase = -increase;
		}
	}
	TCCR0B = 0;
	TCNT0 = 0;
	OCR0A = 0;
	rolloutFlag = 1;
	
}

void rollin()
{
	uint8_t pwm = 0;
	int increase = 5;
	PORTB &= ~(1<<PINB0);
	PORTB|=(1<<PINB1);
	TCCR0B = (1 << CS01) | (1 << CS00);
	while (get_distance() > 5)
	{
		pwm += increase;
		OCR0A = pwm;
		
		if (pwm == 0 || pwm == 255)
		{
			increase = -increase;
		}
	}
	TCCR0B = 0;
	TCNT0 = 0;
	OCR0A = 0;
	rolloutFlag = 0;
}

void init_rollout()
{
	DDRB = 0xFF; // set all B ports as output
	DDRD |= _BV(DDB6); // Set port d6 as output (OC0A)
	init_timerPWM();
	
	if(avgTemperature >= rollout_temp || avgLux >= rollout_light)
	{
		if (get_distance() < rolloutDistance)
		{
			rollout();
		}
		else
		{
			PORTB &= ~(1<<PINB1);
			PORTB|=(1<<PINB0);
		}
	}
	if(avgTemperature < rollout_temp && avgLux < rollout_light)
	{
		if 	(get_distance() > 6)
		{
			rollin();
		}
		else
		{
			PORTB &= ~(1<<PINB0);
			PORTB|=(1<<PINB1);
		}
	}
}


void check_rollout()
{

	if(avgTemperature >= rollout_temp || avgLux >= rollout_light)
	{
		if (rolloutFlag == 0)
			{
				rollout();
			}
	}
	if(avgTemperature < rollout_temp && avgLux < rollout_light)
	{
		if 	(rolloutFlag == 1)
		{
			rollin();
		}
	}
}


void calculateAvgTemperature()
{
	if (cTemperature==3)
	{
		avgTemperature += get_temp(analogRead(0));
		avgTemperature /= cTemperature;
		cTemperature=1;
	}
	if (cTemperature !=1 && cTemperature !=3)
	{
		avgTemperature += get_temp(analogRead(0));
		avgTemperature /= cTemperature;
		++cTemperature;

	}
	
	if (cTemperature==1)
	{
		avgTemperature = get_temp(analogRead(0));
		++cTemperature;
		
	}
	

}

void calculateAvgLux()
{

	if (cLux==4)
	{
		avgLux += get_lux(analogRead(1));
		avgLux /= cLux;
		cLux=1;
	}
	if (cLux !=1 && cLux !=4)
	{
		avgLux += get_lux(analogRead(1));
		avgLux /= cLux;
		cLux++;
		
	}

	if (cLux==1)
	{
		avgLux = get_lux(analogRead(1));
		cLux++;
	}
}

void sendStatus()
{
	uint16_t distance = get_distance();
	
	printf("T %.1f \n", avgTemperature);
	printf("L %.0f \n", avgLux);
	printf("D %u \n", distance);
}

int main(void)
{
	DDRB = 0xFF;
	
	uart_init();
	stdout = &uart_output;
	stdin  = &uart_input;
	

	
	SCH_Init_T2();
	
	init_ultrasoon();
		
	init_analogRead();
	
	init_rollout();
	
	SCH_Add_Task(calculateAvgTemperature, 0, 4000);
	SCH_Add_Task(calculateAvgLux, 0, 3000);
	SCH_Add_Task(check_rollout, 0, 12000);
	SCH_Add_Task(sendStatus, 0, 6000);


	SCH_Start();
	
	while (1)
	{
		SCH_Dispatch_Tasks();		
	}
}

ISR(INT1_vect)
{
	if (gv_echo==1)
	{
		TCCR1B=0;
		gv_counter=TCNT1;
		TCNT1=0;
		gv_echo=0;
	}
	if (gv_echo==0)
	{
		TCCR1B|=(1<<CS11);
		gv_echo=1;
	}
}