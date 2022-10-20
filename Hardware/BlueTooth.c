#include "stm32f10x.h"

void BlueTooth_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;   
	USART_InitTypeDef USART_InitStruct;   
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO|RCC_APB2Periph_USART1,ENABLE);   
	
	//引脚PA9初始化
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_9;   
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;   
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;   
	GPIO_Init(GPIOA,&GPIO_InitStruct);   
	
	//引脚PA10初始化
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_10;   
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;   
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;   
	GPIO_Init(GPIOA,&GPIO_InitStruct);   
	
	//引脚PA11初始化
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	//USART1初始化
	USART_InitStruct.USART_Mode=USART_Mode_Tx|USART_Mode_Rx;  
	USART_InitStruct.USART_Parity=USART_Parity_No;   
	USART_InitStruct.USART_BaudRate=9600;   
	USART_InitStruct.USART_StopBits=USART_StopBits_1;   
	USART_InitStruct.USART_WordLength=USART_WordLength_8b;   
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;   
	USART_Init(USART1,&USART_InitStruct);   
	USART_Cmd(USART1,ENABLE);   
	
	//配置EXTI中断，Line11，上升沿下降沿触发，关联引脚PA11
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource11);
	EXTI_InitStructure.EXTI_Line = EXTI_Line11;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	//NVIC配置，0先占优先级，3从优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//蓝牙读取设备发送过来的信息并写入指定位置
//TX_Buffer :写入位置，lenth：写入长度
void BlueTooth_Scanf(uint8_t* TX_Buffer,uint16_t lenth)
{
	
	uint8_t TX_Data;
	uint16_t i = 0;
	TX_Data = USART_ReceiveData(USART1);
	
	for(i=0;i<lenth;i++)
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
		TX_Data = USART_ReceiveData(USART1);
		if(TX_Data=='\n'|| i==lenth-1)
		{
			TX_Buffer[i]='\0';
			break;
		}else
		{
			TX_Buffer[i]=TX_Data;
		}
	}
}

//蓝牙打印，发送信息到连接设备上
//content: 需发送的内容
void BlueTooth_Print(uint8_t* content)
{
	uint8_t i = 0;
	for(i=0; content[i]!='\0'; i++)
	{
		USART_SendData(USART1,content[i]);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
	}
	USART_SendData(USART1,'\r');
	USART_SendData(USART1,'\n');
}

uint8_t BlueTooth_IsConnected(void)
{
	uint8_t isConnected;
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11))
	{
		isConnected = 1;
	}
	else
	{
		isConnected = 0;
	}
	return isConnected;
}

