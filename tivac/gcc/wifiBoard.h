
#ifndef __WIFIBOARD_H_
#define __WIFIBOARD_H_
#include <stdbool.h>

bool UART_setup_wifiBoard(void);
unsigned char wifiBoard_reset(void);
void wifiBoard_connect(char host, char port);
//*char wifiBoard_send(char message, int length);
//void wifiBoard_disconnect(void);

#endif /*__WIFIBOARD_H_*/