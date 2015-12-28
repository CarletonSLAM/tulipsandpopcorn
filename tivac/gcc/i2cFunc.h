
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

typedef struct i2c_setup{
	uint32_t periphComp;
	uint32_t periphPort;
	uint32_t i2cBase;
	uint32_t i2cPortBase;
	uint32_t sclPort;
	uint32_t sclPin;
	uint32_t sdaPort;
	uint32_t sdaPin;
}i2c_port;




void I2C_setup(const i2c_port *I2CPort);


uint32_t I2C_single_read_blocking(const i2c_port *I2CPort,const uint8_t i2cAddr, uint8_t* responseBuffer);
uint32_t I2C_multi_read_blocking(const i2c_port *I2CPort,const uint8_t i2cAddr,  uint8_t i2cCommand, uint8_t i2cLength,char* i2cResponseBuffer);

uint32_t I2C_single_write_blocking(const i2c_port *I2CPort,const uint8_t i2cAddr, uint8_t i2cCommand);
uint32_t I2C_multi_write_blocking(const i2c_port *I2CPort,const uint8_t i2cAddr, uint8_t* i2cCommand, uint8_t i2cLength);
