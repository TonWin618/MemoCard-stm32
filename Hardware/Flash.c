#include "Flash.h"

#define Flash_PageSize						256
#define Flash_PerWritePageSize		256

#define W25X_WriteEnable		      0x06 
#define W25X_WriteDisable		      0x04 
#define W25X_ReadStatusReg1		    0x05
#define W25X_ReadStatusReg2				0x35
#define W25X_WriteStatusReg		    0x01 
#define W25X_ReadData			        0x03 
#define W25X_FastReadData		      0x0B 
#define W25X_FastReadDual		      0x3B 
#define W25X_PageProgram		      0x02
#define W25X_BlockErase			      0xD8 
#define W25X_SectorErase		      0x20 
#define W25X_ChipErase			      0xC7 
#define W25X_PowerDown			      0xB9 
#define W25X_ReleasePowerDown	    0xAB 
#define W25X_DeviceID			        0xAB 
#define W25X_ManufactDeviceID   	0x90 
#define W25X_JedecDeviceID		    0x9F 
#define W25X_WriteSecurity     		0x42
#define W25X_ReadSecurity					0x48
#define W25X_EraseSecurity				0x44

#define WIP_Flag                  0x01
#define Dummy_Byte                0xFF

//初始化Flash
void Flash_Init(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(Flash_SCK_GPIO_CLK | Flash_DI_GPIO_CLK| Flash_DO_GPIO_CLK | Flash_CS_GPIO_CLK, ENABLE);
	RCC_APB1PeriphClockCmd(Flash_SPI_CLK, ENABLE);
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	//配置SCK引脚
	GPIO_InitStructure.GPIO_Pin = Flash_SCK_GPIO_Pin;
	GPIO_Init(Flash_SCK_GPIO_Port,&GPIO_InitStructure);
	//配置DO引脚
	GPIO_InitStructure.GPIO_Pin = Flash_DO_GPIO_Pin;
	GPIO_Init(Flash_DO_GPIO_Port,&GPIO_InitStructure);
	//配置DI引脚
	GPIO_InitStructure.GPIO_Pin = Flash_DI_GPIO_Pin;
	GPIO_Init(Flash_DI_GPIO_Port,&GPIO_InitStructure);
	//配置CS引脚
	GPIO_InitStructure.GPIO_Pin = Flash_CS_GPIO_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(Flash_CS_GPIO_Port,&GPIO_InitStructure);
	
	//初始化SPI
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(Flash_SPI, &SPI_InitStructure);
	SPI_Cmd(Flash_SPI, ENABLE);
}
//扇区擦除
//SectorAddr：扇区地址
void Flash_EraseSector(u32 SectorAddr)
{
	Flash_WriteEnable();
	Flash_WaitForWriteEnd();
	GPIO_ResetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	Flash_SendByte(W25X_SectorErase);
	
	Flash_SendByte((SectorAddr & 0xFF0000)>>16);
	Flash_SendByte((SectorAddr & 0xFF00)>>8);
	Flash_SendByte(SectorAddr & 0xFF);
	
	GPIO_SetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	Flash_WaitForWriteEnd();
}
//整片擦除
void Flash_EraseBulk()
{
	Flash_WriteEnable();
	GPIO_ResetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	Flash_SendByte(W25X_ChipErase);
	GPIO_SetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	Flash_WaitForWriteEnd();
}
//开启队列阅读模式
//ReadAddr：读取开始地址
void Flash_StartReadSequence(u32 ReadAddr)
{
	GPIO_WriteBit(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin,Bit_RESET);
	Flash_SendByte(W25X_ReadData);
	Flash_SendByte((ReadAddr & 0xFF0000)>>16);
	Flash_SendByte((ReadAddr & 0xFF00)>>8);
	Flash_SendByte(ReadAddr & 0xFF);
}
//页写
//pBuffer：要写入的数据
//WriteAddr：要写入的地址
//NumByteToWrite：要写入的字节数
void Flash_WritePage(u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
	Flash_WriteEnable();
	GPIO_ResetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	
	Flash_SendByte(W25X_PageProgram);
	Flash_SendByte((WriteAddr & 0xFF0000)>>16);
	Flash_SendByte((WriteAddr & 0xFF00)>>8);
	Flash_SendByte(WriteAddr & 0xFF);
	
	//GPIO_SetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	
	if(NumByteToWrite > Flash_PerWritePageSize)
	{
		NumByteToWrite = Flash_PerWritePageSize;
	}
	
	while(NumByteToWrite--)
	{
		Flash_SendByte(*pBuffer);
		pBuffer++;
	}
	
	GPIO_SetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	Flash_WaitForWriteEnd();
}
//读取
//pBuffer:要存入的位置
//WriteAddr:读取开始的地址
//NumByteToWrite:要读取的字节数
void Flash_ReadBuffer(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	GPIO_ResetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	Flash_SendByte(W25X_ReadData);
	
	Flash_SendByte((ReadAddr & 0xFF0000) >> 16);
	Flash_SendByte((ReadAddr & 0xFF00) >> 8);
	Flash_SendByte(ReadAddr & 0xFF);
	
	while(NumByteToRead--)
	{
		*pBuffer = Flash_SendByte(Dummy_Byte);
		pBuffer ++; 
	}
	
	GPIO_SetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
}
//写入
//pBuffer:要写入的数据
//WriteAddr:要写入的地址
//NumByteToWrite:要写入的字节数
void Flash_WriteBuffer(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
	GPIO_ResetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

  Addr = WriteAddr % Flash_PageSize;
  count = Flash_PageSize - Addr;
  NumOfPage =  NumByteToWrite / Flash_PageSize;
  NumOfSingle = NumByteToWrite % Flash_PageSize;

  if (Addr == 0) //新页写
  {
    if (NumOfPage == 0)//单页写
    {
      Flash_WritePage(pBuffer, WriteAddr, NumByteToWrite);
    }
    else //多页写
    {
      while (NumOfPage--)
      {
        Flash_WritePage(pBuffer, WriteAddr, Flash_PageSize);
        WriteAddr +=  Flash_PageSize;//下一页
        pBuffer += Flash_PageSize;//缓存区内容移动
      }
      Flash_WritePage(pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else //承接上页
  {
    if (NumOfPage == 0)
    {
      if (NumOfSingle > count)
      {
        temp = NumOfSingle - count;

        Flash_WritePage(pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;

        Flash_WritePage(pBuffer, WriteAddr, temp);
      }
      else
      {
        Flash_WritePage(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / Flash_PageSize;
      NumOfSingle = NumByteToWrite % Flash_PageSize;

      Flash_WritePage(pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;

      while (NumOfPage--)
      {
        Flash_WritePage(pBuffer, WriteAddr, Flash_PageSize);
        WriteAddr +=  Flash_PageSize;
        pBuffer += Flash_PageSize;
      }

      if (NumOfSingle != 0)
      {
        Flash_WritePage(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

 u8 Flash_ReadByte(void)
{
	return(Flash_SendByte(Dummy_Byte));
}
//发送一字节数据
u8 Flash_SendByte(u8 byte)
{
	while(SPI_I2S_GetFlagStatus(Flash_SPI,SPI_I2S_FLAG_TXE) == RESET);//等待发送区空
	SPI_I2S_SendData(Flash_SPI,byte);//发送一个byte数据
	while(SPI_I2S_GetFlagStatus(Flash_SPI,SPI_I2S_FLAG_RXNE) == RESET);//等待接收完一个byte
	return SPI_I2S_ReceiveData(Flash_SPI);//返回接收的数据
}
//写使能
void Flash_WriteEnable(void)
{
	GPIO_ResetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	Flash_SendByte(W25X_WriteEnable);
	GPIO_SetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
}
//等待上一个命令完成
void Flash_WaitForWriteEnd(void)
{
	u8 Flash_Status = 0;
	GPIO_ResetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	Flash_SendByte(W25X_ReadStatusReg1);
	do
	{
		Flash_Status = Flash_SendByte(Dummy_Byte);
	}
	while((Flash_Status & WIP_Flag) == SET);
	
	GPIO_SetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
}
//关闭设备
void Flash_PowerDown(void)
{
	GPIO_ResetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	Flash_SendByte(W25X_PowerDown);
	GPIO_SetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
}
//唤醒设备
void Flash_Wakeup(void)
{
	GPIO_ResetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	Flash_SendByte(W25X_ReleasePowerDown);
	GPIO_SetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
}
//读取产品ID
u32 Flash_ReadID(void)
{
  u32 Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;
  GPIO_ResetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
  Flash_SendByte(W25X_JedecDeviceID);
  Temp0 = Flash_SendByte(Dummy_Byte);
  Temp1 = Flash_SendByte(Dummy_Byte);
  Temp2 = Flash_SendByte(Dummy_Byte);
	GPIO_SetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
  Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
  return Temp;
}
//读状态寄存器1
u8 Flash_ReadStatus1()
{
	u8 status;
	GPIO_ResetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	Flash_SendByte(W25X_ReadStatusReg1);
	status = Flash_SendByte(Dummy_Byte);
	GPIO_SetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	return status;
}
//读状态寄存器2
u8 Flash_ReadStatus2()
{
	u8 status;
	GPIO_ResetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	Flash_SendByte(W25X_ReadStatusReg2);
	status = Flash_SendByte(Dummy_Byte);
	GPIO_SetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	return status;
}
//写状态寄存器
//status1：要写入状态寄存器1的数据
//status1：要写入状态寄存器1的数据
void Flash_WriteStatus(u8 status1,u8 status2)
{
	Flash_WriteEnable();
	GPIO_ResetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
  Flash_SendByte(W25X_WriteStatusReg);
	Flash_SendByte(status1);
	Flash_SendByte(status2);
	GPIO_SetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
}
//写安全寄存器
//pBuffer：要写入的数据
//WriteAddr：写入的位置
//NumByteToWrite：要写入的字节
void Flash_WriteSecurity(u8* pBuffer,u32 WriteAddr, u16 NumByteToWrite)
{
	Flash_WriteEnable();
	GPIO_ResetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	
	Flash_SendByte(W25X_WriteSecurity);
	Flash_SendByte((WriteAddr & 0xFF0000)>>16);
	Flash_SendByte((WriteAddr & 0xFF00)>>8);
	Flash_SendByte(WriteAddr & 0xFF);
	
	while(NumByteToWrite--)
	{
		Flash_SendByte(*pBuffer);
		pBuffer++;
	}
	
	GPIO_SetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	Flash_WaitForWriteEnd();
}
//读安全寄存器
//pBuffer：要存入的位置
//ReadAddr：读取的位置
//NumByteToRead：读的字节数
void Flash_ReadSecurity(u8* pBuffer,u32 ReadAddr, u16 NumByteToRead)
{
	GPIO_ResetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	Flash_SendByte(W25X_ReadSecurity);
	
	Flash_SendByte((ReadAddr & 0xFF0000) >> 16);
	Flash_SendByte((ReadAddr & 0xFF00) >> 8);
	Flash_SendByte(ReadAddr & 0xFF);
	Flash_SendByte(Dummy_Byte);
	
	while(NumByteToRead--)
	{
		*pBuffer = Flash_SendByte(Dummy_Byte);
		pBuffer ++; 
	}
	
	GPIO_SetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
}
//擦除安全寄存器
//securityAddr:安全寄存器地址
void Flash_EraseSecurity(u32 securityAddr)
{
	Flash_WriteEnable();
	Flash_WaitForWriteEnd();
	GPIO_ResetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	Flash_SendByte(W25X_EraseSecurity);
	
	Flash_SendByte((securityAddr & 0xFF0000)>>16);
	Flash_SendByte((securityAddr & 0xFF00)>>8);
	Flash_SendByte(securityAddr & 0xFF);
	
	GPIO_SetBits(Flash_CS_GPIO_Port,Flash_CS_GPIO_Pin);
	Flash_WaitForWriteEnd();
}
