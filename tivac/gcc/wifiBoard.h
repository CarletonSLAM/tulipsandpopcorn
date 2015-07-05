
#ifndef __WIFIBOARD_H_
#define __WIFIBOARD_H_
#include <stdbool.h>
#include <stdint.h>
#include "ioFunctions.h"

extern char wifiUARTBuffer[UART_BUFFER_LENGTH];
extern uint16_t wifiUARTIndex;
extern char wifiNetworks[20][50];

bool UART_setup_wifiBoard(void);
bool wifiBoard_reset(void);

bool wifiBoard_setNetworkMode(void);

void wifiBoard_listNetworks(void);
void wifiBoard_connect(char host, char port);

bool wifiBoard_sendCommand(uint32_t wifiBase, const uint8_t* wifiCommand, const uint16_t commandLen,  const char* wifiResponse, uint16_t searchLength, uint32_t commTimeoutmS);

//*char wifiBoard_send(char message, int length);
//void wifiBoard_disconnect(void);

#endif /*__WIFIBOARD_H_*/