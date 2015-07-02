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
#include "ioFunctions.h"

bool UART_setup_wifiBoard()
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

  bool received = UARTSendEcho(UART1_BASE,(uint8_t)"ATE0\r\n",6);

  if(received == true){
    //UARTSend(UART0_BASE,(uint8_t*)"ECHO-SUCC\n",10);
    uint8_t resp[10];
    uint8_t cnt = 0;
    while(ROM_UARTCharsAvail(UART1_BASE)){
      //resp[cnt++]=(uint8_t) ROM_UARTCharGetNonBlocking(UART1_BASE);
      ROM_UARTCharPut(UART0_BASE,ROM_UARTCharGet(UART1_BASE));
    }
    for(int i = 0;i<=cnt;i++){
      if(resp[i]=='o' && resp[i+1]=='k'){
        return true;
      }
    }
    return false;


  }
  else{
    return false;
  }

  
}






unsigned char wifiBoard_reset(void){
  uint8_t RECVLENGTH = 50;
  uint8_t recv[RECVLENGTH];

  uint8_t counter=0;
  
  //UARTSend(UART0_BASE,(uint8_t*)"AT+RST\r\n",8);
  
  UARTSend(UART1_BASE,(uint8_t*)"AT+RST\r\n",8);
  
  UARTSend(UART0_BASE,(uint8_t*)"AT+RST\r\n",8);
  


  while(!ROM_UARTCharsAvail(UART1_BASE));
  while (counter<RECVLENGTH)
    {
      //
      // Read the next character from the UART and write it back to the UART.
      //recv[counter] = ROM_UARTCharGet(UART1_BASE);
        
      ROM_UARTCharPut(UART0_BASE, ROM_UARTCharGet(UART1_BASE));

      //recv[counter++] = (uint8_t) ROM_UARTCharGet(UART1_BASE); 
      //UARTSend(UART0_BASE,(uint8_t*) "OK\n",3);
      //counter++;
      //while(!ROM_UARTCharsAvail(UART1_BASE));  
    }
    UARTSend(UART0_BASE,(uint8_t*)recv[0],RECVLENGTH);

    for(uint8_t i=0;i < RECVLENGTH;i++){
      if(recv[i]=='O'&&recv[i+1]=='K'){
        UARTSend(UART0_BASE,(uint8_t*)"1\n",2);
        return 1;
      }
    }

  UARTSend(UART0_BASE,(uint8_t*)"0\n",2);
  return 0;
  
}



