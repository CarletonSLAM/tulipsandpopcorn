
#ifndef __IOFUNCTIONS_H_
#define __IOFUNCTIONS_H_
#include <stdbool.h>
#include <stdint.h>

#define UART_BUFFER_LENGTH 512

void UARTSend(uint32_t ui32Base, const uint8_t *pui8Buffer, uint32_t ui32Count);
//bool UARTSendEcho(uint32_t ui32Base, const uint8_t *pui8Buffer, uint32_t ui32Count);
void delay_uS(uint32_t us);
void delay_mS(uint32_t us);

void clearUARTBuffer(uint16_t* UARTIndex);


uint16_t findBrute(char* buffer, char*pattern, uint16_t searchLen, uint16_t pattLen);

#endif /*__IOFUNCTIONS_H_*/