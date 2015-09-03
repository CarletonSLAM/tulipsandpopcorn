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
#include "utilities.h"
#include "wifiBoard.h"

const AT_CMD _AT = {4,10,"AT\r\n"};
const AT_CMD _ATE0 = {6,10,"ATE0\r\n"};
const AT_CMD _ATRST = {8,5000,"AT+RST\r\n"};
const AT_CMD _ATCWMODE = {13,100,"AT+CWMODE=3\r\n"};
const AT_CMD _ATCWMODE_CUR = {16,100,"AT+CWMODE?\r\n"};
const AT_CMD _ATCIPMUX =  {13,500, "AT+CIPMUX=1\r\n"};

char wifiUARTBuffer[UART_BUFFER_LENGTH];
uint16_t wifiUARTIndex;
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
}

void WIFI_set_config()
{
  DEBUGCONSOLE_print_line("Wifi:List Networks",18);  
  WIFI_send_commandBlocking(&_ATRST);
  wifiResetComplete = true;
  WIFI_clear_UARTBuffer();
  WIFI_send_commandBlocking(&_ATE0);
  WIFI_send_commandBlocking(&_ATCIPMUX);
  WIFI_send_commandBlocking(&_ATCWMODE);  
}

 
void WIFI_send_commandBlocking(const AT_CMD *wificmd)
{
    for(uint8_t n =0; n < wificmd->length; n++) {
        ROM_UARTCharPut(UART1_BASE, wificmd->cmd[n]);
    }
    while (ROM_UARTBusy(UART1_BASE));

    UART_wait_forResponse(UART1_BASE);
    TIVA_wait_miliSeconds(wificmd->timeout);
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