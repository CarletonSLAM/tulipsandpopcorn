
#ifndef __WIFIBOARD_H_
#define __WIFIBOARD_H_
#include <stdbool.h>
#include <stdint.h>
#include "utilities.h"

#define UART_BUFFER_LENGTH 1024
typedef struct {
  uint8_t length;
  uint32_t timeout;
  char cmd[];
} AT_CMD;

void WIFI_init(void);
void WIFI_set_config(void);
void WIFI_send_commandBlocking(const AT_CMD *cmd);
void WIFI_clear_UARTBuffer(void);
void WIFI_UART_IntHandler(void);


#endif /*__WIFIBOARD_H_*/