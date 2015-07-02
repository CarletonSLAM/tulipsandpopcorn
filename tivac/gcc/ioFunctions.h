
#ifndef __IOFUNCTIONS_H_
#define __IOFUNCTIONS_H_
#include <stdbool.h>
#include <stdint.h>

void UARTSend(uint32_t ui32Base, const uint8_t *pui8Buffer, uint32_t ui32Count);
bool UARTSendEcho(uint32_t ui32Base, const uint8_t *pui8Buffer, uint32_t ui32Count);
void delay_uS(uint32_t us);
;
#endif /*__IOFUNCTIONS_H_*/