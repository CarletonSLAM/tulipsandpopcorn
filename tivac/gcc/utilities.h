
#ifndef __UTILITIES_H_
#define __UTILITIES_H_
#include <stdbool.h>
#include <stdint.h>
#include "wifiBoard.h"


#define UART_BUFFER_LENGTH 1024

void DEBUGCONSOLE_init(void);
void DEBUGCONSOLE_print(const char *pui8Buffer, uint32_t ui32Count);
void DEBUGCONSOLE_print_line(const char *pui8Buffer, uint32_t ui32Count);

int16_t FLASH_write(const uint32_t pui32Address, const char *pui8Data);
int16_t FLASH_read(const uint32_t pui32Address, char* pui8Data);

void TIVA_wait_miliSeconds(uint32_t ms);
void TIVA_init(void); 


void TIMER0_init(void);

#endif /*__UTILITIES_H_*/