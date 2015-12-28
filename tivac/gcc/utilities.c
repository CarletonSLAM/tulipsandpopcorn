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
  /*
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

  DEBUGCONSOLE_print_line("\n--------------RESET-------------\0");
  DEBUGCONSOLE_print_line("DEBUG CONSOLE: Init --> OK\0");
}



void DEBUGCONSOLE_print(const char *pui8Buffer)
{

   while(*pui8Buffer != '\0')
    {
        //ROM_UARTCharPut(UART0_BASE,(uint8_t)*pui8Buffer++);
        ROM_UARTCharPut(UART5_BASE,(uint8_t)*pui8Buffer++);
        //while (ROM_UARTBusy(UART0_BASE));
        while (ROM_UARTBusy(UART5_BASE));
    }
}

void DEBUGCONSOLE_print_line(const char *pui8Buffer)
{

   DEBUGCONSOLE_print(pui8Buffer);

    //ROM_UARTCharPut(UART0_BASE,(uint8_t)'\r');
    //while (ROM_UARTBusy(UART0_BASE));
    //ROM_UARTCharPut(UART0_BASE,(uint8_t)'\n');
    // while (ROM_UARTBusy(UART0_BASE));

    ROM_UARTCharPut(UART5_BASE,(uint8_t)'\r');
    while (ROM_UARTBusy(UART5_BASE));
    ROM_UARTCharPut(UART5_BASE,(uint8_t)'\n');
    while (ROM_UARTBusy(UART5_BASE));

}


void DEBUGCONSOLE_print_length(const char *pui8Buffer,uint32_t pui32Count)
{

   while(pui32Count -- != 0)
    {
        //ROM_UARTCharPut(UART0_BASE,(uint8_t)*pui8Buffer++);
        ROM_UARTCharPut(UART5_BASE,(uint8_t)*pui8Buffer++);
        //while (ROM_UARTBusy(UART0_BASE));
        while (ROM_UARTBusy(UART5_BASE));
    }
}



void UART_wait_forResponse(const uint32_t UARTBase)
{
  while(!ROM_UARTCharsAvail(UARTBase));
}

/*
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
*/


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
