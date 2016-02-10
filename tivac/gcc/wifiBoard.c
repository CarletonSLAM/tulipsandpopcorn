#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "hw_ints.h"
#include "hw_memmap.h"
#include "gpio.h"
#include "interrupt.h"
#include "pin_map.h"
#include "rom.h"
#include "sysctl.h"
#include "uart.h"


#include "utilities.h"
#include "wifiBoard.h"
#include "tulips.h"


extern void UART_wait_forResponse(const uint32_t UARTBase);
extern void TIVA_wait_miliSeconds(const uint32_t ms);




const AT_CMD _AT = {100,"AT\r\n\0"};
const AT_CMD _ATE0 = {100,"ATE0\r\n\0"};
const AT_CMD _ATRST = {2000,"AT+RST\r\n\0"};
const AT_CMD _ATCWMODE = {100,"AT+CWMODE=3\r\n\0"};
const AT_CMD _ATCWMODE_CUR = {100,"AT+CWMODE?\r\n\0"};
const AT_CMD _AT_CW_LAP = {5000, "AT+CWLAP\r\n\0"};
const AT_CMD _AT_CW_LAP_CUR = {500, "AT+CWJAP?\r\n\0"};
const AT_CMD _ATCIPMUX =  {500, "AT+CIPMUX=0\r\n\0"};
const AT_CMD _ATRECEIEVE = {1000,"+IPD\n\0"};
const AT_CMD _ATCIPCLOSE = {500,"AT+CIPCLOSE\r\n\0"};

const AT_CMD _AT_TEST_CWJAP = {10000, "AT+CWJAP=\"dd-wrt\",\"3ab9bcf43er\"\r\n\0"};
const AT_CMD _AT_TEST_PING = {3000, "AT+PING=\"192.168.1.1\"\r\n\0"};
const AT_CMD _AT_TEST_CIP_START = {5000, "AT+CIPSTART=\"TCP\",\"www.google.com\",80\r\n\0"};
const AT_CMD _AT_TEST_CIP_SEND = {1000,"AT+CIPSEND=42\r\n\0"};
const AT_CMD _AT_TEST_DATA = {1000,"GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n\r\n\0"};
const AT_CMD AT_TEST_SEND_DATA_HELLO = {5000,"POST /hello/soilType=LOAM&cropName=TOMATOE&password=ghid HTTP/1.0\r\nHost: 192.168.1.139\r\n\0"};


//AT+CIPSTART="TCP","www.google.com",80

const char STR_AT_CW_JAP [] = "AT+CWJAP=";
const char STR_AT_PING [] = "AT+PING=";
const char STR_AT_CIP_START [] = "AT+CIPSTART=\"TCP\",";
const char STR_AT_CIP_SEND [] = "AT+CIPSEND=";
const char HTTP_HEADER_GET [] = "\0";
const char HTTP_HEADER_POST [] = "\r\nContent-Type: application/json\r\nAccept: application/json\r\nConnection: keep-alive\r\nContent-Length: ";
const char STR_NEXT_STATE [] = "POST /viewState/username=";


char wifiUARTBuffer[UART_BUFFER_LENGTH];
uint16_t wifiUARTIndex = 0;
bool wifiResetComplete = false;

void WIFI_init(void)
{
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

  ROM_GPIOPinConfigure(GPIO_PB0_U1RX);
  ROM_GPIOPinConfigure(GPIO_PB1_U1TX);
  ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  ROM_UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet(), 9600,
                         (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

  ROM_IntEnable(INT_UART1);
  ROM_UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);

  TIVA_wait_miliSeconds(1);
  DEBUGCONSOLE_print_line("WIFI: Init --> OK\0");

}

uint8_t WIFI_set_config(void)
{
  WIFI_send_commandBlocking(&_ATRST);
  wifiResetComplete = true;
  WIFI_send_commandBlocking(&_ATE0);
  if(!WIFI_check_Ack("OK",2)){
  DEBUGCONSOLE_print_length(wifiUARTBuffer, UART_BUFFER_LENGTH);
    TIVA_error_encoutered("Wifi: ECHO Disable --> ERROR:",1);
    return 0;
  }

  WIFI_send_commandBlocking(&_ATCIPMUX);
  if(!WIFI_check_Ack("OK",2)){
    DEBUGCONSOLE_print_length(wifiUARTBuffer, UART_BUFFER_LENGTH);
    TIVA_error_encoutered("Wifi: MUX MODE Change --> ERROR:",2);
    return 0;
  }
  WIFI_send_commandBlocking(&_ATCWMODE);

  DEBUGCONSOLE_print_line("Wifi:Set Config--> OK\0");
  return 1;
}


void WIFI_send_commandBlocking(const AT_CMD *wificmd)
{
    uint16_t n =0;
    WIFI_clear_UARTBuffer();

    while(wificmd->cmd[n]!='\0'){
        ROM_UARTCharPut(UART1_BASE, wificmd->cmd[n]);
        n++;
    }
    #ifdef __VERBOSE_DEBUG_MODE__
      DEBUGCONSOLE_print_line("Command SENT!\0");
    #endif
    UART_wait_forResponse(UART1_BASE);

    TIVA_wait_miliSeconds(wificmd->timeout);


}

uint8_t WIFI_check_Ack(char* ackMsg,uint16_t ackLen){
  uint16_t i;
  for(i =0; i< UART_BUFFER_LENGTH; i++){

    if(*(wifiUARTBuffer+i) !='\0' &&!strncmp((char*)(wifiUARTBuffer+i),ackMsg,ackLen)){
      #ifdef __VERBOSE_DEBUG_MODE__
        DEBUGCONSOLE_print_line("ACK FOUND!\0");
      #endif
      return 1;
    }
  }
  return 0;
}

void WIFI_clear_UARTBuffer()
{
  for(uint16_t i =0; i< UART_BUFFER_LENGTH; i++){
    wifiUARTBuffer[i] = (char) '\0';
  }
  wifiUARTIndex=0;
}


void WIFI_UART_IntHandler(void)
{
  uint32_t ui32Status = ROM_UARTIntStatus(UART1_BASE, true);
  ROM_UARTIntClear(UART1_BASE, ui32Status);

  if (wifiResetComplete) {
      char c;
      // Loop while there are characters in the receive FIFO.
      while(ROM_UARTCharsAvail(UART1_BASE))
      {
        // Read the next character from the UART and write it back to the UART.
        c = ROM_UARTCharGetNonBlocking(UART1_BASE);
        //reset buffer to beginning
        if (wifiUARTIndex++ >= UART_BUFFER_LENGTH){
          wifiUARTIndex = 0;
        }
        wifiUARTBuffer[wifiUARTIndex] = c;
      }
  }
}



uint8_t EVENT_connect_to_wifi_network(const char* ssid, const char* pwd) {

  char tempBuf [100];
  AT_CMD JOIN_IP;

  strcpy(tempBuf,STR_AT_CW_JAP);
  strcat(tempBuf,"\"");
  strcat(tempBuf,ssid);
  strcat(tempBuf,"\",\"");
  strcat(tempBuf,pwd);
  strcat(tempBuf,"\"\r\n\0");

  JOIN_IP.timeout =6000;
  strcpy(JOIN_IP.cmd,tempBuf);


  #ifdef __VERBOSE_DEBUG_MODE__
    WIFI_send_commandBlocking(&_AT_CW_LAP);
    if(!WIFI_check_Ack("OK",2)){
      DEBUGCONSOLE_print_length(wifiUARTBuffer, UART_BUFFER_LENGTH);
      TIVA_error_encoutered("Wifi: List Networks --> ERROR:",1);
      return 0;
    }
    DEBUGCONSOLE_print_length(wifiUARTBuffer, UART_BUFFER_LENGTH);
    DEBUGCONSOLE_print_line("Wifi:List Networks --> OK\0");
  #endif
  WIFI_send_commandBlocking(&JOIN_IP);
  if(!WIFI_check_Ack("OK",2)){
    DEBUGCONSOLE_print_length(wifiUARTBuffer, UART_BUFFER_LENGTH);
    TIVA_error_encoutered("Wifi: CW Join Network --> ERROR:",2);
    return 0;
  }
  DEBUGCONSOLE_print_line("Wifi:Join Network --> OK\0");

  WIFI_send_commandBlocking(&_AT_CW_LAP_CUR);
  if(!WIFI_check_Ack("OK",2)){
    DEBUGCONSOLE_print_length(wifiUARTBuffer, UART_BUFFER_LENGTH);
    TIVA_error_encoutered("Wifi: Connected Network --> ERROR:",3);
    return 0;
  }
  DEBUGCONSOLE_print("Wifi:Connected Network --> \0");
  DEBUGCONSOLE_print_line(wifiUARTBuffer+TIVA_find_First_Occurance_Char(wifiUARTBuffer,'\"')+1);
  WIFI_clear_UARTBuffer();

  return 1;
}





uint8_t EVENT_connect_to_server(const char *host, const char *port){
  char tempBuf [100];
  AT_CMD START_CONN;
  strcpy(tempBuf,STR_AT_CIP_START);
  strcat(tempBuf,"\"");
  strcat(tempBuf,host);
  strcat(tempBuf,"\",");
  strcat(tempBuf,port);
  strcat(tempBuf,"\r\n\0");

  START_CONN.timeout =5000;
  strcpy(START_CONN.cmd,tempBuf);
  WIFI_clear_UARTBuffer();
  WIFI_send_commandBlocking(&START_CONN);
  if(!WIFI_check_Ack("Linked",6)){
    DEBUGCONSOLE_print_length(wifiUARTBuffer, UART_BUFFER_LENGTH);
    TIVA_error_encoutered("Wifi: TCP Server Link --> ERROR:",1);
    return 0;
  }
  DEBUGCONSOLE_print_line("Wifi:TCP Server Link --> OK\0");
  WIFI_clear_UARTBuffer();
  return 1;
}

uint8_t EVENT_send_to_server(const char *httpMethod,const char *httpCmd, const char* host, char* data){


  char cmdLenBuf[10];
  char tempBuf [1024];
  char *tempPtr;
  int16_t tempLength=0, cmdLength=0,hostLength=0,dataLength=0,headerLength=0;
  AT_CMD SEND_CONN,SEND_DATA;

  while(httpMethod[tempLength]!='\0'){
    tempLength++;
  }

  while(httpCmd[cmdLength]!='\0'){
    cmdLength++;
  }
  while(host[hostLength]!='\0'){
    hostLength++;
  }

  strcpy(tempBuf,httpMethod);
  strcat(tempBuf," ");
  strcat(tempBuf,httpCmd);
  tempLength+=cmdLength+1;
  strcat(tempBuf," HTTP/1.1\r\nHost: ");
  tempLength+=17;
  strcat(tempBuf,host);
  tempLength+=hostLength;

  if(!strncmp(httpMethod,"GET",3)){
    DEBUGCONSOLE_print_line("GET Method Called\0");
    while(HTTP_HEADER_GET[headerLength]!='\0'){
      headerLength++;
    }
    strcat(tempBuf,HTTP_HEADER_GET);
    tempLength+=headerLength;
  }
  else if(!strncmp(httpMethod,"POST",4)){
    DEBUGCONSOLE_print_line("POST Method Called\0");
    while(HTTP_HEADER_POST[headerLength]!='\0'){
      headerLength++;
    }

    while(data[dataLength]!='\0'){
      dataLength++;
    }

    strcat(tempBuf,HTTP_HEADER_POST);
    tempLength+=headerLength;

    strcat(tempBuf,TIVA_int_to_String(cmdLenBuf,dataLength));
    tempLength+=strlen(TIVA_int_to_String(cmdLenBuf,dataLength));

    strcat(tempBuf,"\r\n\r\n");
    tempLength+=4;

    strcat(tempBuf,data);
    tempLength+= dataLength;
  }
  else{
    TIVA_error_encoutered("Wifi:Unknown/Unimplemented HTTP Method Call--> ERROR:",1);
    return 0;
  }
  strcat(tempBuf,"\r\n\r\n");
  tempLength+=4;
  SEND_DATA.timeout =5000;
  strcpy(SEND_DATA.cmd,tempBuf);

  strcpy(tempBuf,STR_AT_CIP_SEND);
  strcat(tempBuf,TIVA_int_to_String(cmdLenBuf,tempLength));
  strcat(tempBuf,"\r\n");
  SEND_CONN.timeout =500;
  strcpy(SEND_CONN.cmd,tempBuf);

  WIFI_clear_UARTBuffer();
  WIFI_send_commandBlocking(&SEND_CONN);

  DEBUGCONSOLE_print_length(wifiUARTBuffer, UART_BUFFER_LENGTH);
  if(!WIFI_check_Ack(">",1)){
    DEBUGCONSOLE_print_length(wifiUARTBuffer, UART_BUFFER_LENGTH);
    TIVA_error_encoutered("Wifi:Send Length--> ERROR:",2);
    return 0;
  }
  DEBUGCONSOLE_print_line(SEND_CONN.cmd);
  DEBUGCONSOLE_print_line("Wifi:Send Length --> OK\0");


  DEBUGCONSOLE_print_line(SEND_DATA.cmd);

  WIFI_send_commandBlocking(&SEND_DATA);

  if(!WIFI_check_Ack("OK",2)){
    DEBUGCONSOLE_print_length(wifiUARTBuffer, UART_BUFFER_LENGTH);
    TIVA_error_encoutered("Wifi:Send Command--> ERROR:",3);
    return 0;
  }
  DEBUGCONSOLE_print_line("Wifi:Send Command --> OK\0");
  return 1;

}


char* WIFI_get_Buffer(void){
  return &wifiUARTBuffer;
}

uint8_t EVENT_close_connection(void){
  WIFI_clear_UARTBuffer();
  WIFI_send_commandBlocking(&_ATCIPCLOSE);
  if(!WIFI_check_Ack("Unlink",6)){
    DEBUGCONSOLE_print_length(wifiUARTBuffer, UART_BUFFER_LENGTH);
    TIVA_error_encoutered("Wifi:TCP Conn Clos--> ERROR:",3);
    return 0;
  }
  DEBUGCONSOLE_print_line("Wifi:TCP Conn Close --> OK\0");
  WIFI_clear_UARTBuffer();
  return 1;
}
