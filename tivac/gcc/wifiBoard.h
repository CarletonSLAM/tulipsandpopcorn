
#ifndef __WIFIBOARD_H_
#define __WIFIBOARD_H_




void UART_setup_wifiBoard(void);
void wifiBoard_reset(void);
void wifiBoard_connect(char host, char port);
*char wifiBoard_send(char message, int length);
void wifiBoard_disconnect(void);
void UARTSend(uint32_t, const uint8_t*, uint32_t);

#endif /*__WIFIBOARD_H_*/