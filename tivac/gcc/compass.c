//Standard Libraries
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <malloc.h>

//TIVA Drivelib
#include "gpio.h"
#include "pin_map.h"
#include "rom.h"
#include "sysctl.h"
#include "i2c.h"
#include "hw_i2c.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_gpio.h"

//Custom Libraries
#include "utilities.h"
#include "i2cFunc.h"
#include "compass.h"
#include "tulips.h"

//----------------------------- CONSTANTS-------------------------------------------------//
//----------------------------------------------------------------------------------------//
const i2c_port I2CChannel1 = {SYSCTL_PERIPH_I2C1,   //periphComp
                        SYSCTL_PERIPH_GPIOA,  //periphPort
                        I2C1_BASE,             //i2cBase
                        GPIO_PORTA_BASE,      //i2cPortBase
                        GPIO_PA6_I2C1SCL,     //sclPort
                        GPIO_PIN_6,           //sclPin
                        GPIO_PA7_I2C1SDA,     //sdaPort
                        GPIO_PIN_7            //sdaPin
                        };

const uint8_t COMPASS_CMD_CTRL1_MODE [2]= {0x20,0x57};
const uint8_t COMPASS_CMD_CTRL2_MODE [2]= {0x21, 0x00};
const uint8_t COMPASS_CMD_CTRL5_MODE [2]= {0x24, 0x64};
const uint8_t COMPASS_CMD_CTRL6_MODE [2]= {0x25, 0x20};
const uint8_t COMPASS_CMD_CTRL7_MODE [2]= {0x26, 0x00};

const int16_vec_t COMPASS_ACCELL_CALIB_MIN = {-24880,-24618,-26749};
const int16_vec_t COMPASS_ACCELL_CALIB_MAX = {16049,32760,32760};
const int16_vec_t COMPASS_MAGN_CALIB_MIN = {-3997,-3308,-2583};
const int16_vec_t COMPASS_MAGN_CALIB_MAX = {3541,3763,4216};
const int16_vec_t COMPASS_BODY_VEC = {1, 0, 0};

//MIN ACC: X = -24880	Y = -24618	Z = -22066 MAX : X = 16049	Y = 32760	Z = 32760
//MIN MAG: X = -3997	Y = -3138	Z = -2403 MAX MAG : X = 3020	Y = 3763	Z = 4216



int16_vec_t AccellData, MagnData;

//int16_t AccellData.x, AccellData.y, AccellData.z, MagnData.x, MagnData.y, MagnData.z,
int16_t CompassHeading;
uint8_t CompassSection;
uint8_t CompassSectionDegree;
uint8_t CompassSectionSafe;



uint8_t COMPASS_init(void){
  uint8_t tempBuffer;
  char tempStr [10];
  uint32_t I2Cstatus;




  I2C_setup(&I2CChannel1);
  DEBUGCONSOLE_print_line("I2C: Init --> OK\0");

  I2Cstatus = I2C_single_write_blocking(&I2CChannel1,COMPASS_ADDR, COMPASS_CMD_WHO_AM_I);
  if( I2Cstatus != I2C_MASTER_ERR_NONE){
      TIVA_error_encoutered("I2C: Compass --> WHO_AM_I Write ERROR:", I2Cstatus);
      return 0;
  }

  I2Cstatus = I2C_single_read_blocking(&I2CChannel1,COMPASS_ADDR,&tempBuffer);
  if( I2Cstatus != I2C_MASTER_ERR_NONE){
      TIVA_error_encoutered("I2C: Compass --> WHO_AM_I Read ERROR:", I2Cstatus);
      return 0;
  }
  if( tempBuffer != COMPASS_RESULT_WHO_AM_I){
      TIVA_error_encoutered("I2C: Compass --> ERROR: Unexpected WHO AM I Result", tempBuffer);
      return 0;
  }

  DEBUGCONSOLE_print_line("I2C: Compass --> OK\0");

  //----------------------Writing to Configuration Registers
  I2Cstatus = I2C_multi_write_blocking(&I2CChannel1,COMPASS_ADDR, &COMPASS_CMD_CTRL1_MODE, 2);
  if( I2Cstatus != I2C_MASTER_ERR_NONE){
      TIVA_error_encoutered("I2C: Compass --> CTRL1 Write ERROR:", I2Cstatus);
      return 0;
  }
  I2Cstatus = I2C_multi_write_blocking(&I2CChannel1,COMPASS_ADDR, &COMPASS_CMD_CTRL2_MODE, 2);
  if( I2Cstatus != I2C_MASTER_ERR_NONE){
      TIVA_error_encoutered("I2C: Compass --> CTRL2 Write ERROR:", I2Cstatus);
      return 0;
  }
  I2Cstatus = I2C_multi_write_blocking(&I2CChannel1,COMPASS_ADDR, &COMPASS_CMD_CTRL5_MODE, 2);
  if( I2Cstatus != I2C_MASTER_ERR_NONE){
      TIVA_error_encoutered("I2C: Compass --> CTRL5 Write ERROR:", I2Cstatus);
      return 0;
  }
  I2Cstatus = I2C_multi_write_blocking(&I2CChannel1,COMPASS_ADDR, &COMPASS_CMD_CTRL6_MODE, 2);
  if( I2Cstatus != I2C_MASTER_ERR_NONE){
      TIVA_error_encoutered("I2C: Compass --> CTRL6 Write ERROR:", I2Cstatus);
      return 0;
  }
  I2Cstatus = I2C_multi_write_blocking(&I2CChannel1,COMPASS_ADDR, &COMPASS_CMD_CTRL7_MODE, 2);
  if( I2Cstatus != I2C_MASTER_ERR_NONE){
      TIVA_error_encoutered("I2C: Compass --> CTRL7 Write ERROR:", I2Cstatus);
      return 0;
  }

  return 1;
}




uint8_t COMPASS_get_raw_accel_values(void){
    uint8_t lowByte,highByte;
    char tempStr [10];
    uint32_t I2Cstatus;


    //Send out Acceleromter Read Stream Start Command
    I2Cstatus = I2C_single_write_blocking(&I2CChannel1,COMPASS_ADDR, COMPASS_CMD_ACCELL_X_LSB|0x80);
    if( I2Cstatus != I2C_MASTER_ERR_NONE){
        TIVA_error_encoutered("I2C: Compass --> Accel Stream Write ERROR:", I2Cstatus);
        return 0;
    }

    //Read X Low and High Bytes
    I2Cstatus = I2C_single_read_blocking(&I2CChannel1,COMPASS_ADDR,&lowByte);
    if( I2Cstatus != I2C_MASTER_ERR_NONE){
        TIVA_error_encoutered("I2C: Compass --> Accel X_LSB Read ERROR:", I2Cstatus);
        return 0;
    }
    I2Cstatus = I2C_single_read_blocking(&I2CChannel1,COMPASS_ADDR,&highByte);
    if( I2Cstatus != I2C_MASTER_ERR_NONE){
        TIVA_error_encoutered("I2C: Compass --> Accel X_MSB Read ERROR:", I2Cstatus);
        return 0;
    }
    AccellData.x = (highByte<<8) | lowByte;


    //Read Y Low and High Bytes
    I2Cstatus = I2C_single_read_blocking(&I2CChannel1,COMPASS_ADDR,&lowByte);
    if( I2Cstatus != I2C_MASTER_ERR_NONE){
        TIVA_error_encoutered("I2C: Compass --> Accel Y_LSB Read ERROR:", I2Cstatus);
        return 0;
    }
    I2Cstatus = I2C_single_read_blocking(&I2CChannel1,COMPASS_ADDR,&highByte);
    if( I2Cstatus != I2C_MASTER_ERR_NONE){
        TIVA_error_encoutered("I2C: Compass --> Accel Y_MSB Read ERROR:", I2Cstatus);
        return 0;
    }
    AccellData.y = (highByte<<8) | lowByte;


    //Read Z Low and High Bytes
    I2Cstatus = I2C_single_read_blocking(&I2CChannel1,COMPASS_ADDR,&lowByte);
    if( I2Cstatus != I2C_MASTER_ERR_NONE){
        TIVA_error_encoutered("I2C: Compass --> Accel Z_LSB Read ERROR:", I2Cstatus);
        return 0;
    }
    I2Cstatus = I2C_single_read_blocking(&I2CChannel1,COMPASS_ADDR,&highByte);
    if( I2Cstatus != I2C_MASTER_ERR_NONE){
        TIVA_error_encoutered("I2C: Compass --> Accel Z_MSB Read ERROR:", I2Cstatus);
        return 0;
    }
    AccellData.z = (highByte<<8) | lowByte;


    #ifdef __VERBOSE_DEBUG_MODE__
        //Print Acceleromter Data
        DEBUGCONSOLE_print("I2C: ACELL: AX = \0");
        DEBUGCONSOLE_print(TIVA_int_to_String(tempStr, AccellData.x));
        DEBUGCONSOLE_print("\tAY = \0");
        DEBUGCONSOLE_print(TIVA_int_to_String(tempStr, AccellData.y));
        DEBUGCONSOLE_print("\tAZ = \0");
        DEBUGCONSOLE_print(TIVA_int_to_String(tempStr, AccellData.z));
        DEBUGCONSOLE_print("\n\r\0");
    #endif

    return 1;
}




uint8_t COMPASS_get_raw_magn_values(void){
    uint8_t lowByte,highByte;
    char tempStr [10];
    uint32_t I2Cstatus;


    //Send out Magnetometer Read Stream Start Command
    I2Cstatus = I2C_single_write_blocking(&I2CChannel1,COMPASS_ADDR, COMPASS_CMD_MAGN_X_LSB|0x80);
    if( I2Cstatus != I2C_MASTER_ERR_NONE){
        TIVA_error_encoutered("I2C: Compass --> Magn Stream Write ERROR:", I2Cstatus);
        return 0;
    }

    //Read X Low and High Bytes
    I2Cstatus = I2C_single_read_blocking(&I2CChannel1,COMPASS_ADDR,&lowByte);
    if( I2Cstatus != I2C_MASTER_ERR_NONE){
        TIVA_error_encoutered("I2C: Compass --> Magn X_LSB Read ERROR:", I2Cstatus);
        return 0;
    }
    I2Cstatus = I2C_single_read_blocking(&I2CChannel1,COMPASS_ADDR,&highByte);
    if( I2Cstatus != I2C_MASTER_ERR_NONE){
        TIVA_error_encoutered("I2C: Compass --> Magn X_MSB Read ERROR:", I2Cstatus);
        return 0;
    }
    MagnData.x = (highByte<<8) | lowByte;


    //Read Y Low and High Bytes
    I2Cstatus = I2C_single_read_blocking(&I2CChannel1,COMPASS_ADDR,&lowByte);
    if( I2Cstatus != I2C_MASTER_ERR_NONE){
        TIVA_error_encoutered("I2C: Compass --> Magn Y_LSB Read ERROR:", I2Cstatus);
        return 0;
    }
    I2Cstatus = I2C_single_read_blocking(&I2CChannel1,COMPASS_ADDR, &highByte);
    if( I2Cstatus != I2C_MASTER_ERR_NONE){
        TIVA_error_encoutered("I2C: Compass --> Magn Y_MSB Read ERROR:", I2Cstatus);
        return 0;
    }
    MagnData.y = (highByte<<8) | lowByte;


    //Read Z Low and High Bytes
    I2Cstatus = I2C_single_read_blocking(&I2CChannel1,COMPASS_ADDR,&lowByte);
    if( I2Cstatus != I2C_MASTER_ERR_NONE){
        TIVA_error_encoutered("I2C: Compass --> Magn Z_LSB Read ERROR:", I2Cstatus);
        return 0;
    }
    I2Cstatus = I2C_single_read_blocking(&I2CChannel1,COMPASS_ADDR,&highByte);
    if( I2Cstatus != I2C_MASTER_ERR_NONE){
        TIVA_error_encoutered("I2C: Compass --> Magn Z_MSB Read ERROR:", I2Cstatus);
        return 0;
    }
    MagnData.z = (highByte<<8) | lowByte;


    #ifdef __VERBOSE_DEBUG_MODE__
        //Print Magnetometer Data
        DEBUGCONSOLE_print("I2C: MAGN: MX = \0");
        DEBUGCONSOLE_print(TIVA_int_to_String(tempStr, MagnData.x));
        DEBUGCONSOLE_print("\tMY = \0");
        DEBUGCONSOLE_print(TIVA_int_to_String(tempStr, MagnData.y));
        DEBUGCONSOLE_print("\tMZ = \0");
        DEBUGCONSOLE_print(TIVA_int_to_String(tempStr, MagnData.z));
        DEBUGCONSOLE_print("\n\r\0");
    #endif
    return 1;
}

void COMPASS_update_heading(void){
    char tempStr [10];

    /*

    //Pitch Calculation
    pitch = asin(-COMPASS_AX);
    roll = asin(COMPASS_AY)-asin(cos(pitch));

    //Tilt compensated X-coordinate
    TILT_COMP_MX = COMPASS_MX*cos(pitch) + COMPASS_MZ*sin(pitch);

    TILT_COMP_MY = COMPASS_MX*sin(pitch)*sin(roll) +
            COMPASS_MY*cos(roll)-COMPASS_MZ*sin(roll)*cos(pitch);

    TILT_COMP_MZ = -COMPASS_MX*cos(roll)*sin(pitch) +
            COMPASS_MY*sin(roll) +
            COMPASS_MZ*cos(roll)*cos(pitch);

    //Heading Calculation
    if(TILT_COMP_MX > 0 && TILT_COMP_MY >= 0){
        heading =	atan2( TILT_COMP_MY, TILT_COMP_MX);
    }
    else if(TILT_COMP_MX < 0){
        heading =	atan2( TILT_COMP_MY, TILT_COMP_MX) + 180.0;
    }
    else if(TILT_COMP_MX > 0 && TILT_COMP_MY <= 0){
        heading =	atan2( TILT_COMP_MY, TILT_COMP_MX) + 360.0;
    }
    else if(TILT_COMP_MX == 0 && TILT_COMP_MY < 0){
        heading = 90.0;
    }
    else if(TILT_COMP_MX == 0 && TILT_COMP_MY > 0){
        heading =	270.0;
    }
    */
    /*
    int32_vec_t temp_m = {MagnData.x, MagnData.y, MagnData.z};

    // subtract offset (average of min and max) from magnetometer readings
    temp_m.x -= ((int32_t)COMPASS_MAGN_CALIB_MIN.x + COMPASS_MAGN_CALIB_MAX.x) / 2;
    temp_m.y -= ((int32_t)COMPASS_MAGN_CALIB_MIN.y + COMPASS_MAGN_CALIB_MAX.y) / 2;
    temp_m.z -= ((int32_t)COMPASS_MAGN_CALIB_MIN.z + COMPASS_MAGN_CALIB_MAX.z) / 2;

    // compute E and N
    float_vec_t E;
    float_vec_t N;
    TIVA_vector_cross_32_16(&temp_m, &AccellData, &E);
    TIVA_vector_normalize(&E);
    TIVA_vector_cross_16_float(&AccellData, &E, &N);
    TIVA_vector_normalize(&N);

    // compute heading
    CompassHeading = atan2(TIVA_vector_dot(&E, &COMPASS_BODY_VEC), TIVA_vector_dot(&N, &COMPASS_BODY_VEC)) * 180 / MATH_PI;

    */
    CompassHeading =  atan2( MagnData.y, MagnData.x) *  180.0/ MATH_PI;

    if (CompassHeading < 0) CompassHeading += 360;


    CompassSection = (uint16_t) CompassHeading /DEGREES_PER_SECTION;
    CompassSectionDegree =(uint8_t) CompassHeading %DEGREES_PER_SECTION;
    CompassSectionSafe = (uint8_t)(CompassSectionDegree>6 && CompassSectionDegree <13) ? 1:0;


    DEBUGCONSOLE_print("Heading: \0");
    DEBUGCONSOLE_print(TIVA_int_to_String(tempStr, CompassHeading));

    DEBUGCONSOLE_print("\tSection: \0");
    DEBUGCONSOLE_print(TIVA_int_to_String(tempStr, CompassSection));

    DEBUGCONSOLE_print("\tSection Middle: \0");
    DEBUGCONSOLE_print(TIVA_int_to_String(tempStr, CompassSectionSafe));
    DEBUGCONSOLE_print("\n\r\0");


}



void COMPASS_caliberate_Mode(void){

  int16_t Caliberate_min_MAG [3] = {0,0,0};
  int16_t Caliberate_max_MAG [3] = {0,0,0};
  int16_t Caliberate_min_ACC [3] = {0,0,0};
  int16_t Caliberate_max_ACC [3] = {0,0,0};

  char strBuf[15];
  while(1){

      TIVA_wait_miliSeconds(250);

      if(!COMPASS_get_raw_accel_values()){  break;}
      Caliberate_min_MAG[0] = (MagnData.x<Caliberate_min_MAG[0]) ?  MagnData.x: Caliberate_min_MAG[0];
      Caliberate_min_MAG[1] = (MagnData.y<Caliberate_min_MAG[1]) ?  MagnData.y: Caliberate_min_MAG[1];
      Caliberate_min_MAG[2] = (MagnData.z<Caliberate_min_MAG[2]) ?  MagnData.z: Caliberate_min_MAG[2];

      Caliberate_max_MAG[0] = (MagnData.x>Caliberate_max_MAG[0]) ?  MagnData.x: Caliberate_max_MAG[0];
      Caliberate_max_MAG[1] = (MagnData.y>Caliberate_max_MAG[1]) ?  MagnData.y: Caliberate_max_MAG[1];
      Caliberate_max_MAG[2] = (MagnData.z>Caliberate_max_MAG[2]) ?  MagnData.z: Caliberate_max_MAG[2];

      if(!COMPASS_get_raw_magn_values()){  break;}

      Caliberate_min_ACC[0] = (AccellData.x<Caliberate_min_ACC[0]) ?  AccellData.x: Caliberate_min_ACC[0];
      Caliberate_min_ACC[1] = (AccellData.y<Caliberate_min_ACC[1]) ?  AccellData.y: Caliberate_min_ACC[1];
      Caliberate_min_ACC[2] = (AccellData.z<Caliberate_min_ACC[2]) ?  AccellData.z: Caliberate_min_ACC[2];


      Caliberate_max_ACC[0] = (AccellData.x>Caliberate_max_ACC[0]) ?  AccellData.x: Caliberate_max_ACC[0];
      Caliberate_max_ACC[1] = (AccellData.y>Caliberate_max_ACC[1]) ?  AccellData.y: Caliberate_max_ACC[1];
      Caliberate_max_ACC[2] = (AccellData.z>Caliberate_max_ACC[2]) ?  AccellData.z: Caliberate_max_ACC[2];

      DEBUGCONSOLE_print("MIN ACC: X = \0");
      DEBUGCONSOLE_print(TIVA_int_to_String(strBuf, Caliberate_min_ACC[0]));
      DEBUGCONSOLE_print("\tY = \0");
      DEBUGCONSOLE_print(TIVA_int_to_String(strBuf, Caliberate_min_ACC[1]));
      DEBUGCONSOLE_print("\tZ = \0");
      DEBUGCONSOLE_print(TIVA_int_to_String(strBuf, Caliberate_min_ACC[2]));

      DEBUGCONSOLE_print(" MAX : X = \0");
      DEBUGCONSOLE_print(TIVA_int_to_String(strBuf, Caliberate_max_ACC[0]));
      DEBUGCONSOLE_print("\tY = \0");
      DEBUGCONSOLE_print(TIVA_int_to_String(strBuf, Caliberate_max_ACC[1]));
      DEBUGCONSOLE_print("\tZ = \0");
      DEBUGCONSOLE_print(TIVA_int_to_String(strBuf, Caliberate_max_ACC[2]));
      DEBUGCONSOLE_print("\n\r\0");

      DEBUGCONSOLE_print("MIN MAG: X = \0");
      DEBUGCONSOLE_print(TIVA_int_to_String(strBuf, Caliberate_min_MAG[0]));
      DEBUGCONSOLE_print("\tY = \0");
      DEBUGCONSOLE_print(TIVA_int_to_String(strBuf, Caliberate_min_MAG[1]));
      DEBUGCONSOLE_print("\tZ = \0");
      DEBUGCONSOLE_print(TIVA_int_to_String(strBuf, Caliberate_min_MAG[2]));

      DEBUGCONSOLE_print(" MAX MAG : X = \0");
      DEBUGCONSOLE_print(TIVA_int_to_String(strBuf, Caliberate_max_MAG[0]));
      DEBUGCONSOLE_print("\tY = \0");
      DEBUGCONSOLE_print(TIVA_int_to_String(strBuf, Caliberate_max_MAG[1]));
      DEBUGCONSOLE_print("\tZ = \0");
      DEBUGCONSOLE_print(TIVA_int_to_String(strBuf, Caliberate_max_MAG[2]));
      DEBUGCONSOLE_print("\n\r\0");
  }
}


void COMPASS_wait_Section_Reached(uint8_t section){
  while(CompassSection!= section || !CompassSectionSafe);
}
