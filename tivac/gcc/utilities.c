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

void TIVA_init() 
{
  ROM_FPUEnable();
  ROM_FPULazyStackingEnable();

  //16 MHz clock
  ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
  
  ROM_IntMasterEnable();
  DEBUGCONSOLE_init();
  WIFI_init();
}

void DEBUGCONSOLE_init() 
{
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
  ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
  ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));



  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  ROM_GPIOPinConfigure(GPIO_PE4_U5RX);
  ROM_GPIOPinConfigure(GPIO_PE5_U5TX);
  ROM_GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);
  ROM_UARTConfigSetExpClk(UART5_BASE, ROM_SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}



void DEBUGCONSOLE_print(const char *pui8Buffer, uint32_t ui32Count)
{

   while(ui32Count--)
    {
        //ROM_UARTCharPut(UART0_BASE,(uint8_t)*pui8Buffer++);
        ROM_UARTCharPut(UART5_BASE,(uint8_t)*pui8Buffer++);
        //while (ROM_UARTBusy(UART0_BASE));
        while (ROM_UARTBusy(UART5_BASE));
    }
}
 
void DEBUGCONSOLE_print_line(const char *pui8Buffer, uint32_t ui32Count)
{

   while(ui32Count--)
    {
        //ROM_UARTCharPut(UART0_BASE,(uint8_t)*pui8Buffer++);
        ROM_UARTCharPut(UART5_BASE,(uint8_t)*pui8Buffer++);
        //while (ROM_UARTBusy(UART0_BASE));
        while (ROM_UARTBusy(UART5_BASE));
    }

    ROM_UARTCharPut(UART5_BASE,(uint8_t)'\r');
    ROM_UARTCharPut(UART5_BASE,(uint8_t)'\n');
    while (ROM_UARTBusy(UART5_BASE));
}

void UART_wait_forResponse(const uint32_t UARTBase)
{
  while(!ROM_UARTCharsAvail(UARTBase));
} 


int16_t FLASH_write(const uint32_t pui32Address, const char *pui8Data){

  int16_t response;

  DEBUGCONSOLE_print("BEFORE: ", 10);
  DEBUGCONSOLE_print_line(pui32Address, 16);

  // Erase a block of the flash.
  ROM_FlashErase(pui32Address);

  DEBUGCONSOLE_print("ERASED: ", 8);
  DEBUGCONSOLE_print_line(pui32Address, 16);

  DEBUGCONSOLE_print("FLASHING: ",10);
  DEBUGCONSOLE_print_line(pui8Data, sizeof(pui8Data)/sizeof(char));

  // Program some data into the newly erased block of the flash.
  response = ROM_FlashProgram(pui8Data, pui32Address, sizeof(pui8Data));
 
  
  if(response==0)
    DEBUGCONSOLE_print_line("FLASH WRITE: SUCCESS", 20);  
  else
    DEBUGCONSOLE_print_line("FLASH WRITE: FAIL", 18);  
  return response;
}

int16_t FLASH_read(const uint32_t pui32Address, char* pui8Data){

  uint32_t* addressPointer= (uint32_t*)pui32Address;
  pui8Data[0] = '\0';

  for(uint16_t i = 0; *addressPointer!=0xFF;i++){
    *(pui8Data+i) = *(addressPointer++);
  }
  if(pui8Data[0]=='\0')
    return -1;
  else
    return 0;
}



void TIVA_wait_miliSeconds(uint32_t ms) {
    uint16_t cnt =1001;

    while(cnt-- > 1){
        ROM_SysCtlDelay((ROM_SysCtlClockGet()/(3*1000000)*ms ));  // more accurate
    }
}

