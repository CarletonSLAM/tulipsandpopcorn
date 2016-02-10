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


char deviceMode;   //C for Config, N for Normal

char wifiSSID[32];
char wifiPass[64];
char serverIP[32];
char serverPort[8];
char serverAccessToken[64];


uint8_t EVENT_login_to_server(serverIP,serverPort){
	cJSON *tempJSON;
	tempJSON = cJSON_CreateObject();
	cJSON_AddItemToObject(tempJSON, "email", cJSON_CreateString("JaneDoe@test.com"));
	cJSON_AddItemToObject(tempJSON, "password",cJSON_CreateString("okay"));

	DEBUGCONSOLE_clear_UARTBuffer();
  if(!EVENT_connect_to_server(serverIP,serverPort))  return 0;
	if(!EVENT_send_to_server("POST","/api/Users/login",serverIP,cJSON_PrintUnformatted(tempJSON)))  return 0;
	cJSON_Delete(tempJSON);

	tempJSON = cJSON_Parse((char*)(WIFI_get_Buffer()+TIVA_find_First_Occurance_Char(WIFI_get_Buffer(),'{')));
	ROM_EEPROMProgram(cJSON_GetObjectItem(tempJSON,"id")->valuestring,0xCB, 64);
	cJSON_Delete(tempJSON);
  if(!EVENT_close_connection()) return 0;
	return 1;
	//while(1);
}

uint8_t EVENT_register_device(serverIP, serverPort){
	cJSON *tempJSON, *top;
	tempJSON = cJSON_CreateObject();


	cJSON_AddItemToObject(tempJSON, "email", cJSON_CreateString("JaneDoe@test.com"));
	cJSON_AddItemToObject(tempJSON, "password",cJSON_CreateString("okay"));

	DEBUGCONSOLE_clear_UARTBuffer();
  if(!EVENT_connect_to_server(serverIP,serverPort))  return 0;
	if(!EVENT_send_to_server("POST","/api/Users/",serverIP,cJSON_PrintUnformatted(tempJSON)))  return 0;
	cJSON_Delete(tempJSON);
  if(!EVENT_close_connection()) return 0;
	if(!EVENT_login_to_server(serverIP,serverPort))  return 0;
	// cJSON_AddItemToObject(tempJSON, "soilType", cJSON_CreateString("LOAM"));
	// cJSON_AddItemToObject(tempJSON, "cropName", cJSON_CreateString("TOMATOE"));
	// cJSON_AddItemToObject(tempJSON, "password", cJSON_CreateString("pass123"));
	// cJSON_AddItemToObject(tempJSON, "location",cJSON_CreateString("L5R2W9"));
	//
	// DEBUGCONSOLE_print_line("Registering Device\0");
	// DEBUGCONSOLE_clear_UARTBuffer();
	// if(!EVENT_connect_to_server(serverIP,serverPort))  return 0;
	// //if(!EVENT_send_to_server("GET",serverIP,"/\0","\0"))  return 0;
	// if(!EVENT_send_to_server("POST","/hello/",serverIP,cJSON_PrintUnformatted(tempJSON)))  return 0;
	// DEBUGCONSOLE_print_length(WIFI_get_Buffer(),1024);
	// cJSON_Delete(tempJSON);

	//tempJSON = cJSON_Parse((char*)(WIFI_get_Buffer()+TIVA_find_First_Occurance_Char(WIFI_get_Buffer(),'{')));

	// cJSON * top = cJSON_GetObjectItem(tempJSON,"Response");
	// ROM_EEPROMProgram(cJSON_GetObjectItem(top,"Username")->valuestring,0xCB, 64);
	//cJSON_Delete(tempJSON);

	return 1;
}


void TIVA_get_wifiSSID(char* buffer){
  ROM_EEPROMRead(buffer, 0x04, 64);
}
void TIVA_get_wifiPW(char* buffer){
  ROM_EEPROMRead(buffer, 0x48, 64);
}

void TIVA_get_serverIP(char* buffer){
  ROM_EEPROMRead(buffer, 0x8C, 32);
}
void TIVA_get_serverPort(char* buffer){
  ROM_EEPROMRead(buffer, 0xB4, 8 );
}

uint8_t TIVA_get_accessToken(char* buffer){
  ROM_EEPROMRead(buffer, 0xCB, 64);
	if(!strncmp(buffer,"ACCESS_TOKEN_NOT_SET",20)){
		if(!EVENT_register_device(serverIP,serverPort)) return 0;
		ROM_EEPROMRead(buffer, 0xCB, 64);
	}
	return 1;
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
    DEBUGCONSOLE_print_line("ENTER \'+\' REPEATEDLY TO ENTER CONFIGURATION MODE.\0");

    TIVA_wait_miliSeconds(5000);

    deviceMode = TIVA_checkForConfigMode();
    switch(deviceMode){
      case 'C':
        DEBUGCONSOLE_print_line("CONFIGURATION MODE SELECTED\0");
        TIVA_configureNetwork();
        break;

      case 'N':

				while(1){
	        DEBUGCONSOLE_print_line("NORMAL DEVICE MODE SELECTED\0");

	        TIVA_get_wifiSSID(&wifiSSID);
	        TIVA_get_wifiPW(&wifiPass);
	        TIVA_get_serverIP(&serverIP);
					TIVA_get_serverPort(&serverPort);

	        if(!EVENT_connect_to_wifi_network(wifiSSID,wifiPass))  return 0;
					if(!TIVA_get_accessToken(&serverAccessToken)) return 0;

					DEBUGCONSOLE_print_line("PARSING JSON\0");
					DEBUGCONSOLE_print_length(serverAccessToken,64);
		      break;

			}
    }
	}



    /*
    DCMOTOR_move_forward();
    COMPASS_wait_Section_Reached(4);
    DCMOTOR_stop();
    */
