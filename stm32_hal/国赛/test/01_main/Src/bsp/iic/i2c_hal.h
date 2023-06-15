#ifndef __I2C_HAL_H
#define __I2C_HAL_H

#include "stm32g4xx_hal.h"

#define					SCL					GPIO_PIN_4
#define					SDA					GPIO_PIN_5

void I2CStart(void);
void I2CStop(void);
unsigned char I2CWaitAck(void);
void I2CSendAck(void);
void I2CSendNotAck(void);
void I2CSendByte(unsigned char cSendByte);
unsigned char I2CReceiveByte(void);
void I2CInit(void);



void IIC_AT24C02_Write(uint8_t * Date, uint8_t Address, uint8_t num);
void IIC_AT24C02_Read(uint8_t * Date, uint8_t Address, uint8_t num);

void LIS302_Config(void);
void LIS302_Output(int8_t * xx, int8_t * yy, int8_t * zz);
#endif
