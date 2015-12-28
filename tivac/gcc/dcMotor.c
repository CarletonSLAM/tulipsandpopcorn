#include <stdint.h>
#include <stdbool.h>
#include "hw_ints.h"
#include "hw_memmap.h"
#include "gpio.h"
#include "interrupt.h"
#include "pin_map.h"
#include "rom.h"
#include "sysctl.h"

#include "tulips.h"
#include "utilities.h"
#include "dcMotor.h"


bool	motorPowered;

// BASE PIN, ENABLE PIN, FORWARD PIN, REVERSE PIN
const MOTOR_PINS MOTOR_L = {GPIO_PORTA_BASE,GPIO_PIN_4,GPIO_PIN_3,GPIO_PIN_2};

const MOTOR_PINS MOTOR_R = {GPIO_PORTC_BASE,GPIO_PIN_4,GPIO_PIN_5,GPIO_PIN_6};

void DCMOTOR_init(void){
  DEBUGCONSOLE_print_line("DEBUG 1\0");
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	DEBUGCONSOLE_print_line("DEBUG 2\0");
	ROM_GPIOPinTypeGPIOOutput( MOTOR_L.Base,MOTOR_L.Enable|MOTOR_L.Clockwise|MOTOR_L.CounterClockwise);

	DEBUGCONSOLE_print_line("DEBUG 3\0");
	motorPowered=false;
}



void DCMOTOR_move_forward(void){
	if(!motorPowered){

	  //Turn Left Motor Clockwise
		ROM_GPIOPinWrite( MOTOR_L.Base,
				MOTOR_L.Enable|MOTOR_L.Clockwise|MOTOR_L.CounterClockwise,
				MOTOR_L.Enable|~MOTOR_L.Clockwise|MOTOR_L.CounterClockwise);

	  /*Turn Right Motor Counter Clockwise
		ROM_GPIOPinWrite( MOTOR_R.Base,
				MOTOR_R.Enable|MOTOR_R.Clockwise|MOTOR_R.CounterClockwise,
				MOTOR_R.Enable|~MOTOR_R.Clockwise|MOTOR_R.CounterClockwise);
		*/
		motorPowered=true;
  }
}

void DCMOTOR_stop(void){
	if(motorPowered){

		//Turn off Left Motor
		ROM_GPIOPinWrite( MOTOR_L.Base,
				MOTOR_L.Enable|MOTOR_L.Clockwise|MOTOR_L.CounterClockwise,
				~(MOTOR_L.Enable|MOTOR_L.Clockwise|MOTOR_L.CounterClockwise));

		/*Turn off Right Motor
		ROM_GPIOPinWrite( MOTOR_R.Base,
				MOTOR_R.Enable|MOTOR_R.Clockwise|MOTOR_R.CounterClockwise,
				~(MOTOR_R.Enable|MOTOR_R.Clockwise|MOTOR_R.CounterClockwise));
		*/
		motorPowered=false;

	}
}
