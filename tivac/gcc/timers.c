#include <stdint.h>
#include <stdbool.h>


#include "hw_ints.h"
#include "hw_memmap.h"
#include "gpio.h"
#include "interrupt.h"
#include "pin_map.h"
#include "rom.h"
#include "sysctl.h"
#include "timer.h"



#include "utilities.h"
#include "tulips.h"


void TIVA_one_Second_Timer_Handler(void){

  ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  if(!COMPASS_get_raw_accel_values()){  while(1);}
  if(!COMPASS_get_raw_magn_values()){  while(1);}
  COMPASS_update_heading();
}

void TIVA_Timer_setup(void){

  //Activates every one second
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

  //
  // Configure the two 32-bit periodic timers.
  //
  ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
  ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ROM_SysCtlClockGet()>>3);

  //
  // Setup the interrupts for the timer timeouts.
  //
  ROM_IntEnable(INT_TIMER0A);
  ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

  //
  // Enable the timers.
  //
  ROM_TimerEnable(TIMER0_BASE, TIMER_A);
  DEBUGCONSOLE_print_line("500ms TIMER: Init --> OK\0");

}
