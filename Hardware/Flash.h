/*
	W25Q64闪存芯片驱动
	
	接线
	CLK	：PB10
	DO	：B1
	DI	：PB11
	CS	：B0
*/
#ifndef __FLASH_H
#define __FLASH_H
#include "stm32f10x.h"

#define Flash_SPI                   SPI2
#define Flash_SPI_CLK								RCC_APB1Periph_SPI2
#define Flash_CS_GPIO_Pin						GPIO_Pin_12
#define Flash_CS_GPIO_Port					GPIOB
#define Flash_CS_GPIO_CLK						RCC_APB2Periph_GPIOB
#define Flash_SCK_GPIO_Pin					GPIO_Pin_13
#define Flash_SCK_GPIO_Port					GPIOB
#define Flash_SCK_GPIO_CLK					RCC_APB2Periph_GPIOB
#define Flash_DO_GPIO_Pin						GPIO_Pin_14
#define Flash_DO_GPIO_Port					GPIOB
#define Flash_DO_GPIO_CLK						RCC_APB2Periph_GPIOB
#define Flash_DI_GPIO_Pin						GPIO_Pin_15
#define Flash_DI_GPIO_Port					GPIOB
#define Flash_DI_GPIO_CLK						RCC_APB2Periph_GPIOB

void Flash_Init(void);
void Flash_EraseSector(u32 SectorAddr);
void Flash_EraseBulk(void);

void Flash_WritePage(u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void Flash_WriteBuffer(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void Flash_ReadBuffer(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void Flash_StartReadSequence(u32 SectorAddr);

void Flash_WriteSecurity(u8* pBuffer,u32 WriteAddr, u16 NumByteToWrite);
void Flash_ReadSecurity(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void Flash_EraseSecurity(u32 securityAddr);

void Flash_PowerDown(void);
void Flash_Wakeup(void);

u32 Flash_ReadID(void);
u32 Flash_ReadDeviceID(void);

u8 Flash_ReadStatus1(void);
u8 Flash_ReadStatus2(void);
void Flash_WriteStatus(u8 status1, u8 status2);

u8 Flash_ReadByte(void);
u8 Flash_SendByte(u8 byte);
void Flash_WriteEnable(void);
void Flash_WaitForWriteEnd(void);

#endif
