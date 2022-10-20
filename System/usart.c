#include "stm32f10x.h"
#include "stdio.h"

void USART_init(uint32_t bound)
{
	GPIO_InitTypeDef GPIO_InitStruct;   //����GPIO�ṹ�����
	USART_InitTypeDef USART_InitStruct;   //���崮�ڽṹ�����
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1,ENABLE);   //ʹ��GPIOC��ʱ��
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_9;   //����TX����
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;   //����PA9Ϊ�����������
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;   //����PA9����
	GPIO_Init(GPIOA,&GPIO_InitStruct);   //GPIO��ʼ������
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_10;   //����RX����
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;   //����PA10Ϊ��������
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;   //����PA10����
	GPIO_Init(GPIOA,&GPIO_InitStruct);   //GPIO��ʼ������
	
	USART_InitStruct.USART_Mode=USART_Mode_Tx|USART_Mode_Rx;   //���ͽ���ģʽ
	USART_InitStruct.USART_Parity=USART_Parity_No;   //����żУ��
	USART_InitStruct.USART_BaudRate=bound;   //������
	USART_InitStruct.USART_StopBits=USART_StopBits_1;   //ֹͣλ1λ
	USART_InitStruct.USART_WordLength=USART_WordLength_8b;   //�ֳ�8λ
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;   //��Ӳ������������
	USART_Init(USART1,&USART_InitStruct);   //���ڳ�ʼ������
	
	USART_Cmd(USART1,ENABLE);   //ʹ��USART1
}

int fputc(int ch,FILE *f)   //printf�ض�����
{
	USART_SendData(USART1,(uint8_t)ch);   //����һ�ֽ�����
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);   //�ȴ��������
	return ch;
}

