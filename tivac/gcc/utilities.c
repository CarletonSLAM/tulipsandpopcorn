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
#include "tulips.h"


uint16_t debugUARTIndex = 0;
char debugUARTBuffer [UART_BUFFER_LENGTH];
bool debugDelimiterEntered;
char configCharCount;
bool debugStartPrompt;


void TIVA_init()
{
  ROM_FPUEnable();
  ROM_FPULazyStackingEnable();

  //16 MHz clock
  ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

  ROM_IntMasterEnable();
}

void DEBUGCONSOLE_init()
{
  /* FOR UART SERIAL THROUGH USB
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
  ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
  ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
  */


  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  ROM_GPIOPinConfigure(GPIO_PE4_U5RX);
  ROM_GPIOPinConfigure(GPIO_PE5_U5TX);
  ROM_GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);
  ROM_UARTConfigSetExpClk(UART5_BASE, ROM_SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

  ROM_IntEnable(INT_UART5);
  ROM_UARTIntEnable(UART5_BASE, UART_INT_RX | UART_INT_RT);


  DEBUGCONSOLE_print_line("\n--------------RESET-------------\0");
  DEBUGCONSOLE_print_line("DEBUG CONSOLE: Init --> OK\0");
  debugDelimiterEntered = false;
  configCharCount=0;
  debugStartPrompt = false;
}



void DEBUGCONSOLE_print(const char *pui8Buffer)
{

   while(*pui8Buffer != '\0')
    {
        ROM_UARTCharPutNonBlocking(UART5_BASE,(uint8_t)*pui8Buffer++);
        while (ROM_UARTBusy(UART5_BASE));
    }
}

void DEBUGCONSOLE_print_line(const char *pui8Buffer)
{

   DEBUGCONSOLE_print(pui8Buffer);

    //ROM_UARTCharPutNonBlocking(UART0_BASE,(uint8_t)'\r');
    //while (ROM_UARTBusy(UART0_BASE));
    //ROM_UARTCharPutNonBlocking(UART0_BASE,(uint8_t)'\n');
    // while (ROM_UARTBusy(UART0_BASE));

    ROM_UARTCharPutNonBlocking(UART5_BASE,(uint8_t)'\r');
    while (ROM_UARTBusy(UART5_BASE));
    ROM_UARTCharPutNonBlocking(UART5_BASE,(uint8_t)'\n');
    while (ROM_UARTBusy(UART5_BASE));

}


void DEBUGCONSOLE_print_length(const char *pui8Buffer,uint32_t pui32Count)
{

   while(pui32Count -- != 0)
    {
        //ROM_UARTCharPutNonBlocking(UART0_BASE,(uint8_t)*pui8Buffer++);
        ROM_UARTCharPutNonBlocking(UART5_BASE,(uint8_t)*pui8Buffer++);
        //while (ROM_UARTBusy(UART0_BASE));
        while (ROM_UARTBusy(UART5_BASE));
    }
}



void UART_wait_forResponse(const uint32_t UARTBase)
{
  while(!ROM_UARTCharsAvail(UARTBase));
}




void TIVA_wait_miliSeconds(uint32_t ms) {
    uint16_t cnt =1001;

    while(cnt-- > 1){
        ROM_SysCtlDelay((ROM_SysCtlClockGet()/(3*1000000)*ms ));  // more accurate
    }
}

char* TIVA_int_to_String(char b[],int i){
    char const digit[] = "0123456789";
    char* p = b;
    int shifter;

    if(i<0){
        *p++ = '-';
        i *= -1;
    }

    shifter = i;
    do{ //Move to where representation ends
        p++;
        shifter = shifter/10;
    }while(shifter);

    *p = '\0';
    do{ //Move back, inserting digits as u go
      *--p = digit[i%10];
      i = i/10;
    }while(i);
    return b;
}

int16_t TIVA_find_First_Occurance_Char(char* str, char c){
    uint16_t index = 0;
    while(*(str+index)!=c){
      index++;
      if (index>UART_BUFFER_LENGTH){
        TIVA_error_encoutered("TIVA: Could not find character in buffer ERROR\0",index );
        return -1;
      }
    }
    return index;
}
void TIVA_error_encoutered(const char* strErrorMsg, const int errorNum ){
    char strErrorBuf [50];
    char strErrorNum [10];
    TIVA_int_to_String(strErrorNum,errorNum);
    strcpy(strErrorBuf,strErrorMsg);
    strcat(strErrorBuf,": ");
    strcat(strErrorBuf,strErrorNum);
    strcat(strErrorBuf,"\r\n\0");
    DEBUGCONSOLE_print_line(strErrorBuf);
    DEBUGCONSOLE_print_line("STOP PROGRAM\r\n\0");

    while(1);
}


void TIVA_vector_normalize(float_vec_t *a)
{
  float mag = sqrt(TIVA_vector_dot(a, a));
  a->x /= mag;
  a->y /= mag;
  a->z /= mag;
}

void TIVA_vector_cross_32_16(int32_vec_t *a,  int16_vec_t *b, float_vec_t *out)
{
  out->x = (a->y * b->z) - (a->z * b->y);
  out->y = (a->z * b->x) - (a->x * b->z);
  out->z = (a->x * b->y) - (a->y * b->x);
}


void TIVA_vector_cross_16_float( int16_vec_t *a,  float_vec_t *b, float_vec_t *out)
{
  out->x = (a->y * b->z) - (a->z * b->y);
  out->y = (a->z * b->x) - (a->x * b->z);
  out->z = (a->x * b->y) - (a->y * b->x);
}

 float TIVA_vector_dot( int16_vec_t *a,  int16_vec_t *b)
{
  return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}


void DEBUGCONSOLE_UART_IntHandler(void){
  uint32_t ui32Status = ROM_UARTIntStatus(UART5_BASE, true);
  ROM_UARTIntClear(UART5_BASE, ui32Status);
  char c;
  // Loop while there are characters in the receive FIFO.
  while(ROM_UARTCharsAvail(UART5_BASE))
  {
    // Read the next character from the UART and write it back to the UART.
    c = ROM_UARTCharGetNonBlocking(UART5_BASE);

    if(c == TIVA_CONFIG_CHAR ){
      configCharCount++;
      c = '\0';

    }
    else if(debugStartPrompt){
      debugUARTBuffer[debugUARTIndex++] ='~';
      debugStartPrompt = false;
    }
    else if(c == END_DELIMTER){
      debugDelimiterEntered=true;
      c = '\0';
    }
    //reset buffer to beginning
    if (debugUARTIndex++ >= UART_BUFFER_LENGTH){
      debugUARTIndex = 0;
    }
    debugUARTBuffer[debugUARTIndex] = c;
    ROM_UARTCharPutNonBlocking(UART5_BASE,c);
  }
}


void DEBUGCONSOLE_clear_UARTBuffer()
{
  for(uint16_t i =0; i< UART_BUFFER_LENGTH; i++){
    debugUARTBuffer[i] = (char) '\0';
  }
  debugUARTIndex=0;
}

void DEBUGCONSOLE_add_startDelimeter(void){
  debugStartPrompt = true;
}

char TIVA_checkForConfigMode(void){
  if(configCharCount>2){
    return 'C';
  }
  return 'N';
}


void TIVA_configureNetwork(void){

  char tempBuffer[64];
  DEBUGCONSOLE_clear_UARTBuffer();
  DEBUGCONSOLE_print("Enter desired Wifi-Network SSID (start with the tilda \'~\' sign, IF UNCHANGED enter \'~!\'): \0");
  //DEBUGCONSOLE_add_startDelimeter();
  while(!debugDelimiterEntered);
  debugDelimiterEntered = false;
  strcpy(&tempBuffer,debugUARTBuffer+TIVA_find_First_Occurance_Char(debugUARTBuffer,'~')+1);
  if(tempBuffer[0]!='!'){
  DEBUGCONSOLE_print("Network SSID captured as: \0");
    DEBUGCONSOLE_print_line(&tempBuffer);
    ROM_EEPROMProgram(&tempBuffer, 0x04, 64);
  }
  else{
    DEBUGCONSOLE_print_line("Network SSID unchanged\0");
  }

  DEBUGCONSOLE_clear_UARTBuffer();
  DEBUGCONSOLE_print("Enter desired Wifi-Network password (start with the tilda \'~\' sign, IF UNCHANGED enter \'~!\'): \0");
  //DEBUGCONSOLE_add_startDelimeter();
  while(!debugDelimiterEntered);
  debugDelimiterEntered = false;
  strcpy(&tempBuffer,debugUARTBuffer+TIVA_find_First_Occurance_Char(debugUARTBuffer,'~')+1);
  if(tempBuffer[0]!='!'){
    DEBUGCONSOLE_print("Network PW captured as: \0");
    DEBUGCONSOLE_print_line(&tempBuffer);
    ROM_EEPROMProgram(&tempBuffer, 0x48, 64 );
  }
  else{
    DEBUGCONSOLE_print_line("Network PW unchanged\0");
  }

  DEBUGCONSOLE_print("NETWORK CONFIGURATION DONE\n\0");


  DEBUGCONSOLE_clear_UARTBuffer();
  DEBUGCONSOLE_print("Enter the Host IP of server (start with the tilda \'~\' sign, IF UNCHANGED enter \'~!\'): \0");
  //DEBUGCONSOLE_add_startDelimeter();
  while(!debugDelimiterEntered);
  debugDelimiterEntered = false;
  strcpy(&tempBuffer,debugUARTBuffer+TIVA_find_First_Occurance_Char(debugUARTBuffer,'~')+1);
  if(tempBuffer[0]!='!'){
    DEBUGCONSOLE_print("Host IP captured as: \0");
    DEBUGCONSOLE_print_line(&tempBuffer);
    ROM_EEPROMProgram(&tempBuffer, 0x8C, 32 );
  }
  else{
    DEBUGCONSOLE_print_line("Host IP unchanged\0");
  }

  DEBUGCONSOLE_clear_UARTBuffer();
  //DEBUGCONSOLE_add_startDelimeter();
  DEBUGCONSOLE_print("Enter the Host Port of server (start with the tilda \'~\' sign, IF UNCHANGED enter \'~!\'): \0");
  while(!debugDelimiterEntered);
  debugDelimiterEntered = false;
  strcpy(&tempBuffer,debugUARTBuffer+TIVA_find_First_Occurance_Char(debugUARTBuffer,'~')+1);
  if(tempBuffer[0] !='!'){
    DEBUGCONSOLE_print("Host PORT captured as: \0");
    DEBUGCONSOLE_print_line(&tempBuffer);
    ROM_EEPROMProgram(&tempBuffer, 0xB4, 8 );
  }
  else{
    DEBUGCONSOLE_print_line("Host PORT unchanged\0");
  }

  DEBUGCONSOLE_clear_UARTBuffer();
  //DEBUGCONSOLE_add_startDelimeter();
  DEBUGCONSOLE_print("Clear Access Token? (\'~Y\' for yes,IF UNCHANGED enter \'~!\'): \0");
  while(!debugDelimiterEntered);
  debugDelimiterEntered = false;
  strcpy(&tempBuffer,debugUARTBuffer+TIVA_find_First_Occurance_Char(debugUARTBuffer,'~')+1);
  if(tempBuffer[0] =='Y'){
    ROM_EEPROMProgram("ACCESS_TOKEN_NOT_SET", 0xCB, 64);
    DEBUGCONSOLE_print_line("Access Token Reset\0");
  }
  else if(tempBuffer[0] =='!'){
    DEBUGCONSOLE_print_line("Access Token unchanged\0");
  }

  DEBUGCONSOLE_print_line("PLEASE RESTART DEVICE NOW\0");

}
