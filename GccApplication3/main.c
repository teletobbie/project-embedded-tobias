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

// The array of tasks
sTask SCH_tasks_G[SCH_MAX_TASKS];


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
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0)| (1 << ADEN); // 128 prescale for 16Mhz & enable adc
}
uint16_t analogRead(uint8_t pin)
{
	
	_delay_ms(10);

	if (pin >= 14) pin -= 14; // allow for channel or pin numbersuint8_t low, high;
	
	ADMUX = (1 << REFS0) | (pin & 0x07);
	// start the conversion
	//sbi(ADCSRA, ADSC);
	ADCSRA |= (1<<ADSC);
	
	// ADSC is cleared when the conversion finishes
	while (bit_is_set(ADCSRA, ADSC));
	
	return ADC;
}


float send_temp(void){
		int input = analogRead(0);
		float voltage = input * 5.0;
		voltage /= 1024;
			
		float temperature;
		temperature = (voltage - 0.5) * 100 ;
		printf("T %.1f\n", temperature);
}
float send_lux(void){
	
	int input = analogRead(1);
	
	float voltageLight = input * 5.0;
	voltageLight /= 1024;
	
	float rldr = (10*voltageLight)/(5-voltageLight);
	float lux = 500/rldr;
	printf("L %.1f\n", lux);
	
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

void check_rollout()
{
	float temperature = get_temp(analogRead(0));;
	float lux = get_lux(analogRead(1));
	if((temperature > rollout_temp) || (lux > rollout_light)){
		PORTB = 0xFF;
	}
	else{
		PORTB = 0x00;
	}
}
void rollout(){
	PORTB = 0xFF;
}
void rollin(){
	PORTB = 0x00;
}

int main(void)
{
	DDRB = 0xFF;
	
	uart_init();
	stdout = &uart_output;
	stdin  = &uart_input;
	
	init_analogRead();
	SCH_Init_T2();
	
	SCH_Add_Task(send_lux, 1000, 10000);
	SCH_Add_Task(send_temp, 1000, 5000);
	SCH_Add_Task(check_rollout, 1000, 1000);
	SCH_Start();
	
	while (1) {
		SCH_Dispatch_Tasks();		
	}
}