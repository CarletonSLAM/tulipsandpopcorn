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

  
  // Enable the peripherals used by wifiBoard.
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

  // Set GPIO B0 and B1 as UART pins for the gps.
  ROM_GPIOPinConfigure(GPIO_PB0_U1RX);
  ROM_GPIOPinConfigure(GPIO_PB1_U1TX);
  ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  // Configure the GPS for 9600, 8-N-1 operation.
  ROM_UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet(), 9600,
                          (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                           UART_CONFIG_PAR_NONE));

  // Enable the UART interrupt.
  ROM_IntEnable(INT_UART1);
  ROM_UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);


  
  delay_uS(1000);

  UARTSend(UART0_BASE,(uint8_t*)"Wifi - Echo Off: ",17);
  clearUARTBuffer(&wifiUARTIndex);

  return wifiBoard_sendCommand(UART1_BASE,(uint8_t*)"ATE0",4,(char*)"OK" ,20, 100);

}



bool wifiBoard_reset(void){

  UARTSend(UART0_BASE,(uint8_t*)"Wifi - Reset: ",14);

  //Reset wifi buffer pointer
  clearUARTBuffer(&wifiUARTIndex);
  
  return wifiBoard_sendCommand(UART1_BASE,(uint8_t*)"AT+RST",6 ,(char*)"ready" ,200, 2000);

}





bool wifiBoard_setNetworkMode(void){

  
  UARTSend(UART0_BASE,(uint8_t*)"Wifi - Setting Network Mode: ",29);

  //Reset wifi buffer pointer
  clearUARTBuffer(&wifiUARTIndex);
  
  return wifiBoard_sendCommand(UART1_BASE,(uint8_t*)"AT+CWMODE=3",11 ,(char*)"OK" ,20, 10);

}


void wifiBoard_listNetworks(void){

  UARTSend(UART0_BASE,(uint8_t*)"List of available networks:\n",28);


  //Reset wifi buffer pointer
  clearUARTBuffer(&wifiUARTIndex);

  UARTSend(UART1_BASE,(uint8_t*)  "AT+CWLAP", 8);
  UARTSend(UART1_BASE,(uint8_t*)"\r\n",2);

  delay_mS(1000);

  //Reset wifi buffer pointer
  clearUARTBuffer(&wifiUARTIndex);

  delay_mS(2000);
  uint16_t indexBuff =0;
  uint16_t indexBuffnew=0;


  //Trying to get the parsing for the network lists
  while(indexBuffnew>wifiUARTIndex - 50){

    for(uint8_t k=0; k<10;k++){
      
      indexBuffnew = findBrute( (wifiUARTBuffer+indexBuff) , "\n", 150 , 1);

      if(indexBuffnew!=1099){
        for( uint16_t i = 0 ; i < indexBuffnew-indexBuff ; i++ ){
          wifiNetworks[k][i] = wifiUARTBuffer[indexBuff+i];
        }

        indexBuff = indexBuffnew+1;

        UARTSend(UART0_BASE,(uint8_t*) wifiNetworks[k], 150);
        
        UARTSend(UART0_BASE,(uint8_t*)"\r\n",2);
      }
    }
  }



  //Prints our raw buffer
  UARTSend(UART0_BASE,(uint8_t*)"\n\n\n\n\n",5);
   
  UARTSend(UART0_BASE,(uint8_t*) wifiUARTBuffer, 1024);



}




bool wifiBoard_sendCommand(uint32_t wifiBase, const uint8_t* wifiCommand, const uint16_t commandLen,  const char* wifiResponse, uint16_t searchLength, uint32_t commTimeoutmS){


  UARTSend(UART1_BASE,(uint8_t*) wifiCommand, commandLen);
  UARTSend(UART1_BASE,(uint8_t*)"\r\n",2);

  delay_mS(commTimeoutmS);

  uint16_t index = findBrute(wifiUARTBuffer, wifiResponse, searchLength, (sizeof(*wifiResponse)/sizeof(char)) );

  if(index !=1099){
    
    //Reset wifi buffer pointer
    clearUARTBuffer(&wifiUARTIndex);
    
    UARTSend(UART0_BASE,(uint8_t*)"SUCCESS\n",8);
    return true;
  }
  else{
    UARTSend(UART0_BASE,(uint8_t*)"ERROR\n",6);
    UARTSend(UART0_BASE,(uint8_t*)"Wifi Buffer Output:\n",20); 
    UARTSend(UART0_BASE,(uint8_t*) wifiUARTBuffer, 100);
    return false;
  }

}