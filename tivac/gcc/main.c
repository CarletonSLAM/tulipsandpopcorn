#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"
#include "driverlib/pwm.h"
#include "ioFunctions.h"
#include "wifiBoard.h"

#define UART_BUFFER_LENGTH 512


/******************************************************************************************************/
/***************************************FUNCTION PROTOTYPES********************************************/
/******************************************************************************************************/

void Tiva_setup(void);
void UART_setup_debug(void);

/******************************************************************************************************/
/*******************************************CONSTANTS**************************************************/
/******************************************************************************************************/


/******************************************************************************************************/
/*******************************************VARIABLES**************************************************/
/******************************************************************************************************/

extern char wifiUARTBuffer[UART_BUFFER_LENGTH];
extern uint16_t wifiUARTIndex; 

//**************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//**************************************************************
#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//************************************************************
//
//
//
//************************************************************
int main(void)
{
  //unsigned char wifi_reset =0;
  wifiUARTIndex = 0;

  Tiva_setup();
  // setup the logger
  UART_setup_debug();

  UARTSend(UART0_BASE,(uint8_t*)"\n---------------\n",17);

  bool setup = UART_setup_wifiBoard();

  if(setup == true){

    //Reinitialize setup variable
    setup = false;

    setup = wifiBoard_reset();

    if(setup==true){
       
       setup = wifiBoard_setNetworkMode();

       if(setup==true){
          wifiBoard_listNetworks();
       }

    }
  }
  //}

  //UARTSend(UART1_BASE,(uint8_t*)"ATE0\r\n",6);


  //UARTSend(UART0_BASE,(uint8_t*)"RESET DONE\n",11);
  //while(ROM_UARTBusy(UART1_BASE));
  
  while(1){

  }
}


void UART_setup_debug(void)
{
  //
  // Enable the peripherals used by the debug line.
  //
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

  //
  // Set GPIO A0 and A1 as UART pins for the debug.
  //
  ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
  ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
  ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  //
  // Configure the UART for 115,200, 8-N-1 operation.
  //
  ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 115200,
                          (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                           UART_CONFIG_PAR_NONE));
}



//************************************************************
//
// Initialization code for the protocols we use.
//
//************************************************************
void Tiva_setup(void)
{
  //
  // Enable lazy stacking for interrupt handlers.  This allows floating-point
  // instructions to be used within interrupt handlers, but at the expense of
  // extra stack usage.
  //
  ROM_FPUEnable();
  ROM_FPULazyStackingEnable();

  //
  // Set the clocking to run directly from the crystal.
  //
  ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                     SYSCTL_XTAL_16MHZ);
  //
  // Enable processor interrupts.
  //
  ROM_IntMasterEnable();


}


void wifiUARTIntHandler(void){
  uint32_t ui32Status;
    char c;
    uint32_t counter = 0;


    // Get the interrrupt status.
    ui32Status = ROM_UARTIntStatus(UART1_BASE, true);

    // Clear the asserted interrupts.
    ROM_UARTIntClear(UART1_BASE, ui32Status);

    // Loop while there are characters in the receive FIFO.
    while (ROM_UARTCharsAvail(UART1_BASE))
    {
      counter++;
        
        // Read the next character from the UART and write it back to the UART.
        c = ROM_UARTCharGetNonBlocking(UART1_BASE);
        
        //ROM_UARTCharPutNonBlocking(UART0_BASE,(uint8_t*) c);
        
        if (wifiUARTIndex++ >= UART_BUFFER_LENGTH){
          wifiUARTIndex = 0;
        }

        wifiUARTBuffer[wifiUARTIndex] = c;
    }


}