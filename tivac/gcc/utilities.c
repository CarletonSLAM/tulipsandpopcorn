#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "wifiBoard.h"
#include "utilities.h"

void setup_tiva() 
{
  ROM_FPUEnable();
  ROM_FPULazyStackingEnable();
  ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
  ROM_IntMasterEnable();
  init_debugConsole();
  init_wifi();
}

void init_debugConsole() 
{
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
  ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
  ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}



void print_toDebugConsole(const char *pui8Buffer, uint32_t ui32Count)
{

   while(ui32Count--)
    {
        ROM_UARTCharPut(UART0_BASE,(uint8_t)*pui8Buffer++);
        while (ROM_UARTBusy(UART0_BASE));
    }
}
 

void wait_forResponse(const uint32_t UARTBase)
{
  while(!ROM_UARTCharsAvail(UARTBase));
} 




void delay_miliSeconds(uint32_t ms) {
    uint16_t cnt =1001;

    while(cnt-- > 1){
        ROM_SysCtlDelay((ROM_SysCtlClockGet()/(3*1000000)*ms ));  // more accurate
    }
}