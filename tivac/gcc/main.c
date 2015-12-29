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



//----------------------------- CONSTANTS-------------------------------------------------//
//----------------------------------------------------------------------------------------//

const char WIFI_SSID []= "dd-wrt";
const char WIFI_PWD []= "*";
const char WIFI_HOST []= "192.168.1.139";//"192.168.1.139";
const char WIFI_PORT []= "8000";


//------------------------- FUNCTION PROTOTYPES ------------------------------------------//
//----------------------------------------------------------------------------------------//

extern int16_vec_t AccellData, MagnData;



int main(void){


    TIVA_init();
    TIVA_wait_miliSeconds(5000);
    DEBUGCONSOLE_init();
    //TIVA_Timer_setup();
    //DCMOTOR_init();
    //if(!COMPASS_init()){  return 0; }

    TIVA_wait_miliSeconds(2000);

    WIFI_init();
    WIFI_set_config();


    if(!EVENT_connect_to_wifi_network(WIFI_SSID,WIFI_PWD))  return 0;


    if(!EVENT_connect_to_server(WIFI_HOST,WIFI_PORT))  return 0;


    if(!EVENT_send_to_server(WIFI_HOST,"POST /hello/soilType=LOAM&cropName=TOMATOE&password=ghid\0"))  return 0;


    /*
    DCMOTOR_move_forward();
    COMPASS_wait_Section_Reached(4);
    DCMOTOR_stop();
    */

}
