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


#include "cJSON.h"



//----------------------------- CONSTANTS-------------------------------------------------//
//----------------------------------------------------------------------------------------//



//------------------------- FUNCTION PROTOTYPES ------------------------------------------//
//----------------------------------------------------------------------------------------//

extern int16_vec_t AccellData, MagnData;
extern bool debugDelimiterEntered;
extern char debugUARTBuffer [UART_BUFFER_LENGTH];
extern char configCharCount;


char deviceMode;   //C for Config, N for Norma

char wifiSSID[32];
char wifiPass[64];
char serverIP[32]="api.openweathermap.org";
char serverPort[8]="80";


/* Parse text to JSON, then render back to text, and print! */
void doit(char *text)
{
	char *out;
  cJSON *json;

  char* jsonStringValue;

  json=cJSON_Parse(text);
	if (!json) {printf("Error before: [%s]\n",cJSON_GetErrorPtr());}
	else
	{
		out=cJSON_Print(json);

		DEBUGCONSOLE_print_line(out);
  	DEBUGCONSOLE_print_line(&(cJSON_GetObjectItem(json,"dict")->valuestring));
    DEBUGCONSOLE_print_line(&(cJSON_GetObjectItem(json,"next_state")->valuestring));
    DEBUGCONSOLE_print_line(&(cJSON_GetObjectItem(json,"username")->valuestring));

		cJSON_Delete(json);
		free(out);
	}
}

void TIVA_get_wifiSSID(char* buffer){
  ROM_EEPROMRead(buffer, 0x0E, 32);
}
void TIVA_get_wifiPW(char* buffer){
  ROM_EEPROMRead(buffer, 0x19, 64);
}

void TIVA_get_serverIP(char* buffer){
  ROM_EEPROMRead(buffer, 0x29, 32);
}
void TIVA_get_serverPort(char* buffer){
  ROM_EEPROMRead(buffer, 0x3B, 8 );
}

void TIVA_EEPROM_init(void){

  uint32_t tempReturn;
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
  do
   {
       tempReturn=ROM_EEPROMInit(); // EEPROM start
   }
   while(tempReturn!=EEPROM_INIT_OK);

}

int main(void){

    TIVA_init();
    TIVA_EEPROM_init();
    DEBUGCONSOLE_init();
    TIVA_wait_miliSeconds(500);
    //TIVA_Timer_setup();
    //DCMOTOR_init();
    //if(!COMPASS_init()){  return 0; }
    WIFI_init();
    WIFI_set_config();
    DEBUGCONSOLE_print("ENTER \'+\' REPEATEDLY TO ENTER CONFIGURATION MODE.\n\0");

    TIVA_wait_miliSeconds(7000);

    deviceMode = TIVA_checkForConfigMode();
    switch(deviceMode){
      case 'C':
        DEBUGCONSOLE_print("CONFIGURATION MODE SELECTED\n\0");
        TIVA_configureNetwork();
        break;

      case 'N':
        DEBUGCONSOLE_print("NORMAL DEVICE MODE SELECTED\n\0");
        TIVA_get_wifiSSID(&wifiSSID);
        TIVA_get_wifiPW(&wifiPass);
        //TIVA_get_serverIP(&serverIP);
        //TIVA_get_serverPort(&serverPort);

        if(!EVENT_connect_to_wifi_network(wifiSSID,wifiPass))  return 0;
        if(!EVENT_connect_to_server(serverIP,serverPort))  return 0;
        //if(!EVENT_send_to_server(serverIP,"POST /hello/soilType=LOAM&cropName=TOMATOE&password=ghid\0"))  return 0;
        if(!EVENT_send_to_server(serverIP,"GET /data/2.5/weather?q=Ottawa,ca&appid=2de143494c0b295cca9337e1e96b00e0\0")) return 0;
        DEBUGCONSOLE_print_length(WIFI_get_Buffer(),1024);
        doit((char*)(WIFI_get_Buffer()+TIVA_find_First_Occurance_Char(WIFI_get_Buffer(),'{')));
        break;

    }



    /*
    DCMOTOR_move_forward();
    COMPASS_wait_Section_Reached(4);
    DCMOTOR_stop();
    */

}
