
#ifndef __WIFIBOARD_H_
#define __WIFIBOARD_H_
#include <stdbool.h>
#include <stdint.h>
#include "utilities.h"

#define UART_BUFFER_LENGTH 1024
typedef struct {
  uint32_t timeout;
  char cmd[512];
} AT_CMD;

void WIFI_init(void);
uint8_t WIFI_set_config(void);
void WIFI_send_commandBlocking(const AT_CMD *cmd);
uint8_t WIFI_check_Ack(char* ackMsg, uint16_t ackLen);
void WIFI_clear_UARTBuffer(void);
void WIFI_UART_IntHandler(void);



uint8_t EVENT_connect_to_wifi_network(const char* ssid, const char* pwd);
uint8_t EVENT_connect_to_server(const char *host, const char *port);
uint8_t EVENT_send_to_server(const char *host,char* cmd);

char* WIFI_get_Buffer(void);
uint8_t EVENT_close_connection(void);


#endif /*__WIFIBOARD_H_*/
