#ifndef __I2C_H
#define __I2C_H

#include "stm32g4xx_hal.h"

void I2CStart(void);
void I2CStop(void);
unsigned char I2CWaitAck(void);
void I2CSendAck(void);
void I2CSendNotAck(void);
void I2CSendByte(unsigned char cSendByte);
unsigned char I2CReceiveByte(void);
void I2CInit(void);

void I2C_24C02_Write(unsigned char * acDate, unsigned char acADDR, unsigned char Num);
void I2C_24C02_Read(unsigned char * acDate, unsigned char acADDR, unsigned char Num);
	
void I2C_MCP4017_Write(unsigned char value);
unsigned char	I2C_MCP4017_Read(void);


void I2C_LIS302DL_Output(int8_t * xx, int8_t * yy, int8_t * zz);
void I2C_LIS302DL_Config(void);




#endif
