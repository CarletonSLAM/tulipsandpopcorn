#include "ioFunctions.h"
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"


void delay_uS(uint32_t us) {
  ROM_SysCtlDelay((ROM_SysCtlClockGet()/(3*1000000)*us ));  // more accurate
}
