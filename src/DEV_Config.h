#ifndef _DEV_CONFIG_H_
#define _DEV_CONFIG_H_
/***********************************************************************************************************************
			------------------------------------------------------------------------
			|\\\																///|
			|\\\					Hardware interface							///|
			------------------------------------------------------------------------
***********************************************************************************************************************/
#include "sysfs_gpio.h"
#include "dev_hardware_UART.h"

#include <stdint.h>
#include "Debug.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define DEV_SPI 0
#define DEV_I2C 0
#define DEV_UART 1

/**
 * data
**/
#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t

extern int IRQ_PIN;//25
extern int TXDEN_1; //27
extern int TXDEN_2; //22
/*-------------------------------------------------*/
uint8_t DEV_ModuleInit(void);
void    DEV_ModuleExit(void);

void DEV_GPIO_Mode(UWORD Pin, UWORD Mode);
void DEV_Digital_Write(UWORD Pin, uint8_t Value);
uint8_t DEV_Digital_Read(UWORD Pin);

void DEV_Delay_ms(UDOUBLE xms);

void DEV_UART_Init(char *Device);
void UART_Write_Byte(uint8_t data);
int UART_Read_Byte(void);
int UART_Read_Bytes(uint8_t *data,uint32_t len);
void UART_Set_Baudrate(uint32_t Baudrate);
int UART_Write_nByte(uint8_t *pData, uint32_t Lan);

#endif
