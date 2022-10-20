#include "stm32f10x.h"

void BlueTooth_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;   
	USART_InitTypeDef USART_InitStruct;   
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO|RCC_APB2Periph_USART1,ENABLE);   
	
	//����PA9��ʼ��
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_9;   
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;   
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;   
	GPIO_Init(GPIOA,&GPIO_InitStruct);   
	
	//����PA10��ʼ��
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_10;   
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;   
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;   
	GPIO_Init(GPIOA,&GPIO_InitStruct);   
	
	//����PA11��ʼ��
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	//USART1��ʼ��
	USART_InitStruct.USART_Mode=USART_Mode_Tx|USART_Mode_Rx;  
	USART_InitStruct.USART_Parity=USART_Parity_No;   
	USART_InitStruct.USART_BaudRate=9600;   
	USART_InitStruct.USART_StopBits=USART_StopBits_1;   
	USART_InitStruct.USART_WordLength=USART_WordLength_8b;   
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;   
	USART_Init(USART1,&USART_InitStruct);   
	USART_Cmd(USART1,ENABLE);   
	
	//����EXTI�жϣ�Line11���������½��ش�������������PA11
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource11);
	EXTI_InitStructure.EXTI_Line = EXTI_Line11;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	//NVIC���ã�0��ռ���ȼ���3�����ȼ�
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//������ȡ�豸���͹�������Ϣ��д��ָ��λ��
//TX_Buffer :д��λ�ã�lenth��д�볤��
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

//������ӡ��������Ϣ�������豸��
//content: �跢�͵�����
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

