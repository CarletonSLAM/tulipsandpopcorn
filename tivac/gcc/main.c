//------------------------------- INCLUDES -----------------------------------------------//
//----------------------------------------------------------------------------------------//
//Standard Libraries
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>

//TIVA Drivelib
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_gpio.h"
#include "timer.h"

//Custom Libraries
#include "utilities.h"
#include "compass.h"
#include "tulips.h"
#include "dcMotor.h"
#include "timers.h"
#include "wifiBoard.h"


#include "rom.h"
#include "sysctl.h"
#include "eeprom.h"



//----------------------------- CONSTANTS-------------------------------------------------//
//----------------------------------------------------------------------------------------//


const char WIFI_HOST []= "192.168.1.139";//"192.168.1.139";
const char WIFI_PORT []= "8000";

//------------------------- FUNCTION PROTOTYPES ------------------------------------------//
//----------------------------------------------------------------------------------------//

extern int16_vec_t AccellData, MagnData;
extern bool debugDelimiterEntered;
extern char debugUARTBuffer [UART_BUFFER_LENGTH];
extern char configCharCount;


char deviceMode;   //C for Config, N for Normal
char wifiSSID[32];
char wifiPass[64];
char serverIP[32]="https://slack.com/api";
char serverPort[8]="80";

void TIVA_checkForConfigMode(void){
  if(configCharCount>2){
    deviceMode = 'C';
  }
  else{
    deviceMode = 'N';
  }
}


void TIVA_configureNetwork(void){

  char tempBuffer[64];
  DEBUGCONSOLE_print("CONFIGURATION MODE SELECTED\n\0");

  DEBUGCONSOLE_clear_UARTBuffer();
  DEBUGCONSOLE_print("Enter desired Wifi-Network SSID (start with the tilda \'~\' sign, IF UNCHANGED enter \'~!\'): \0");
  //DEBUGCONSOLE_add_startDelimeter();
  while(!debugDelimiterEntered);
  debugDelimiterEntered = false;
  strcpy(&tempBuffer,debugUARTBuffer+TIVA_find_First_Occurance_Char(debugUARTBuffer,'~')+1);
  if(tempBuffer[0]!='!'){
  DEBUGCONSOLE_print("Network SSID captured as: \0");
    DEBUGCONSOLE_print_line(&tempBuffer);
    ROM_EEPROMProgram(&tempBuffer, 0x0E, 32 );
  }
  else{
    DEBUGCONSOLE_print_line("Network SSID unchanged\0");
  }

  DEBUGCONSOLE_clear_UARTBuffer();
  DEBUGCONSOLE_print("Enter desired Wifi-Network password (start with the tilda \'~\' sign, IF UNCHANGED enter \'~!\'): \0");
  //DEBUGCONSOLE_add_startDelimeter();
  while(!debugDelimiterEntered);
  debugDelimiterEntered = false;
  strcpy(&tempBuffer,debugUARTBuffer+TIVA_find_First_Occurance_Char(debugUARTBuffer,'~')+1);
  if(tempBuffer[0]!='!'){
    DEBUGCONSOLE_print("Network PW captured as: \0");
    DEBUGCONSOLE_print_line(&tempBuffer);
    ROM_EEPROMProgram(&tempBuffer, 0x19, 64 );
  }
  else{
    DEBUGCONSOLE_print_line("Network PW unchanged\0");
  }

  DEBUGCONSOLE_print("NETWORK CONFIGURATION DONE\n\0");


  DEBUGCONSOLE_clear_UARTBuffer();
  DEBUGCONSOLE_print("Enter the Host IP of server (start with the tilda \'~\' sign, IF UNCHANGED enter \'~!\'): \0");
  //DEBUGCONSOLE_add_startDelimeter();
  while(!debugDelimiterEntered);
  debugDelimiterEntered = false;
  strcpy(&tempBuffer,debugUARTBuffer+TIVA_find_First_Occurance_Char(debugUARTBuffer,'~')+1);
  if(tempBuffer[0]!='!'){
    DEBUGCONSOLE_print("Host IP captured as: \0");
    DEBUGCONSOLE_print_line(&tempBuffer);
    ROM_EEPROMProgram(&tempBuffer, 0x29, 32 );
  }
  else{
    DEBUGCONSOLE_print_line("Host IP unchanged\0");
  }

  DEBUGCONSOLE_clear_UARTBuffer();
  //DEBUGCONSOLE_add_startDelimeter();
  DEBUGCONSOLE_print("Enter the Host Port of server (start with the tilda \'~\' sign, IF UNCHANGED enter \'~!\'): \0");
  while(!debugDelimiterEntered);
  debugDelimiterEntered = false;
  strcpy(&tempBuffer,debugUARTBuffer+TIVA_find_First_Occurance_Char(debugUARTBuffer,'~')+1);
  if(tempBuffer[0] !='!'){
    DEBUGCONSOLE_print("Host PORT captured as: \0");
    DEBUGCONSOLE_print_line(&tempBuffer);
    ROM_EEPROMProgram(&tempBuffer, 0x3B, 8 );
  }
  else{
    DEBUGCONSOLE_print_line("Host PORT unchanged\0");
  }

  DEBUGCONSOLE_print_line("PLEASE RESTART DEVICE NOW\0");

}
int main(void){

    uint32_t tempReturn;
    TIVA_init();
    TIVA_wait_miliSeconds(5000);
    DEBUGCONSOLE_init();
    //TIVA_Timer_setup();
    //DCMOTOR_init();
    //if(!COMPASS_init()){  return 0; }
    WIFI_init();
    WIFI_set_config();
    DEBUGCONSOLE_print("ENTER \'+\' REPEATEDLY TO ENTER CONFIGURATION MODE.\n\0");

    TIVA_wait_miliSeconds(5000);
    TIVA_checkForConfigMode();

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    do
     {
         tempReturn=ROM_EEPROMInit(); // EEPROM start
     }
     while(tempReturn!=EEPROM_INIT_OK);


    switch(deviceMode){
      case 'C':
        TIVA_configureNetwork();


        break;

      case 'N':
        DEBUGCONSOLE_print("NORMAL DEVICE MODE SELECTED\n\0");
        ROM_EEPROMRead(&wifiSSID, 0x0E, 32);
        ROM_EEPROMRead(&wifiPass, 0x19, 64);
        //ROM_EEPROMRead(&serverIP, 0x29, 32);
        //ROM_EEPROMRead(&serverPort, 0x3B, 8 );

        if(!EVENT_connect_to_wifi_network(wifiSSID,wifiPass))  return 0;
        if(!EVENT_connect_to_server(serverIP,serverPort))  return 0;
        //if(!EVENT_send_to_server(serverIP,"POST /hello/soilType=LOAM&cropName=TOMATOE&password=ghid\0"))  return 0;
        if(!EVENT_send_to_server(serverIP,"GET /api.test\0")) return 0;
        break;

    }



    /*
    DCMOTOR_move_forward();
    COMPASS_wait_Section_Reached(4);
    DCMOTOR_stop();
    */

}
