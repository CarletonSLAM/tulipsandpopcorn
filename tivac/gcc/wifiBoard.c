#include "wifiBoard.h"
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

void UART_setup_wifiBoard()
{
  //
  // Enable the peripherals used by wifiBoard.
  //
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

  //
  // Set GPIO B0 and B1 as UART pins for the gps.
  //
  ROM_GPIOPinConfigure(GPIO_PB0_U1RX);
  ROM_GPIOPinConfigure(GPIO_PB1_U1TX);
  ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  //
  // Configure the GPS for 9600, 8-N-1 operation.
  //
  ROM_UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet(), 9600,
                          (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                           UART_CONFIG_PAR_NONE));

  //
  // Enable the UART interrupt.
  //
  ROM_IntEnable(INT_UART1);
  ROM_UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
}

void wifiBoard_reset(void){
  
  UARTSend("AT+RST\n");
  
  delayms(1000);

}
  //*************************************************************
//
// Send a string to the UART.
//
//*************************************************************
void UARTSend(uint32_t ui32Base, const uint8_t *pui8Buffer, uint32_t ui32Count)
{
    //
    // Loop while there are more characters to send.
    //
    while(ui32Count--)
    {
        //
        // Write the next character to the UART.
        //
        ROM_UARTCharPutNonBlocking(ui32Base, *pui8Buffer++);
        while (ROM_UARTBusy(ui32Base));

    }
}


}

void wifiBoard_connect(char host, char port);
*char wifiBoard_send(char message, int length);
void wifiBoard_disconnect(void);



