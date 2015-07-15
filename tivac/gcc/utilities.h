
#ifndef __UTILITIES_H_
#define __UTILITIES_H_
#include <stdbool.h>
#include <stdint.h>
#include "wifiBoard.h"


#define UART_BUFFER_LENGTH 1024

void init_debugConsole(void);
void wait_forResponse(const uint32_t UARTBase);
void print_toDebugConsole(const char *pui8Buffer, uint32_t ui32Count);
void delay_miliSeconds(uint32_t ms);
void setup_tiva(void); 

#endif /*__UTILITIES_H_*/