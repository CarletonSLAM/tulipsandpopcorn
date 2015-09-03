#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "utilities.h"
#include "dcMotor.h"

uint8_t motorDuration;
bool	motorPowered;

// BASE PIN, ENABLE PIN, FORWARD PIN, REVERSE PIN
const MOTOR_PINS MOTOR_L = {GPIO_PORTA_BASE,GPIO_PIN_4,GPIO_PIN_3,GPIO_PIN_2};

const MOTOR_PINS MOTOR_R = {GPIO_PORTC_BASE,GPIO_PIN_4,GPIO_PIN_5,GPIO_PIN_6};

void DCMOTOR_init(){
	ROM_SysCtlPeripheralEnable( MOTOR_L.Enable);
	ROM_GPIOPinTypeGPIOOutput( MOTOR_L.Base,MOTOR_L.Enable|MOTOR_L.Clockwise|MOTOR_L.CounterClockwise);

	motorPowered=false;
}



void DCMOTOR_move_forward_seconds(uint16_t seconds){
	if(!motorPowered){

	    //Turn Left Motor Clockwise		
		ROM_GPIOPinWrite( MOTOR_L.Base,
				MOTOR_L.Enable|MOTOR_L.Clockwise|MOTOR_L.CounterClockwise,
				MOTOR_L.Enable|MOTOR_L.Clockwise|~MOTOR_L.CounterClockwise);

	    /*Turn Right Motor Counter Clockwise
		ROM_GPIOPinWrite( MOTOR_R.Base,
				MOTOR_R.Enable|MOTOR_R.Clockwise|MOTOR_R.CounterClockwise,
				MOTOR_R.Enable|~MOTOR_R.Clockwise|MOTOR_R.CounterClockwise);
				*/
		motorDuration=seconds;
		motorPowered=true;
		while(motorPowered==true){
			DEBUGCONSOLE_print("Time Left: ",11);
	    	DEBUGCONSOLE_print_line((char*) motorDuration+'0',1);
	    	if(motorDuration>0){
	    		TIVA_wait_miliSeconds(1000);
	    		motorDuration--;
	    	}
	    	else{
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

	}
}

void DCMOTOR_move_reverse_seconds(uint16_t seconds){
	if(!motorPowered){
		
	    //Turn Left Motor Clockwise		
		ROM_GPIOPinWrite( MOTOR_L.Base,
				MOTOR_L.Enable|MOTOR_L.Clockwise|MOTOR_L.CounterClockwise,
				MOTOR_L.Enable|~MOTOR_L.Clockwise|MOTOR_L.CounterClockwise);

	    /*Turn Right Motor Counter Clockwise
		ROM_GPIOPinWrite( MOTOR_R.Base,
				MOTOR_R.Enable|MOTOR_R.Clockwise|MOTOR_R.CounterClockwise,
				MOTOR_R.Enable|MOTOR_R.Clockwise|~MOTOR_R.CounterClockwise);
		*/
		motorDuration=seconds;
		motorPowered=true;
		while(motorPowered==true){
				DEBUGCONSOLE_print("Time Left: ",11);
		    	DEBUGCONSOLE_print_line((char*)motorDuration+'0',1);
		    	if(motorDuration>0){
		    		TIVA_wait_miliSeconds(1000);
		    		motorDuration--;
		    	}
		    	else{
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
	}

}