#include "sys.h"
#include "usart.h"
 
void uart_init(u32 bound)
	{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);
		
	//USART1_TX   GPIOA.9???
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//??????(GPIO????????STM32??????8-1-11)
	GPIO_Init(GPIOA, &GPIO_InitStructure);
   
	//USART1_RX	  GPIOA.10???
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//????
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
 
	//USART1 NVIC ??
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//?????3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//?????3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ????
	NVIC_Init(&NVIC_InitStructure);	//??????????VIC???
 
	//USART ?????
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//???8?????
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//?????
	USART_InitStructure.USART_Parity = USART_Parity_No;//??????
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//????????
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//????
 
	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//????????
	USART_Cmd(USART1, ENABLE);                    //????1  
}
 
void USART1_IRQHandler(void)    //??1??????
	{
		u8 Res;
		
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //????(RXNE????????,????????1,if????)
		{
			Res = USART_ReceiveData(USART1);	//????????,??Res
			USART_SendData(USART1,Res); 		//????
			if (Res == '1')		//????,???????;???1?????
			{
				 GPIO_ResetBits(GPIOC,GPIO_Pin_13);
			}
     } 
	 }
