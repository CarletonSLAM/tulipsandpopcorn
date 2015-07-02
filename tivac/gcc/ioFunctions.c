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

bool UARTSendEcho(uint32_t ui32Base, const uint8_t *pui8Buffer, uint32_t ui32Count)
{
    uint8_t rec[ui32Count];
    //
    // Loop while there are more characters to send.
    //
    for(uint32_t i=0; i< ui32Count;i++){
        //
        // Write the next character to the UART.
        //
        ROM_UARTCharPutNonBlocking(ui32Base, *(pui8Buffer+i) );
        while (ROM_UARTBusy(ui32Base));
        rec[i] = ROM_UARTCharGet(ui32Base);
    }

   for(uint32_t i=0; i< ui32Count;i++){
        if( !(*(pui8Buffer+i)==*(rec+i)) )
            return false;
    }
    return true;

}