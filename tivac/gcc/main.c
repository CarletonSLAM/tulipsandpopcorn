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



//----------------------------- CONSTANTS-------------------------------------------------//
//----------------------------------------------------------------------------------------//


//------------------------- FUNCTION PROTOTYPES ------------------------------------------//
//----------------------------------------------------------------------------------------//

extern int16_vec_t AccellData, MagnData;



int main(void){
    //uint8_t response ;


    int16_t heading;
    uint8_t singCharBuf;
    char strBuf[15];
    int index;

    int16_t pitch,roll, TILT_COMP_MX,TILT_COMP_MY, TILT_COMP_MZ;
    TIVA_init();
    TIVA_wait_miliSeconds(5000);
    DEBUGCONSOLE_init();
    TIVA_Timer_setup();
    DCMOTOR_init();
    if(!COMPASS_init()){  return 0; }

    TIVA_wait_miliSeconds(2000);
    DCMOTOR_move_forward();
    COMPASS_wait_Section_Reached(4);
    DCMOTOR_stop();


}
