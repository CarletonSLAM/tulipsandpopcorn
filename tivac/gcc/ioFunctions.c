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
#include "driverlib/uart.h"



//Variables
char wifiUARTBuffer[UART_BUFFER_LENGTH];
uint16_t wifiUARTIndex; 

char wifiNetworks[20][100];


void delay_uS(uint32_t us) {
  ROM_SysCtlDelay((ROM_SysCtlClockGet()/(3*1000000)*us ));  // more accurate
}

void delay_mS(uint32_t ms) {
    uint16_t cnt =1001;

    while(cnt-- > 1){
        ROM_SysCtlDelay((ROM_SysCtlClockGet()/(3*1000000)*ms ));  // more accurate
    }
}


//*************************************************************
//
// Send a string to the UART.
//
//*************************************************************
void UARTSend(uint32_t ui32Base, const uint8_t *pui8Buffer, uint32_t ui32Count){
    //
    // Loop while there are more characters to send.
    //
    while(ui32Count--)
    {
        // Write the next character to the UART.
        ROM_UARTCharPut(ui32Base, *pui8Buffer++);
    }    
    while (ROM_UARTBusy(ui32Base));
}


/*
bool UARTSendEcho(uint32_t destUARTBase, const uint8_t *destMessage, uint32_t destLength)
{
    char recieve[UART_BUFFER_LENGTH];
    int i =0;
    //
    // Loop while there are more characters to send.
    //
    UARTSend(destUARTBase,destMessage,destLength);
    //UARTSend(UART0_BASE,destMessage,destLength);

        //for(uint32_t i=0; i< destLength;i++){
        //
        // Write the next character to the UART.
        //
        //ROM_UARTCharPutNonBlocking(UART0_BASE, *(destMessage+i) );
    //    ROM_UARTCharPutNonBlocking(destUARTBase, *(destMessage+i) );
        //while(ROM_UARTBusy(destUARTBase));
    //}
    while(!ROM_UARTCharsAvail(destUARTBase));

    
    UARTSend(UART0_BASE,(uint8_t*) wifiUARTBuffer , UART_BUFFER_LENGTH);
    
    //while (ROM_UARTBusy(UART0_BASE));

    // /UARTSend(UART0_BASE, (uint8_t*)"ECHO RECEIEVED\n" ,15);
    //UARTSend(UART0_BASE,(uint8_t*) recieve , destLength);
    
    for(uint32_t i=0; i< destLength;i++){
        if( !(*(destMessage+i)==*(recieve+i)) )
            UARTSend(UART0_BASE, (uint8_t*)"ECHO NOT RECEIVED\n" ,18);
            return false;
    }

    UARTSend(UART0_BASE, (uint8_t*)"ECHO RECEIEVED\n" ,15);
    
    return true;

}
 */



void clearUARTBuffer(uint16_t* UARTIndex){
    *UARTIndex=0;

}


uint16_t findBrute(char* buffer, char*pattern, uint16_t searchLen, uint16_t pattLen){
     for (uint16_t i = 0; i <= searchLen-pattLen; i++) {
        uint16_t k = 0;
        while(k < pattLen &&  *(buffer+i+k)== *(pattern + k)){
            k++;
        }
        if( k == pattLen){
            return i;
        }
    }

    return 1099;

}

