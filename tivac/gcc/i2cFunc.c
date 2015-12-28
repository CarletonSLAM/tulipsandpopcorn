#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "hw_i2c.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_gpio.h"
#include "i2c.h"
#include "sysctl.h"
#include "gpio.h"
#include "pin_map.h"
#include "rom.h"
#include "uart.h"
#include "fpu.h"


#include "i2cFunc.h"
#include "tulips.h"


void I2C_setup(const i2c_port *I2CPort) {

  //Enable and reset I2C 1 channel
  ROM_SysCtlPeripheralEnable(I2CPort->periphComp);
  ROM_SysCtlPeripheralReset(I2CPort->periphComp);

  //Enable the GPIO A Pheriperal
  ROM_SysCtlPeripheralEnable(I2CPort->periphPort);

  // Set GPIO A6 and A7 as I2C SCL and SDA pins
  ROM_GPIOPinConfigure(I2CPort->sclPort);
  ROM_GPIOPinConfigure(I2CPort->sdaPort);

  ROM_GPIOPinTypeI2CSCL(I2CPort->i2cPortBase, I2CPort->sclPin);
  ROM_GPIOPinTypeI2C(I2CPort->i2cPortBase, I2CPort->sdaPin);


  //Set the I2C clock bus speed to 100 kbps
  ROM_I2CMasterInitExpClk(I2CPort->i2cBase, ROM_SysCtlClockGet(), false);

  //Clear the FIFO flags
  HWREG(I2CPort->i2cBase + I2C_O_FIFOCTL) = 80008000;
}



uint32_t I2C_single_read_blocking(const i2c_port *I2CPort,const uint8_t i2cAddr, uint8_t* responseBuffer){
    uint32_t I2Cstatus;

    ROM_I2CMasterSlaveAddrSet(I2CPort->i2cBase, i2cAddr, true);
    ROM_I2CMasterControl(I2CPort->i2cBase,I2C_MASTER_CMD_SINGLE_RECEIVE);
    while(ROM_I2CMasterBusy(I2CPort->i2cBase));

    I2Cstatus = ROM_I2CMasterErr(I2CPort->i2cBase);
    if(I2Cstatus != I2C_MASTER_ERR_NONE){
        return I2Cstatus;
    }

    *responseBuffer = ROM_I2CMasterDataGet(I2CPort->i2cBase);

    return I2Cstatus;
}



uint32_t I2C_multi_read_blocking(const i2c_port *I2CPort,const uint8_t i2cAddr,   uint8_t i2cCommand, uint8_t i2cLength,char* i2cResponseBuffer){
    int msgIndex;
    uint32_t I2Cstatus;

    //Set to write to salve device
    ROM_I2CMasterSlaveAddrSet(I2CPort->i2cBase, i2cAddr, false);

    ROM_I2CMasterDataPut(I2CPort->i2cBase, i2cCommand);
    ROM_I2CMasterControl(I2CPort->i2cBase, I2C_MASTER_CMD_BURST_SEND_START);
    while(ROM_I2CMasterBusy(I2CPort->i2cBase));

    I2Cstatus = ROM_I2CMasterErr(I2CPort->i2cBase);
    if(I2Cstatus != I2C_MASTER_ERR_NONE){
        return I2Cstatus;
    }

    //Set to read from salve device
    ROM_I2CMasterSlaveAddrSet(I2CPort->i2cBase, i2cAddr, true);

    ROM_I2CMasterControl(I2CPort->i2cBase, I2C_MASTER_CMD_BURST_RECEIVE_START);
    while(ROM_I2CMasterBusy(I2CPort->i2cBase));

    I2Cstatus = ROM_I2CMasterErr(I2CPort->i2cBase);
    if(I2Cstatus != I2C_MASTER_ERR_NONE){
        return I2Cstatus;
    }


    *i2cResponseBuffer = ROM_I2CMasterDataGet(I2CPort->i2cBase);


    for(msgIndex = 1;msgIndex < i2cLength-1;msgIndex++ ){

      ROM_I2CMasterControl(I2CPort->i2cBase, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
      while(ROM_I2CMasterBusy(I2CPort->i2cBase));

      I2Cstatus = ROM_I2CMasterErr(I2CPort->i2cBase);
      if(I2Cstatus != I2C_MASTER_ERR_NONE){
          return I2Cstatus;
      }
      *(i2cResponseBuffer+msgIndex) = ROM_I2CMasterDataGet(I2CPort->i2cBase);

    }
    ROM_I2CMasterControl(I2CPort->i2cBase, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    while(ROM_I2CMasterBusy(I2CPort->i2cBase));

    I2Cstatus = ROM_I2CMasterErr(I2CPort->i2cBase);
    if(I2Cstatus != I2C_MASTER_ERR_NONE){
        return I2Cstatus;
    }

    *(i2cResponseBuffer+(i2cLength-1)) = ROM_I2CMasterDataGet(I2CPort->i2cBase);

    return I2Cstatus;
}


uint32_t I2C_single_write_blocking(const i2c_port *I2CPort,const uint8_t i2cAddr, uint8_t i2cCommand){
    uint32_t I2Cstatus;
    //Set to write from salve device
    ROM_I2CMasterSlaveAddrSet(I2CPort->i2cBase, i2cAddr, false);
    ROM_I2CMasterDataPut(I2CPort->i2cBase, i2cCommand);
    ROM_I2CMasterControl(I2CPort->i2cBase,I2C_MASTER_CMD_SINGLE_SEND);
    while(ROM_I2CMasterBusy(I2CPort->i2cBase));

    I2Cstatus = ROM_I2CMasterErr(I2CPort->i2cBase);
    if(I2Cstatus != I2C_MASTER_ERR_NONE){
        return I2Cstatus;
    }

    return I2Cstatus;
}

uint32_t I2C_multi_write_blocking(const i2c_port *I2CPort,const uint8_t i2cAddr, uint8_t* i2cCommand, uint8_t i2cLength){
    uint32_t I2Cstatus;
    int msgIndex;
    //Set to write from salve device
    ROM_I2CMasterSlaveAddrSet(I2CPort->i2cBase, i2cAddr, false);

    ROM_I2CMasterDataPut(I2CPort->i2cBase, *i2cCommand);
    ROM_I2CMasterControl(I2CPort->i2cBase, I2C_MASTER_CMD_BURST_SEND_START);
    while(ROM_I2CMasterBusy(I2CPort->i2cBase));

    I2Cstatus = ROM_I2CMasterErr(I2CPort->i2cBase);
    if(I2Cstatus != I2C_MASTER_ERR_NONE){
        return I2Cstatus;
    }

    for(msgIndex = 1;msgIndex < i2cLength-1;msgIndex++ ){

      ROM_I2CMasterDataPut(I2CPort->i2cBase, *(i2cCommand+msgIndex));

      ROM_I2CMasterControl(I2CPort->i2cBase, I2C_MASTER_CMD_BURST_SEND_CONT);
      while(ROM_I2CMasterBusy(I2CPort->i2cBase));
      I2Cstatus = ROM_I2CMasterErr(I2CPort->i2cBase);
      if(I2Cstatus != I2C_MASTER_ERR_NONE){
          return I2Cstatus;
      }
    }
    ROM_I2CMasterDataPut(I2CPort->i2cBase, *(i2cCommand+(i2cLength-1)));
    ROM_I2CMasterControl(I2CPort->i2cBase, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(ROM_I2CMasterBusy(I2CPort->i2cBase));

    I2Cstatus = ROM_I2CMasterErr(I2CPort->i2cBase);
    if(I2Cstatus != I2C_MASTER_ERR_NONE){
        return I2Cstatus;
    }

    return I2Cstatus;
}
