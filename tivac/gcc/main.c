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
#include "rom.h"
#include "sysctl.h"
#include "eeprom.h"
#include "systick.h"

//Custom Libraries
#include "utilities.h"
#include "compass.h"
#include "tulips.h"
#include "dcMotor.h"
#include "timers.h"
#include "wifiBoard.h"
#include "tulips.h"

#include "cJSON.h"



//----------------------------- CONSTANTS-------------------------------------------------//
//----------------------------------------------------------------------------------------//

#define __VERBOSE_DEBUG_MODE__

//------------------------- FUNCTION PROTOTYPES ------------------------------------------//
//----------------------------------------------------------------------------------------//

extern int16_vec_t AccellData, MagnData;
extern bool debugDelimiterEntered;
extern char debugUARTBuffer [UART_BUFFER_LENGTH];
extern char configCharCount;


char deviceMode;   //C for Config, N for Normal

char wifiSSID[32];
char wifiPass[64];
char serverIP[32] = "192.168.1.139";
char serverPort[8] = "3000";
char serverAccessUserToken[40];
char serverAccessPWToken[10];


uint8_t EVENT_get_next_action(char* serverIP,char* serverPort){
	cJSON *tempJSON;
	tempJSON = cJSON_CreateObject();
	cJSON_AddItemToObject(tempJSON, "username", cJSON_CreateString(serverAccessUserToken));
	cJSON_AddItemToObject(tempJSON, "password",cJSON_CreateString("pass123"));

	DEBUGCONSOLE_clear_UARTBuffer();
  if(!EVENT_connect_to_server(serverIP,serverPort))  return 0;
	if(!EVENT_send_to_server("POST","/viewState/",serverIP,cJSON_PrintUnformatted(tempJSON)))  return 0;
	DEBUGCONSOLE_print_length(WIFI_get_Buffer(),1024);

	tempJSON = cJSON_Parse((char*)(WIFI_get_Buffer()+TIVA_find_First_Occurance_Char(WIFI_get_Buffer(),'{')));

	cJSON_Delete(tempJSON);
  if(!EVENT_close_connection()) return 0;
	return 1;
}


uint8_t EVENT_send_moisture_data(char* serverIP,char* serverPort){
	cJSON *tempJSON,*innerJSON1,*innerJSON2,*innerJSON3;

	tempJSON = cJSON_CreateObject();
	cJSON_AddItemToObject(tempJSON, "username", cJSON_CreateString(serverAccessUserToken));
	cJSON_AddItemToObject(tempJSON, "password",cJSON_CreateString(serverAccessPWToken));
	innerJSON1	= cJSON_CreateObject();
	cJSON_AddItemToObject(innerJSON1, "aaa",cJSON_CreateNumber(0.56));
	cJSON_AddItemToObject(innerJSON1, "bbb",cJSON_CreateNumber(0.32));
	cJSON_AddItemToObject(innerJSON1, "ccc",cJSON_CreateNumber(0.12));
	cJSON_AddItemToObject(tempJSON, "sector1",innerJSON1);
	innerJSON2	= cJSON_CreateObject();
	cJSON_AddItemToObject(innerJSON2, "aaa",cJSON_CreateNumber(0.11));
	cJSON_AddItemToObject(innerJSON2, "bbb",cJSON_CreateNumber(0.23));
	cJSON_AddItemToObject(innerJSON2, "ccc",cJSON_CreateNumber(0.54));
	cJSON_AddItemToObject(tempJSON, "sector2",innerJSON2);
	innerJSON3	= cJSON_CreateObject();
	cJSON_AddItemToObject(innerJSON3, "aaa",cJSON_CreateNumber(0.88));
	cJSON_AddItemToObject(innerJSON3, "bbb",cJSON_CreateNumber(0.22));
	cJSON_AddItemToObject(innerJSON3, "ccc",cJSON_CreateNumber(0.09));
	cJSON_AddItemToObject(tempJSON, "sector3",innerJSON3);
	DEBUGCONSOLE_clear_UARTBuffer();
  if(!EVENT_connect_to_server(serverIP,serverPort))  return 0;

	if(!EVENT_send_to_server("POST","/giveMoistData/",serverIP,cJSON_PrintUnformatted(tempJSON)))  return 0;
	DEBUGCONSOLE_print_length(WIFI_get_Buffer(),1024);

	//tempJSON = cJSON_Parse((char*)(WIFI_get_Buffer()+TIVA_find_First_Occurance_Char(WIFI_get_Buffer(),'{')));

	cJSON_Delete(innerJSON1);
	cJSON_Delete(innerJSON2);
	cJSON_Delete(innerJSON3);
	cJSON_Delete(tempJSON);
  if(!EVENT_close_connection()) return 0;
	return 1;
}

uint8_t EVENT_register_device(char* serverIP,char* serverPort){
	char tempBuf [10]="test1302";
	uint8_t nowTicks;
	if(!strncmp(serverAccessUserToken,"ACCESS_TOKEN_NOT_SET",20)){
		cJSON *tempJSON, *top;
		DEBUGCONSOLE_print_line("Registering Device on the server\0");
		ROM_EEPROMProgram(tempBuf, MEMORY_ACCESS_PW_LOCACTION, 12);
		strcpy(serverAccessPWToken,tempBuf);
		DEBUGCONSOLE_print_line(tempBuf);
		tempJSON = cJSON_CreateObject();
		cJSON_AddItemToObject(tempJSON, "soilType", cJSON_CreateString("LOAM"));
		cJSON_AddItemToObject(tempJSON, "cropName", cJSON_CreateString("TOMATOE"));
		cJSON_AddItemToObject(tempJSON, "password", cJSON_CreateString(serverAccessPWToken));
		cJSON_AddItemToObject(tempJSON, "location",cJSON_CreateString("L5R2W9"));

		DEBUGCONSOLE_clear_UARTBuffer();
		if(!EVENT_connect_to_server(serverIP,serverPort))  return 0;
		if(!EVENT_send_to_server("POST","/hello/",serverIP,cJSON_PrintUnformatted(tempJSON)))  return 0;
		DEBUGCONSOLE_print_length(WIFI_get_Buffer(),1024);
		cJSON_Delete(tempJSON);

		tempJSON = cJSON_Parse((char*)(WIFI_get_Buffer()+TIVA_find_First_Occurance_Char(WIFI_get_Buffer(),'{')));

		top = cJSON_GetObjectItem(tempJSON,"Response");
		ROM_EEPROMProgram(cJSON_GetObjectItem(top,"Username")->valuestring,MEMORY_ACCESS_USER_LOCACTION, 40);
		cJSON_Delete(top);
		cJSON_Delete(tempJSON);
		if(!EVENT_close_connection()) return 0;

		return 1;
	}
	else{
		DEBUGCONSOLE_print_line("Device already registered on the server\0");
		return 1;

	}
}


void TIVA_get_wifiSSID(char* buffer){
  ROM_EEPROMRead(buffer, MEMORY_WIFI_SSID_LOCATION, 64);
}
void TIVA_get_wifiPW(char* buffer){
  ROM_EEPROMRead(buffer, MEMORY_WIFI_PW_LOCATION, 64);
}

void TIVA_get_accessUserToken(char* buffer){
  ROM_EEPROMRead(buffer, MEMORY_ACCESS_USER_LOCACTION, 40);
}

void TIVA_get_accessPWToken(char* buffer){
  ROM_EEPROMRead(buffer, MEMORY_ACCESS_PW_LOCACTION, 12);
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
					TIVA_get_accessUserToken(&serverAccessUserToken);
					TIVA_get_accessPWToken(&serverAccessPWToken);

	        if(!EVENT_connect_to_wifi_network(wifiSSID,wifiPass))  return 0;
					if(!EVENT_register_device(serverIP,serverPort)) return 0;
					DEBUGCONSOLE_print_line("PARSING JSON\0");
					DEBUGCONSOLE_print_length(serverAccessUserToken,37);
					if(!EVENT_get_next_action(serverIP,serverPort))	return 0;
					DEBUGCONSOLE_print_line("DONE DELEGATOR\0");
					if(!EVENT_send_moisture_data(serverIP,serverPort))	return 0;
					DEBUGCONSOLE_print_line("DONE MOISTURE\0");
		      while(1);

			}
    }
	}



    /*
    DCMOTOR_move_forward();
    COMPASS_wait_Section_Reached(4);
    DCMOTOR_stop();
    */
