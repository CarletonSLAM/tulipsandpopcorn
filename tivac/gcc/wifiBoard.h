
#ifndef __WIFIBOARD_H_
#define __WIFIBOARD_H_
#include <stdbool.h>
#include <stdint.h>
#include "utilities.h"

#define UART_BUFFER_LENGTH 1024
typedef struct {
  uint8_t lenght;
  uint32_t timeout;
  char cmd[];
} AT_CMD;

void setup_wifi(void);
void init_debugConsole(void);
void init_wifi(void);
void send_wifiCommandBlocking(const AT_CMD *cmd);
void clear_wifiUartBuffer(void);
void wifiUARTIntHandler(void);


#endif /*__WIFIBOARD_H_*/