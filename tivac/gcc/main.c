#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "inc/hw_i2c.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/uart.h"
#include "driverlib/fpu.h"
#include "utilities.h"
#include "wifiBoard.h"


extern void init_debugConsole();
extern void wait_forResponse(const uint32_t UARTBase);
extern void print_toDebugConsole(const char *pui8Buffer, uint32_t ui32Count);
extern void clear_wifiUartBuffer();
extern void delay_miliSeconds(uint32_t ms);

extern void setup_wifi(void);
extern void setup_tiva(void);
extern void send_wifiCommandBlocking(const AT_CMD *cmd);
extern void wifiUARTIntHandler(void);

extern char wifiUARTBuffer[UART_BUFFER_LENGTH];
extern uint16_t wifiUARTIndex;
extern bool wifiResetComplete;


typedef void (*ResponseOperator)(char dataFromHW);

void operateOnResponseData(ResponseOperator operation);

const AT_CMD _ATCWLAP = {10,5000, "AT+CWLAP\r\n"};
const AT_CMD _ATCWJAP = {42,10000, "AT+CWJAP=\"dd-wrt\",\"3ab9bcf43er\"\r\n"};
const AT_CMD _ATCWJAP_CUR = {11,500, "AT+CWJAP?\r\n"};

const AT_CMD _ATPING = {26, 3000, "AT+PING=\"192.168.1.1\"\r\n"};
const AT_CMD _ATCIPSTART = {41,5000, "AT+CIPSTART=4,\"TCP\",\"www.google.com\",80\r\n"};
const AT_CMD _ATCIPSEND = {17,1000,"AT+CIPSEND=4,42\r\n"};
const AT_CMD dataSend = {42,1000,"GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n\r\n"};



int main(void) {
  char command[25];
  setup_tiva();

  setup_wifi();

  print_toDebugConsole(wifiUARTBuffer, UART_BUFFER_LENGTH);
  clear_wifiUartBuffer();

  //---------------------------------

  strcpy(command,"Wifi:List Networks\n");
  print_toDebugConsole(command, sizeof(command)/sizeof(char));
  
  send_wifiCommandBlocking(&_ATCWLAP);
  print_toDebugConsole(wifiUARTBuffer, UART_BUFFER_LENGTH);
  clear_wifiUartBuffer();

  //---------------------------------

  strcpy(command,"Wifi:Join Network\n");
  print_toDebugConsole(command, sizeof(command)/sizeof(char));
  
  send_wifiCommandBlocking(&_ATCWJAP);
  print_toDebugConsole(wifiUARTBuffer, UART_BUFFER_LENGTH);
  clear_wifiUartBuffer();

  //---------------------------------

  strcpy(command,"Wifi:Connected to: ");
  print_toDebugConsole(command, sizeof(command)/sizeof(char));
  
  send_wifiCommandBlocking(&_ATCWJAP_CUR);
  print_toDebugConsole(wifiUARTBuffer, UART_BUFFER_LENGTH);
  clear_wifiUartBuffer();

  //---------------------------------


  strcpy(command,"Wifi:TCP w/ Google\n");
  print_toDebugConsole(command, sizeof(command)/sizeof(char));

  send_wifiCommandBlocking(&_ATCIPSTART);
  print_toDebugConsole(wifiUARTBuffer, UART_BUFFER_LENGTH);
  clear_wifiUartBuffer();

  //---------------------------------


  strcpy(command,"Wifi:Send Command\n");
  
  print_toDebugConsole(command, sizeof(command)/sizeof(char));

  send_wifiCommandBlocking(&_ATCIPSEND);
  //print_toDebugConsole(wifiUARTBuffer, UART_BUFFER_LENGTH);
  clear_wifiUartBuffer();
  send_wifiCommandBlocking(&dataSend);
  


  //---------------------------------


  strcpy(command,"Wifi:Receive from Google\n");
  print_toDebugConsole(command, sizeof(command)/sizeof(char));
 
  print_toDebugConsole(wifiUARTBuffer, UART_BUFFER_LENGTH);
  
  //operateOnResponseData(&print_toDebugConsole);

  //send_wifiCommandBlocking(&_ATE0);
  

  //operateOnResponseData(&print_toDebugConsole);
 
  //send_wifiCommandBlocking(&_ATE0);
    //operateOnResponseData(&print_toDebugConsole);
   while(1);
 
}





void operateOnResponseData(ResponseOperator operation)
{
  while(ROM_UARTCharsAvail(UART1_BASE)) {
    operation(ROM_UARTCharGetNonBlocking(UART1_BASE));
  }
}
