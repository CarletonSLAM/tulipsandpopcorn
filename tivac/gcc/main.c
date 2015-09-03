#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "inc/hw_i2c.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/uart.h"
#include "driverlib/fpu.h"
#include "driverlib/flash.h"
#include "driverlib/pwm.h"
#include "utilities.h"
#include "wifiBoard.h"
#include "dcMotor.h"


extern void TIVA_init(void);
extern void TIVA_wait_miliSeconds(uint32_t ms);

extern void DEBUGCONSOLE_init();
extern void DEBUGCONSOLE_print(const char *pui8Buffer, uint32_t ui32Count);
extern void DEBUGCONSOLE_print_line(const char *pui8Buffer, uint32_t ui32Count);

extern void UART_wait_forResponse(const uint32_t UARTBase);

extern int16_t FLASH_write(const uint32_t pui32Address, const char *pui8Data);
extern int16_t FLASH_read(const uint32_t pui32Address, char* pui8Data);

extern void WIFI_init(void);
extern void WIFI_set_config(void);
extern void WIFI_send_commandBlocking(const AT_CMD *cmd);
extern void WIFI_clear_UARTBuffer(void);

extern void TIMER0_init(void);



extern char wifiUARTBuffer[UART_BUFFER_LENGTH];
extern uint16_t wifiUARTIndex;
extern bool wifiResetComplete;

extern uint8_t motorDuration;
extern bool  motorPowered;




typedef void (*ResponseOperator)(char dataFromHW);
void operateOnResponseData(ResponseOperator operation);
int16_t EVENT_connect_to_wifi_network();
int16_t EVENT_connect_to_server();
void EVENT_register_on_server(char* authCode);

void SERVOMOTOR_init();
void SERVOMOTOR_move(uint16_t PWMWidth);

extern void DCMOTOR_init(void);
extern void DCMOTOR_move_forward_seconds(uint16_t seconds);
extern void DCMOTOR_move_reverse_seconds(uint16_t seconds);

typedef struct {
  uint8_t Freq;
  uint32_t LoadReg;

} PWM_CONFIG;



#define PWM_FREQUENCY 46
volatile uint32_t ui32Load;
volatile uint32_t ui32PWMClock;

// 141 = No move
// 141+ = Counter-Clockwise
// 141- = Clockwise
volatile uint16_t PWMAdjust;


const AT_CMD _ATCWLAP = {10,5000, "AT+CWLAP\r\n"};
const AT_CMD _ATCWJAP = {42,10000, "AT+CWJAP=\"Davids Room\",\"P3591ng358\"\r\n"};
const AT_CMD _ATCWJAP_CUR = {11,500, "AT+CWJAP?\r\n"};

const AT_CMD _ATPING = {26, 3000, "AT+PING=\"192.168.1.1\"\r\n"};
const AT_CMD _ATCIPSTART = {41,5000, "AT+CIPSTART=4,\"TCP\",\"www.google.com\",80\r\n"};
const AT_CMD _ATCIPSEND = {17,1000,"AT+CIPSEND=4,42\r\n"};
const AT_CMD dataSend = {42,1000,"GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n\r\n"};

const uint32_t FLASH_KEY_ADDRESS = 0x0003FFEE;

char auth_code[256];

int main(void) {
  TIVA_init();
  //TIMER0_init();
  //EVENT_connect_to_wifi_network();
  SERVOMOTOR_init();
  DCMOTOR_init();
    

  
  //operateOnResponseData(&DEBUGCONSOLE_print);

  //WIFI_send_commandBlocking(&_ATE0);
  

  //operateOnResponseData(&DEBUGCONSOLE_print);
 
  //WIFI_send_commandBlocking(&_ATE0);
    //operateOnResponseData(&DEBUGCONSOLE_print); 
  /*
  while(1){
    int16_t PWMconfig = 141;
    for(;PWMconfig >130;PWMconfig--){
      TIVA_wait_miliSeconds(1000);
      SERVOMOTOR_move(PWMconfig);
    }
  
    for(;PWMconfig <141;PWMconfig++){
      TIVA_wait_miliSeconds(1000);
      SERVOMOTOR_move(PWMconfig);
    }
  
  }
  */

  while(1){

    //Turn Servo clockwise
    int16_t PWMconfig = 141;
    for(;PWMconfig >=136;PWMconfig=PWMconfig-2){
      TIVA_wait_miliSeconds(1000);
      SERVOMOTOR_move(PWMconfig);
    }
    TIVA_wait_miliSeconds(2000);
    for(;PWMconfig <=141;PWMconfig=PWMconfig+2){
      TIVA_wait_miliSeconds(1000);
      SERVOMOTOR_move(PWMconfig);
    }


    //Turn Servo Counterclockwise
    for(;PWMconfig <=146;PWMconfig=PWMconfig+2){
      TIVA_wait_miliSeconds(1000);
      SERVOMOTOR_move(PWMconfig);
    }
    TIVA_wait_miliSeconds(2000);
    
    for(;PWMconfig >=141;PWMconfig=PWMconfig-2){
      TIVA_wait_miliSeconds(1000);
      SERVOMOTOR_move(PWMconfig);
    }

    DEBUGCONSOLE_print_line("Motor:Forward",13);  
    DCMOTOR_move_forward_seconds(5);
    TIVA_wait_miliSeconds(1000);
    DEBUGCONSOLE_print_line("Motor:Reverse",13);
    DCMOTOR_move_reverse_seconds(5);
    TIVA_wait_miliSeconds(1000);
  }
 
}

int16_t EVENT_connect_to_wifi_network(){
  WIFI_set_config();
  WIFI_clear_UARTBuffer();

  DEBUGCONSOLE_print_line("Wifi:List Networks",18);  
  WIFI_send_commandBlocking(&_ATCWLAP);
  DEBUGCONSOLE_print(wifiUARTBuffer, UART_BUFFER_LENGTH);
  WIFI_clear_UARTBuffer();

  DEBUGCONSOLE_print_line("Wifi:Join Network",17);
  WIFI_send_commandBlocking(&_ATCWJAP);
  DEBUGCONSOLE_print(wifiUARTBuffer, UART_BUFFER_LENGTH);
  WIFI_clear_UARTBuffer();

  DEBUGCONSOLE_print_line("Wifi:Connected to: ", 19 );
  
  WIFI_send_commandBlocking(&_ATCWJAP_CUR);
  DEBUGCONSOLE_print(wifiUARTBuffer, UART_BUFFER_LENGTH);
  WIFI_clear_UARTBuffer();

  return 0; 
}





int16_t EVENT_connect_to_server(){

  DEBUGCONSOLE_print_line("Wifi:TCP w/ Server",18);

  WIFI_send_commandBlocking(&_ATCIPSTART);
  DEBUGCONSOLE_print(wifiUARTBuffer, UART_BUFFER_LENGTH);

  WIFI_clear_UARTBuffer();

  return 0;

}

int16_t EVENT_send_to_server(){

  DEBUGCONSOLE_print_line("Wifi:Send Command",18);

  WIFI_send_commandBlocking(&_ATCIPSEND);
  DEBUGCONSOLE_print(wifiUARTBuffer, UART_BUFFER_LENGTH);
  WIFI_clear_UARTBuffer();
  WIFI_send_commandBlocking(&dataSend);
  
  return 0; 
}


void operateOnResponseData(ResponseOperator operation)
{
  while(ROM_UARTCharsAvail(UART1_BASE)) {
    operation(ROM_UARTCharGetNonBlocking(UART1_BASE));
  }
}

void EVENT_register_on_server(char* authCode){


  if(FLASH_read(FLASH_KEY_ADDRESS,authCode)==-1){
    EVENT_connect_to_server();
  }


}



void SERVOMOTOR_init(){
  PWMAdjust = 141;

  ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_16);
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
  ROM_GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_0);
  ROM_GPIOPinConfigure(GPIO_PD0_M1PWM0);
  
  ui32PWMClock = ROM_SysCtlClockGet() / 16;
  ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
  
  ROM_PWMGenConfigure(PWM1_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
  ROM_PWMGenPeriodSet(PWM1_BASE, PWM_GEN_0, ui32Load);

  ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, PWMAdjust * ui32Load / 2000);
  ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);
  ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_0);
}

void SERVOMOTOR_move(uint16_t PWMWidth){
  ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, PWMWidth * ui32Load / 2000);
  
  }